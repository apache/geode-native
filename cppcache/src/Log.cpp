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

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>
#include <boost/regex.hpp>

#include <geode/ExceptionTypes.hpp>
#include <geode/util/LogLevel.hpp>

#include "DistributedSystemImpl.hpp"
#include "geodeBanner.hpp"
#include "util/chrono/time_point.hpp"

namespace {

static size_t g_bytesWritten = 0;

static size_t g_fileSizeLimit = GEODE_MAX_LOG_FILE_LIMIT;
static size_t g_diskSpaceLimit = GEODE_MAX_LOG_DISK_LIMIT;

static std::mutex g_logMutex;

static int g_rollIndex = 0;
static size_t g_spaceUsed = 0;

static boost::filesystem::path g_fullpath;
static std::map<int32_t, boost::filesystem::path> g_rollFiles;

static FILE* g_log = nullptr;

static std::string g_hostName;

const int __1K__ = 1024;
const int __1M__ = (__1K__ * __1K__);

}  // namespace

namespace apache {
namespace geode {
namespace client {

LogLevel Log::s_logLevel = LogLevel::Default;

/*****************************************************************************/

LogLevel Log::logLevel() { return s_logLevel; }

/**
 * Set the current log level.
 */
void Log::setLogLevel(LogLevel level) { s_logLevel = level; }

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

void Log::rollLogFile() {
  if (g_log) {
    fclose(g_log);
    g_log = nullptr;
  }

  auto rollFileName =
      (g_fullpath.parent_path() /
       (g_fullpath.stem().string() + "-" + std::to_string(g_rollIndex) +
        g_fullpath.extension().string()))
          .string();
  try {
    auto rollFile = boost::filesystem::path(rollFileName);
    boost::filesystem::rename(g_fullpath, rollFile);
    g_rollFiles[g_rollIndex] = rollFile;
    g_rollIndex++;
  } catch (const boost::filesystem::filesystem_error&) {
    throw IllegalStateException("Failed to roll log file");
  }
}

void Log::removeOldestRolledLogFile() {
  if (g_rollFiles.size()) {
    auto index = g_rollFiles.begin()->first;
    auto fileToRemove = g_rollFiles.begin()->second;
    auto fileSize = boost::filesystem::file_size(fileToRemove);
    boost::filesystem::remove(fileToRemove);
    g_rollFiles.erase(index);
    g_spaceUsed -= fileSize;
  } else {
    throw IllegalStateException(
        "Failed to free sufficient disk space for logs");
  }
}

void Log::calculateUsedDiskSpace() {
  g_spaceUsed = 0;
  if (boost::filesystem::exists(g_fullpath)) {
    g_spaceUsed = boost::filesystem::file_size(g_fullpath);
    for (auto const& item : g_rollFiles) {
      g_spaceUsed += boost::filesystem::file_size(item.second);
    }
  }
}

void Log::buildRollFileMapping() {
  const auto filterstring = g_fullpath.stem().string() + "-(\\d+)\\.log$";
  const boost::regex my_filter(filterstring);

  g_rollFiles.clear();

  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator i(
           g_fullpath.parent_path().string());
       i != end_itr; ++i) {
    if (boost::filesystem::is_regular_file(i->status())) {
      std::string filename = i->path().filename().string();
      boost::regex testPattern(filterstring);
      boost::match_results<std::string::const_iterator> testMatches;
      if (boost::regex_search(std::string::const_iterator(filename.begin()),
                              filename.cend(), testMatches, testPattern)) {
        auto index = std::atoi(
            std::string(testMatches[1].first, testMatches[1].second).c_str());
        g_rollFiles[index] = i->path();
      }
    }
  }
}

void Log::setRollFileIndex() {
  g_rollIndex = 0;
  if (g_rollFiles.size()) {
    g_rollIndex = g_rollFiles.rbegin()->first + 1;
  }
}

void Log::setSizeLimits(int32_t logFileLimit, int64_t logDiskSpaceLimit) {
  validateSizeLimits(logFileLimit, logDiskSpaceLimit);

  // Default to 10MB file limit and 1GB disk limit
  if (logFileLimit == 0 && logDiskSpaceLimit == 0) {
    g_fileSizeLimit = 10 * __1M__;
    g_diskSpaceLimit = 1000 * __1M__;
  }
  // disk space specified but file size is defaulted.  Just use a single
  // log file, i.e. set file limit == disk limit
  else if (logFileLimit == 0) {
    g_diskSpaceLimit = logDiskSpaceLimit * __1M__;
    g_fileSizeLimit = g_diskSpaceLimit;
  } else if (logDiskSpaceLimit == 0) {
    g_fileSizeLimit = logFileLimit * __1M__;
    g_diskSpaceLimit = g_fileSizeLimit;
  } else {
    g_fileSizeLimit = logFileLimit * __1M__;
    g_diskSpaceLimit = logDiskSpaceLimit * __1M__;
  }
}

void Log::init(LogLevel level, const std::string& logFileName,
               int32_t logFileLimit, int64_t logDiskSpaceLimit) {
  if (g_log != nullptr) {
    throw IllegalStateException(
        "The Log has already been initialized. "
        "Call Log::close() before calling Log::init again.");
  }
  s_logLevel = level;

  try {
    std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);

    g_hostName = boost::asio::ip::host_name();

    g_fullpath =
        boost::filesystem::absolute(boost::filesystem::path(logFileName));

    // if no extension then add .log extension
    if (g_fullpath.extension().empty() || (g_fullpath.extension() != ".log")) {
      g_fullpath = g_fullpath.string() + ".log";
    }

    setSizeLimits(logFileLimit, logDiskSpaceLimit);

    g_bytesWritten = 0;
    g_spaceUsed = 0;

    // Ensure that directory exists for log files.  We're going to attempt
    // to iterate through files in that folder, and if it doesn't exist boost
    // will throw an exception.
    const auto target_path = g_fullpath.parent_path().string();
    if (!boost::filesystem::exists(target_path)) {
      boost::filesystem::create_directories(target_path);
    }

    buildRollFileMapping();
    setRollFileIndex();
    calculateUsedDiskSpace();
    while (g_spaceUsed > g_diskSpaceLimit) {
      removeOldestRolledLogFile();
    }

    if (boost::filesystem::exists(g_fullpath) && logFileLimit > 0) {
      rollLogFile();
    }
    writeBanner();
  } catch (const boost::exception&) {
    auto msg = std::string("Unable to log to file '") + logFileName + "'";
    throw IllegalArgumentException(msg.c_str());
  } catch (const std::exception& ex) {
    auto msg = std::string("Unable to log to file '") + logFileName +
               "': " + ex.what();
    throw IllegalArgumentException(msg.c_str());
  }
}

void Log::close() {
  std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);

  if (g_log) {
    fclose(g_log);
    g_log = nullptr;
  }
  g_fullpath = "";
}

void Log::writeBanner() {
  if (s_logLevel != LogLevel::None) {
    std::string bannertext = geodeBanner::getBanner();

    // fullpath empty --> we're logging to stdout
    if (g_fullpath.string().empty()) {
      std::cout << bannertext << std::flush;
    } else {
      if (boost::filesystem::exists(
              g_fullpath.parent_path().string().c_str()) ||
          boost::filesystem::create_directories(g_fullpath.parent_path())) {
        g_log = fopen(g_fullpath.string().c_str(), "a");
        if (g_log) {
          if (fwrite(bannertext.c_str(), sizeof(char), bannertext.length(),
                     g_log) == bannertext.length()) {
            g_bytesWritten += static_cast<int32_t>(bannertext.length());
            fflush(g_log);
          }
        }
      }
    }
  }
}

const char* Log::levelToChars(LogLevel level) {
  switch (level) {
    case LogLevel::None:
      return "none";

    case LogLevel::Error:
      return "error";

    case LogLevel::Warning:
      return "warning";

    case LogLevel::Info:
      return "info";

    case LogLevel::Default:
      return "default";

    case LogLevel::Config:
      return "config";

    case LogLevel::Fine:
      return "fine";

    case LogLevel::Finer:
      return "finer";

    case LogLevel::Finest:
      return "finest";

    case LogLevel::Debug:
      return "debug";

    case LogLevel::All:
      return "all";
  }
  throw IllegalArgumentException(std::string("Unexpected log level: ") +
                                 std::to_string(static_cast<int>(level)));
}

LogLevel Log::charsToLevel(const std::string& chars) {
  std::string level = chars;

  if (level.empty()) return LogLevel::None;

  std::transform(level.begin(), level.end(), level.begin(), ::tolower);

  if (level == "none") {
    return LogLevel::None;
  } else if (level == "error") {
    return LogLevel::Error;
  } else if (level == "warning") {
    return LogLevel::Warning;
  } else if (level == "info") {
    return LogLevel::Info;
  } else if (level == "default") {
    return LogLevel::Default;
  } else if (level == "config") {
    return LogLevel::Config;
  } else if (level == "fine") {
    return LogLevel::Fine;
  } else if (level == "finer") {
    return LogLevel::Finer;
  } else if (level == "finest") {
    return LogLevel::Finest;
  } else if (level == "debug") {
    return LogLevel::Debug;
  } else if (level == "all") {
    return LogLevel::All;
  } else {
    throw IllegalArgumentException(("Unexpected log level: " + level).c_str());
  }
}

std::string Log::formatLogLine(LogLevel level) {
  std::stringstream msg;
  const size_t MINBUFSIZE = 128;
  auto now = std::chrono::system_clock::now();
  auto secs = std::chrono::system_clock::to_time_t(now);
  auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
      now - std::chrono::system_clock::from_time_t(secs));
  auto tm_val = apache::geode::util::chrono::localtime(secs);

  msg << "[" << Log::levelToChars(level) << " "
      << std::put_time(&tm_val, "%Y/%m/%d %H:%M:%S") << '.' << std::setfill('0')
      << std::setw(6) << microseconds.count() << ' '
      << std::put_time(&tm_val, "%z  ") << g_hostName << ":"
      << boost::this_process::get_id() << " "
      << DistributedSystemImpl::getThreadName(std::this_thread::get_id())
      << "] ";

  return msg.str();
}

void Log::log(LogLevel level, const std::string& msg) {
  Log::logInternal(level, msg);
}

void Log::logInternal(LogLevel level, const std::string& msg) {
  std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);

  std::string buf;
  char fullpath[512] = {0};

  if (g_fullpath.string().empty()) {
    std::cout << formatLogLine(level) << msg << "\n" << std::flush;
  } else {
    if (!g_log) {
      g_log = fopen(g_fullpath.string().c_str(), "a");
    }

    if (g_log) {
      buf = formatLogLine(level);
      auto numChars = static_cast<int>(buf.length() + msg.length());
      g_bytesWritten +=
          numChars + 2;  // bcoz we have to count trailing new line (\n)

      if ((g_fileSizeLimit != 0) && (g_bytesWritten >= g_fileSizeLimit)) {
        rollLogFile();
        g_bytesWritten = numChars + 2;  // Account for trailing newline
        writeBanner();
      }

      g_spaceUsed += numChars + 2;

      // Remove existing rolled log files until we're below the limit
      while (g_spaceUsed >= g_diskSpaceLimit) {
        removeOldestRolledLogFile();
      }

      auto logLine = buf + msg + "\n";
      if (fwrite(logLine.c_str(), sizeof(char), logLine.length(), g_log) !=
              logLine.length() ||
          ferror(g_log)) {
        // Let's continue without throwing the exception.  It should not cause
        // process to terminate
        fclose(g_log);
        g_log = nullptr;
      } else {
        fflush(g_log);
      }
    }
  }
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

}  // namespace client
}  // namespace geode
}  // namespace apache
