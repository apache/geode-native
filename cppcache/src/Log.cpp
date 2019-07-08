/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "util/Log.hpp"

#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <string>

#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem.hpp>

#include <geode/ExceptionTypes.hpp>
#include <geode/util/LogLevel.hpp>

#include "geodeBanner.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "util/chrono/time_point.hpp"

namespace apache {
namespace geode {
namespace client {

const int __1K__ = 1024;
const int __1M__ = __1K__ * __1K__;
const int __1G__ = __1K__ * __1M__;
const int LOG_SCRATCH_BUFFER_SIZE = 16 * __1K__;

static std::recursive_mutex g_logMutex;
static std::shared_ptr<spdlog::logger> currentLogger;
static LogLevel currentLevel = LogLevel::None;
static std::string logFilePath;
static int32_t adjustedFileSizeLimit;
static int32_t maxFiles;

const std::shared_ptr<spdlog::logger>& getCurrentLogger() {
  if (logFilePath.empty()) {
    static auto consoleLogger = spdlog::stderr_color_mt("console");
    return consoleLogger;
  } else {
    if (!currentLogger) {
      currentLogger = spdlog::rotating_logger_mt(
          "file", logFilePath, adjustedFileSizeLimit, maxFiles);
    }
    return currentLogger;
  }
}

void Log::validateSizeLimits(int64_t fileSizeLimit, int64_t diskSpaceLimit) {
  if (fileSizeLimit * __1M__ > GEODE_MAX_LOG_FILE_LIMIT) {
    throw IllegalArgumentException(
        "Specified file size limit larger than max allowed (1GB)");
  } else if (fileSizeLimit < 0) {
    throw IllegalArgumentException("Specified file size limit must be >= 0");
  }

  if (diskSpaceLimit * __1M__ > GEODE_MAX_LOG_DISK_LIMIT) {
    throw IllegalArgumentException(
        "Specified disk space limit larger than max allowed (1TB)");
  } else if (diskSpaceLimit < 0) {
    throw IllegalArgumentException("Specified disk space limit must be >= 0");
  }

  if (fileSizeLimit > diskSpaceLimit && diskSpaceLimit != 0) {
    throw IllegalArgumentException(
        "Disk space limit must be larger than file size limit");
  }
}

void Log::init(LogLevel level, const char* logFileName, int32_t logFileLimit,
               int64_t logDiskSpaceLimit) {
  auto logFileNameString =
      logFileName ? std::string(logFileName) : std::string("geode-native.log");
  init(level, logFileNameString, logFileLimit, logDiskSpaceLimit);
}

std::string Log::logLineFormat() {
  std::stringstream format;
  const size_t MINBUFSIZE = 128;
  auto now = std::chrono::system_clock::now();
  auto secs = std::chrono::system_clock::to_time_t(now);
  auto tm_val = apache::geode::util::chrono::localtime(secs);

  format << "[%l %Y/%m/%d %H:%M:%S.%f " << std::put_time(&tm_val, "%Z  ")
         << boost::asio::ip::host_name() << ":%P %t] %v";

  return format.str();
}

void Log::setSizeLimits(int32_t logFileLimit, int64_t logDiskSpaceLimit,
                        int32_t& adjustedFileLimit,
                        int64_t& adjustedDiskLimit) {
  validateSizeLimits(logFileLimit, logDiskSpaceLimit);

  // Default to 10MB file limit and 1GB disk limit
  if (logFileLimit == 0 && logDiskSpaceLimit == 0) {
    adjustedFileLimit = 10 * __1M__;
    adjustedDiskLimit = 1000 * __1M__;
  }
  // disk space specified but file size is defaulted.  Just use a single
  // log file, i.e. set file limit == disk limit
  else if (logFileLimit == 0) {
    adjustedDiskLimit = logDiskSpaceLimit * __1M__;
    adjustedFileLimit = static_cast<int32_t>(adjustedDiskLimit);
  } else if (logDiskSpaceLimit == 0) {
    adjustedFileLimit = logFileLimit * __1M__;
    adjustedDiskLimit = adjustedFileLimit;
  } else {
    adjustedFileLimit = logFileLimit * __1M__;
    adjustedDiskLimit = logDiskSpaceLimit * __1M__;
  }
}

uint32_t Log::calculateMaxFilesForSpaceLimit(uint64_t logDiskSpaceLimit,
                                             uint32_t logFileSizeLimit) {
  uint32_t maxFileCount = 1;

  maxFileCount =
      static_cast<uint32_t>(logDiskSpaceLimit / logFileSizeLimit) - 1;
  // Must specify at least 1!
  maxFileCount = maxFileCount > 0 ? maxFileCount : 0;

  return maxFileCount;
}

spdlog::level::level_enum geodeLogLevelToSpdlogLevel(LogLevel logLevel) {
  auto level = spdlog::level::level_enum::off;
  switch (logLevel) {
    case LogLevel::None:
      level = spdlog::level::level_enum::off;
      break;
    case LogLevel::Error:
      level = spdlog::level::level_enum::err;
      break;
    case LogLevel::Warning:
      level = spdlog::level::level_enum::warn;
      break;
    case LogLevel::Info:
      level = spdlog::level::level_enum::info;
      break;
    case LogLevel::Default:
      level = spdlog::level::level_enum::info;
      break;
    case LogLevel::Config:
      level = spdlog::level::level_enum::info;
      break;
    case LogLevel::Fine:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::Finer:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::Finest:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::Debug:
      level = spdlog::level::level_enum::debug;
      break;
    case LogLevel::All:
      level = spdlog::level::level_enum::debug;
      break;
  }

  return level;
}

void Log::init(LogLevel logLevel, const std::string& logFilename,
               uint32_t logFileSizeLimit, uint64_t logDiskSpaceLimit) {
  if (logLevel == LogLevel::None) {
    currentLevel = LogLevel::None;
    return;
  }

  try {
    std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);

    if (logFilename.empty()) {
      if (logFileSizeLimit || logDiskSpaceLimit) {
        IllegalArgumentException ex(
            "Cannot specify a file or disk space size limit without specifying "
            "a log file name.");
        throw ex;
      }
      currentLevel = logLevel;
      getCurrentLogger()->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
      getCurrentLogger()->set_pattern(logLineFormat());
      return;
    } else if (logDiskSpaceLimit && logFileSizeLimit > logDiskSpaceLimit) {
      IllegalArgumentException ex(
          "File size limit must be smaller than disk space limit for "
          "logging.");
      throw ex;
    }

    int64_t adjustedDiskSpaceLimit;

    setSizeLimits(logFileSizeLimit, logDiskSpaceLimit, adjustedFileSizeLimit,
                  adjustedDiskSpaceLimit);
    maxFiles = calculateMaxFilesForSpaceLimit(adjustedDiskSpaceLimit,
                                              adjustedFileSizeLimit);
    currentLevel = logLevel;

    auto fullpath =
        boost::filesystem::absolute(boost::filesystem::path(logFilename));

    // if no extension then add .log extension
    if (fullpath.extension().empty() || (fullpath.extension() != ".log")) {
      fullpath = fullpath.string() + ".log";
    }

    // Ensure that directory exists for log files.  We're going to attempt
    // to iterate through files in that folder, and if it doesn't exist boost
    // will throw an exception.
    const auto target_path = fullpath.parent_path().string();
    if (!boost::filesystem::exists(target_path)) {
      boost::filesystem::create_directories(target_path);
    }

    logFilePath = fullpath.string();
    getCurrentLogger()->set_level(geodeLogLevelToSpdlogLevel(currentLevel));
    writeBanner();
    getCurrentLogger()->set_pattern(logLineFormat());
  } catch (const spdlog::spdlog_ex& ex) {
    throw IllegalArgumentException(ex.what());
  }
}  // namespace client

void Log::close() {
  if (currentLogger) {
    spdlog::drop("file");
    currentLogger = nullptr;
  }
}

LogLevel Log::stringToLogLevel(const std::string& levelName) {
  auto level = LogLevel::None;

  if (levelName.size()) {
    auto localLevelName = levelName;

    std::transform(localLevelName.begin(), localLevelName.end(),
                   localLevelName.begin(), ::tolower);

    if (localLevelName == "none") {
      level = LogLevel::None;
    } else if (localLevelName == "error") {
      level = LogLevel::Error;
    } else if (localLevelName == "warning") {
      level = LogLevel::Warning;
    } else if (localLevelName == "info") {
      level = LogLevel::Info;
    } else if (localLevelName == "default") {
      level = LogLevel::Default;
    } else if (localLevelName == "config") {
      level = LogLevel::Config;
    } else if (localLevelName == "fine") {
      level = LogLevel::Fine;
    } else if (localLevelName == "finer") {
      level = LogLevel::Finer;
    } else if (localLevelName == "finest") {
      level = LogLevel::Finest;
    } else if (localLevelName == "debug") {
      level = LogLevel::Debug;
    } else if (localLevelName == "all") {
      level = LogLevel::All;
    } else {
      throw IllegalArgumentException(
          ("Unexpected log level name: " + localLevelName).c_str());
    }
  }

  return level;
}

std::string Log::logLevelToString(LogLevel level) {
  std::string levelName = "None";
  switch (level) {
    case LogLevel::None:
      levelName = "none";
      break;
    case LogLevel::Error:
      levelName = "error";
      break;
    case LogLevel::Warning:
      levelName = "warning";
      break;
    case LogLevel::Info:
      levelName = "info";
      break;
    case LogLevel::Default:
      levelName = "default";
      break;
    case LogLevel::Config:
      levelName = "config";
      break;
    case LogLevel::Fine:
      levelName = "fine";
      break;
    case LogLevel::Finer:
      levelName = "finer";
      break;
    case LogLevel::Finest:
      levelName = "finest";
      break;
    case LogLevel::Debug:
      levelName = "debug";
      break;
    case LogLevel::All:
      levelName = "all";
      break;
  }
  return levelName;
}

LogLevel Log::logLevel() { return currentLevel; }

void Log::setLogLevel(LogLevel level) {
  currentLevel = level;
  getCurrentLogger()->set_level(geodeLogLevelToSpdlogLevel(level));
}

void Log::logInternal(LogLevel level, const std::string& msg) {
  getCurrentLogger()->log(geodeLogLevelToSpdlogLevel(level), msg);
}

void Log::log(LogLevel level, const std::string& msg) {
  Log::logInternal(level, std::string(msg));
}

void Log::log(LogLevel level, const char* fmt, ...) {
  char msg[_GEODE_LOG_MESSAGE_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized): clang-tidy bug
  std::vsnprintf(msg, sizeof(msg), fmt, argp);
  Log::logInternal(level, std::string(msg));
  va_end(argp);
}

void Log::logCatch(LogLevel level, const char* msg, const Exception& ex) {
  if (enabled(level)) {
    std::string message = "Geode exception " + ex.getName() +
                          " caught: " + ex.getMessage() + "\n" + msg;
    log(level, message);
  }
}

bool Log::enabled(LogLevel level) {
  return (level != LogLevel::None && level <= logLevel());
}

void Log::writeBanner() {
  std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);
  if (logLevel() != LogLevel::None) {
    getCurrentLogger()->set_pattern("%v");
    getCurrentLogger()->log(geodeLogLevelToSpdlogLevel(logLevel()),
                            geodeBanner::getBanner());
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
