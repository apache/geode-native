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
#include <cinttypes>
#include <ctime>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <ace/ACE.h>
#include <ace/Dirent_Selector.h>
#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>
#include <boost/regex.hpp>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

#include "../internal/hacks/AceThreadId.h"
#include "geodeBanner.hpp"
#include "util/chrono/time_point.hpp"

#if defined(_WIN32)
#include <io.h>
#define GF_FILEEXISTS(x) _access_s(x, 00)
#else
#include <unistd.h>
#define GF_FILEEXISTS(x) access(x, F_OK)
#endif

/*****************************************************************************/

/**
 * The implementation of the Log class
 *
 *
 */

/*****************************************************************************/

namespace apache {
namespace geode {
namespace log {
namespace globals {

static size_t g_bytesWritten = 0;

static size_t g_fileSizeLimit = GEODE_MAX_LOG_FILE_LIMIT;
static size_t g_diskSpaceLimit = GEODE_MAX_LOG_DISK_LIMIT;

static std::mutex g_logMutex;

static int g_rollIndex = 0;
static size_t g_spaceUsed = 0;
// Make a pair for the filename & its size
static std::pair<std::string, int64_t> g_fileInfoPair;
// Vector to hold the fileInformation
typedef std::vector<std::pair<std::string, int64_t> > g_fileInfo;

static boost::filesystem::path g_fullpath;

static FILE* g_log = nullptr;
static pid_t g_pid = 0;

const int __1K__ = 1024;
const int __1M__ = (__1K__ * __1K__);

}  // namespace globals
}  // namespace log
}  // namespace geode
}  // namespace apache

extern "C" {

static int selector(const dirent* d) {
  std::string inputname(d->d_name);
  std::string filebasename =
      apache::geode::log::globals::g_fullpath.filename().string();
  size_t actualHyphenPos = filebasename.find_last_of('.');
  if (strcmp(filebasename.c_str(), d->d_name) == 0) return 1;
  size_t fileExtPos = inputname.find_last_of('.');
  std::string extName = inputname.substr(fileExtPos + 1, inputname.length());
  if (strcmp(extName.c_str(), "log") != 0) return 0;
  if (fileExtPos != std::string::npos) {
    std::string tempname = inputname.substr(0, fileExtPos);
    size_t fileHyphenPos = tempname.find_last_of('-');
    if (fileHyphenPos != std::string::npos) {
      std::string buff1 = tempname.substr(0, fileHyphenPos);
      if (strstr(filebasename.c_str(), buff1.c_str()) == nullptr) {
        return 0;
      }
      if (fileHyphenPos != actualHyphenPos) return 0;
      std::string buff = tempname.substr(fileHyphenPos + 1,
                                         tempname.length() - fileHyphenPos - 1);
      for (std::string::iterator iter = buff.begin(); iter != buff.end();
           ++iter) {
        if (*iter < '0' || *iter > '9') {
          return 0;
        }
      }
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

static int comparator(const dirent** d1, const dirent** d2) {
  if (strlen((*d1)->d_name) < strlen((*d2)->d_name)) {
    return -1;
  } else if (strlen((*d1)->d_name) > strlen((*d2)->d_name)) {
    return 1;
  }

  int diff = std::strcmp((*d1)->d_name, (*d2)->d_name);
  if (diff < 0) {
    return -1;
  } else if (diff > 0) {
    return 1;
  } else {
    return 0;
  }
}
}

namespace apache {
namespace geode {
namespace client {

LogLevel Log::s_logLevel = LogLevel::Default;

using apache::geode::log::globals::__1K__;
using apache::geode::log::globals::__1M__;
using apache::geode::log::globals::g_bytesWritten;
using apache::geode::log::globals::g_diskSpaceLimit;
using apache::geode::log::globals::g_fileInfo;
using apache::geode::log::globals::g_fileInfoPair;
using apache::geode::log::globals::g_fileSizeLimit;
using apache::geode::log::globals::g_fullpath;
using apache::geode::log::globals::g_log;
using apache::geode::log::globals::g_logMutex;
using apache::geode::log::globals::g_pid;
using apache::geode::log::globals::g_rollIndex;
using apache::geode::log::globals::g_spaceUsed;

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

void Log::validateLogFileName(const std::string& filename) {
  auto nameToCheck = boost::filesystem::path(filename).filename().string();
  if (!boost::filesystem::portable_file_name(nameToCheck)) {
    throw IllegalArgumentException("Specified log file (" + nameToCheck +
                                   ") is not a valid portable name.");
  }
}

void Log::init(LogLevel level, const char* logFileName, int32_t logFileLimit,
               int64_t logDiskSpaceLimit) {
  init(level, std::string(logFileName), logFileLimit, logDiskSpaceLimit);
}

void rollLogFile() {
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
    boost::filesystem::rename(g_fullpath,
                              boost::filesystem::path(rollFileName));
    g_rollIndex++;
  } catch (const boost::filesystem::filesystem_error&) {
    throw IllegalStateException("Failed to roll log file");
  }
}

void setRollFileIndex() {
  const auto filterstring = g_fullpath.stem().string() + "-(\\d+)\\.log$";
  const boost::regex my_filter(filterstring);

  // Find the next roll index - will be highest present roll number + 1
  g_rollIndex = 0;
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
        if (index > g_rollIndex) {
          g_rollIndex = index + 1;
        }
      }
    }
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

  validateSizeLimits(logFileLimit, logDiskSpaceLimit);
  validateLogFileName(logFileName);

  std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);

  if (logFileName.empty()) {
    // TODO: What is supposed to happen when you specify an empty log file
    // name????
  } else {
    g_fullpath =
        boost::filesystem::absolute(boost::filesystem::path(logFileName));

    // if no extension then add .log extension
    if (g_fullpath.extension().empty()) {
      g_fullpath = g_fullpath.string() + ".log";
    } else if (g_fullpath.extension() != ".log") {
      g_fullpath = g_fullpath.string() + ".log";
    } else {
    }

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

    g_bytesWritten = 0;
    g_spaceUsed = 0;

    // Ensure that directory exists for log files.  We're going to attempt
    // to iterate through files in that folder, and if it doesn't exist boost
    // will throw an exception.
    const auto target_path = g_fullpath.parent_path().string();
    if (!boost::filesystem::exists(target_path)) {
      boost::filesystem::create_directories(target_path);
    }

    setRollFileIndex();

    if (boost::filesystem::exists(g_fullpath) && logFileLimit > 0) {
      rollLogFile();
    }
    writeBanner();
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
      fprintf(stdout, "%s", bannertext.c_str());
      fflush(stdout);
    } else {
      if (boost::filesystem::exists(
              g_fullpath.parent_path().string().c_str()) ||
          boost::filesystem::create_directories(g_fullpath.parent_path())) {
        g_log = fopen(g_fullpath.string().c_str(), "a");
        if (g_log) {
          if (fprintf(g_log, "%s", bannertext.c_str())) {
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
  if (g_pid == 0) {
    g_pid = boost::this_process::get_id();
  }
  const size_t MINBUFSIZE = 128;
  auto now = std::chrono::system_clock::now();
  auto secs = std::chrono::system_clock::to_time_t(now);
  auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
      now - std::chrono::system_clock::from_time_t(secs));
  auto tm_val = apache::geode::util::chrono::localtime(secs);
  msg << "[" << Log::levelToChars(level) << " ";
  char timebuf[MINBUFSIZE];

  std::strftime(timebuf, MINBUFSIZE, "%Y/%m/%d %H:%M:%S", &tm_val);
  msg << timebuf;

  std::snprintf(timebuf, 15, ".%06" PRId64 " ",
                static_cast<int64_t>(microseconds.count()));
  msg << timebuf << " ";

  std::strftime(timebuf, MINBUFSIZE, "%Z ", &tm_val);
  msg << timebuf << " " << boost::asio::ip::host_name() << ":"
      << boost::this_process::get_id() << " " << std::this_thread::get_id()
      << "] ";

  return msg.str();
}

void Log::put(LogLevel level, const char* msg) { put(level, std::string(msg)); }

// int g_count = 0;
void Log::put(LogLevel level, const std::string& msg) {
  std::lock_guard<decltype(g_logMutex)> guard(g_logMutex);

  g_fileInfo fileInfo;

  std::string buf;
  char fullpath[512] = {0};

  if (g_fullpath.string().empty()) {
    fprintf(stdout, "%s%s\n", formatLogLine(level).c_str(), msg.c_str());
    fflush(stdout);
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

      g_spaceUsed += g_bytesWritten;

      if ((g_diskSpaceLimit > 0) && (g_spaceUsed >= g_diskSpaceLimit)) {
        std::string dirname = g_fullpath.parent_path().string();
        g_spaceUsed = 0;
        ACE_stat statBuf = {};

        ACE_Dirent_Selector sds;
        int status = sds.open(dirname.c_str(), selector, comparator);
        if (status != -1) {
          for (int index = 1; index < sds.length(); ++index) {
            std::snprintf(fullpath, 512, "%s%c%s", dirname.c_str(),
                          ACE_DIRECTORY_SEPARATOR_CHAR, sds[index]->d_name);
            ACE_OS::stat(fullpath, &statBuf);
            g_fileInfoPair = std::make_pair(fullpath, statBuf.st_size);
            fileInfo.push_back(g_fileInfoPair);
            g_spaceUsed += fileInfo[index - 1].second;
          }  // for loop
          g_spaceUsed += g_bytesWritten;
          sds.close();
        }
        int fileIndex = 0;

        while ((g_spaceUsed > (g_diskSpaceLimit /*- g_fileSizeLimit*/))) {
          int64_t fileSize = fileInfo[fileIndex].second;
          if (ACE_OS::unlink(fileInfo[fileIndex].first.c_str()) == 0) {
            g_spaceUsed -= fileSize;
          } else {
            char printmsg[256];
            std::snprintf(printmsg, 256, "%s\t%s\n", "Could not delete",
                          fileInfo[fileIndex].first.c_str());
            numChars = fprintf(g_log, "%s%s\n", formatLogLine(level).c_str(),
                               printmsg);
            g_bytesWritten +=
                numChars + 2;  // bcoz we have to count trailing new line (\n)
          }
          fileIndex++;
        }
      }

      if ((numChars = fprintf(g_log, "%s%s\n", buf.c_str(), msg.c_str())) ==
              0 ||
          ferror(g_log)) {
        if ((g_diskSpaceLimit > 0)) {
          g_spaceUsed = g_spaceUsed - (numChars + 2);
        }
        if (g_fileSizeLimit > 0) {
          g_bytesWritten = g_bytesWritten - (numChars + 2);
        }

        // lets continue wothout throwing the exception; it should not cause
        // process to terminate
        fclose(g_log);
        g_log = nullptr;
      } else {
        fflush(g_log);
      }
    }
  }
}

void Log::putThrow(LogLevel level, const char* msg, const Exception& ex) {
  std::string message = "Geode exception " + ex.getName() +
                        " thrown: " + ex.getMessage() + "\n" + msg;
  put(level, message);
}

void Log::putCatch(LogLevel level, const char* msg, const Exception& ex) {
  std::string message = "Geode exception " + ex.getName() +
                        " caught: " + ex.getMessage() + "\n" + msg;
  put(level, message);
}

void Log::enterFn(LogLevel level, const char* functionName) {
  enum { MAX_NAME_LENGTH = 1024 };
  std::string fn = functionName;
  if (fn.size() > MAX_NAME_LENGTH) {
    fn = fn.substr(fn.size() - MAX_NAME_LENGTH, MAX_NAME_LENGTH);
  }
  char buf[MAX_NAME_LENGTH + 512] = {0};
  std::snprintf(buf, 1536, "{{{===>>> Entering function %s", fn.c_str());
  put(level, buf);
}

void Log::exitFn(LogLevel level, const char* functionName) {
  enum { MAX_NAME_LENGTH = 1024 };
  std::string fn = functionName;
  if (fn.size() > MAX_NAME_LENGTH) {
    fn = fn.substr(fn.size() - MAX_NAME_LENGTH, MAX_NAME_LENGTH);
  }
  char buf[MAX_NAME_LENGTH + 512] = {0};
  std::snprintf(buf, 1536, "<<<===}}} Exiting function %s", fn.c_str());
  put(level, buf);
}

bool Log::enabled(LogLevel level) {
  return GEODE_HIGHEST_LOG_LEVEL >= level && s_logLevel >= level;
}

void Log::log(LogLevel level, const char* msg) {
  if (enabled(level)) put(level, msg);
}

void Log::logThrow(LogLevel level, const char* msg, const Exception& ex) {
  if (enabled(level)) putThrow(level, msg, ex);
}

void Log::logCatch(LogLevel level, const char* msg, const Exception& ex) {
  if (enabled(level)) putCatch(level, msg, ex);
}

bool Log::errorEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Error &&
         s_logLevel >= LogLevel::Error;
}

void Log::error(const char* msg) {
  if (errorEnabled()) put(LogLevel::Error, msg);
}

void Log::error(const std::string& msg) {
  if (errorEnabled()) put(LogLevel::Error, msg.c_str());
}

void Log::errorThrow(const char* msg, const Exception& ex) {
  if (errorEnabled()) putThrow(LogLevel::Error, msg, ex);
}

void Log::errorCatch(const char* msg, const Exception& ex) {
  if (errorEnabled()) putCatch(LogLevel::Error, msg, ex);
}

bool Log::warningEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Warning &&
         s_logLevel >= LogLevel::Warning;
}

void Log::warning(const char* msg) {
  if (warningEnabled()) put(LogLevel::Warning, msg);
}

void Log::warningThrow(const char* msg, const Exception& ex) {
  if (warningEnabled()) putThrow(LogLevel::Warning, msg, ex);
}

void Log::warningCatch(const char* msg, const Exception& ex) {
  if (warningEnabled()) putCatch(LogLevel::Warning, msg, ex);
}

bool Log::infoEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Info &&
         s_logLevel >= LogLevel::Info;
}

void Log::info(const char* msg) {
  if (infoEnabled()) put(LogLevel::Info, msg);
}

void Log::infoThrow(const char* msg, const Exception& ex) {
  if (infoEnabled()) putThrow(LogLevel::Info, msg, ex);
}

void Log::infoCatch(const char* msg, const Exception& ex) {
  if (infoEnabled()) putCatch(LogLevel::Info, msg, ex);
}

bool Log::configEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Config &&
         s_logLevel >= LogLevel::Config;
}

void Log::config(const char* msg) {
  if (configEnabled()) put(LogLevel::Config, msg);
}

void Log::configThrow(const char* msg, const Exception& ex) {
  if (configEnabled()) putThrow(LogLevel::Config, msg, ex);
}

void Log::configCatch(const char* msg, const Exception& ex) {
  if (configEnabled()) putCatch(LogLevel::Config, msg, ex);
}

bool Log::fineEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Fine &&
         s_logLevel >= LogLevel::Fine;
}

void Log::fine(const char* msg) {
  if (fineEnabled()) put(LogLevel::Fine, msg);
}

void Log::fineThrow(const char* msg, const Exception& ex) {
  if (fineEnabled()) putThrow(LogLevel::Fine, msg, ex);
}

void Log::fineCatch(const char* msg, const Exception& ex) {
  if (fineEnabled()) putCatch(LogLevel::Fine, msg, ex);
}

bool Log::finerEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Finer &&
         s_logLevel >= LogLevel::Finer;
}

void Log::finer(const char* msg) {
  if (finerEnabled()) put(LogLevel::Finer, msg);
}

void Log::finerThrow(const char* msg, const Exception& ex) {
  if (finerEnabled()) putThrow(LogLevel::Finer, msg, ex);
}

void Log::finerCatch(const char* msg, const Exception& ex) {
  if (finerEnabled()) putCatch(LogLevel::Finer, msg, ex);
}

bool Log::finestEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Finest &&
         s_logLevel >= LogLevel::Finest;
}

void Log::finest(const char* msg) {
  if (finestEnabled()) put(LogLevel::Finest, msg);
}

void Log::finestThrow(const char* msg, const Exception& ex) {
  if (finestEnabled()) putThrow(LogLevel::Finest, msg, ex);
}

void Log::finestCatch(const char* msg, const Exception& ex) {
  if (finestEnabled()) putCatch(LogLevel::Finest, msg, ex);
}

bool Log::debugEnabled() {
  return GEODE_HIGHEST_LOG_LEVEL >= LogLevel::Debug &&
         s_logLevel >= LogLevel::Debug;
}

void Log::debug(const char* msg) {
  if (debugEnabled()) put(LogLevel::Debug, msg);
}

void Log::debugThrow(const char* msg, const Exception& ex) {
  if (debugEnabled()) putThrow(LogLevel::Debug, msg, ex);
}

void Log::debugCatch(const char* msg, const Exception& ex) {
  if (debugEnabled()) putCatch(LogLevel::Debug, msg, ex);
}

LogFn::LogFn(const char* functionName, LogLevel level)
    : m_functionName(functionName), m_level(level) {
  if (Log::enabled(m_level)) Log::enterFn(m_level, m_functionName);
}

LogFn::~LogFn() {
  if (Log::enabled(m_level)) Log::exitFn(m_level, m_functionName);
}

// var arg logging routines.

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

void LogVarargs::debug(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Debug, msg);
  va_end(argp);
}

void LogVarargs::error(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Error, msg);
  va_end(argp);
}

void LogVarargs::warn(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Warning, msg);
  va_end(argp);
}

void LogVarargs::info(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Info, msg);
  va_end(argp);
}

void LogVarargs::config(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Config, msg);
  va_end(argp);
}

void LogVarargs::fine(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Fine, msg);
  va_end(argp);
}

void LogVarargs::finer(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Finer, msg);
  va_end(argp);
}

void LogVarargs::finest(const char* fmt, ...) {
  char msg[_GF_MSG_LIMIT] = {0};
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(msg, _GF_MSG_LIMIT, fmt, argp);
  /* win doesn't guarantee termination */ msg[_GF_MSG_LIMIT - 1] = '\0';
  Log::put(LogLevel::Finest, msg);
  va_end(argp);
}

void LogVarargs::debug(const std::string& message) {
  Log::put(LogLevel::Debug, message.c_str());
}

void LogVarargs::error(const std::string& message) {
  Log::put(LogLevel::Error, message.c_str());
}

void LogVarargs::warn(const std::string& message) {
  Log::put(LogLevel::Warning, message.c_str());
}

void LogVarargs::info(const std::string& message) {
  Log::put(LogLevel::Info, message.c_str());
}

void LogVarargs::config(const std::string& message) {
  Log::put(LogLevel::Config, message.c_str());
}

void LogVarargs::fine(const std::string& message) {
  Log::put(LogLevel::Fine, message.c_str());
}

void LogVarargs::finer(const std::string& message) {
  Log::put(LogLevel::Finer, message.c_str());
}

void LogVarargs::finest(const std::string& message) {
  Log::put(LogLevel::Finest, message.c_str());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
