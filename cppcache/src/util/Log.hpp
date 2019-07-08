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

#pragma once

#ifndef GEODE_LOG_H_
#define GEODE_LOG_H_

#include <cstdarg>
#include <cstdio>
#include <string>

#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

#include "spdlog/spdlog.h"

#ifndef GEODE_HIGHEST_LOG_LEVEL
#define GEODE_HIGHEST_LOG_LEVEL LogLevel::All
#endif

#ifndef GEODE_MAX_LOG_FILE_LIMIT
#define GEODE_MAX_LOG_FILE_LIMIT (1024 * 1024 * 1024)
#endif

#ifndef GEODE_MAX_LOG_DISK_LIMIT
#define GEODE_MAX_LOG_DISK_LIMIT (1024ll * 1024ll * 1024ll * 1024ll)
#endif

#define _GEODE_LOG_MESSAGE_LIMIT 8192

namespace apache {
namespace geode {
namespace client {

class Exception;

/** Defines methods available to clients that want to write a log message
 * to their Geode system's shared log file.
 * <p>
 * This class must be initialized prior to its use:
 * @ref Log::init
 * <p>
 * For any logged message the log file will contain:
 * <ul>
 * <li> The message's level.
 * <li> The time the message was logged.
 * <li> The id of the connection and thread that logged the message.
 * <li> The message itself which can be a const char* (perhaps with
 * an exception including the exception's stack trace.
 * </ul>
 * <p>
 * A message always has a level.
 * Logging levels are ordered. Enabling logging at a given level also
 * enables logging at higher levels. The higher the level the more
 * important and urgent the message.
 * <p>
 * The levels, in descending order of severity, are:
 * <ul>
 *
 * <li> <code>error</code> (highest severity) is a message level
 * indicating a serious failure.  In general <code>error</code>
 * messages should describe events that are of considerable
 * importance and which will prevent normal program execution. They
 * should be reasonably intelligible to end users and to system
 * administrators.
 *
 * <li> <code>warning</code> is a message level indicating a
 * potential problem.  In general <code>warning</code> messages
 * should describe events that will be of interest to end users or
 * system managers, or which indicate potential problems.
 *
 * <li> <code>info</code> is a message level for informational
 * messages.  Typically <code>info</code> messages should be
 * reasonably significant and should make sense to end users and
 * system administrators.
 *
 * <li> <code>config</code> is a message level for static
 * configuration messages.  <code>config</code> messages are intended
 * to provide a variety of static configuration information, to
 * assist in debugging problems that may be associated with
 * particular configurations.
 *
 * <li> <code>fine</code> is a message level providing tracing
 * information.  In general the <code>fine</code> level should be
 * used for information that will be broadly interesting to
 * developers. This level is for the lowest volume, and most
 * important, tracing messages.
 *
 * <li> <code>finer</code> indicates a moderately detailed tracing
 * message.  This is an intermediate level between <code>fine</code>
 * and <code>finest</code>.
 *
 * <li> <code>finest</code> indicates a very detailed tracing
 * message.  Logging calls for entering, returning, or throwing an
 * exception are traced at the <code>finest</code> level.
 *
 * <li> <code>debug</code> (lowest severity) indicates a highly
 * detailed tracing message.  In general the <code>debug</code> level
 * should be used for the most voluminous detailed tracing messages.
 * </ul>
 *
 * <p>
 * For each level methods exist that will request a message, at that
 * level, to be logged. These methods are all named after their level.
 * <p>
 * For each level a method exists that indicates if messages at that
 * level will currently be logged. The names of these methods are of
 * the form: <em>level</em><code>Enabled</code>.
 *
 *
 */

class APACHE_GEODE_EXPORT Log {
 public:
  /**
   * Returns the current log level.
   */
  static LogLevel logLevel();

  /**
   * Set the current log level.
   */
  static void setLogLevel(LogLevel level);

  /**
   * Initializes logging facility with given level and filenames.
   * This method is called automatically within @ref DistributedSystem::connect
   * with the log-file, log-level, and log-file-size system properties used as
   * arguments
   */
  static void init(LogLevel level, const char* logFileName,
                   int32_t logFileLimit = 0, int64_t logDiskSpaceLimit = 0);

  static void init
      // 0 => use maximum value (currently 1G)
      (LogLevel level, const std::string& logFileName = "",
       uint32_t logFileLimit = 0, uint64_t logDiskSpaceLimit = 0);

  /**
   * closes logging facility (until next init).
   */
  static void close();

  /**
   * returns character string for given log level. The string will be
   * identical to the enum declaration above, except it will be all
   * lower case. Out of range values will throw
   * IllegalArgumentException.
   */
  static std::string logLevelToString(LogLevel level);

  /**
   * returns log level specified by "chars", or throws
   * IllegalArgumentException.  Allowed values are identical to the
   * enum declaration above for LogLevel, but with character case ignored.
   */
  static LogLevel stringToLogLevel(const std::string& chars);

  static void log(LogLevel level, const std::string& msg);
  static void log(LogLevel level, const char* fmt, ...);
  static void logCatch(LogLevel level, const char* msg, const Exception& ex);

  static bool enabled(LogLevel level);

 private:
  static void validateSizeLimits(int64_t fileSizeLimit, int64_t diskSpaceLimit);
  static void setSizeLimits(int32_t logFileLimit, int64_t logDiskSpaceLimit,
                            int32_t& adjustedFileLimit,
                            int64_t& adjustedDiskLimit);
  static uint32_t calculateMaxFilesForSpaceLimit(uint64_t logDiskSpaceLimit,
                                                 uint32_t logFileSizeLimit);
  static void writeBanner();
  static void logInternal(LogLevel level, const std::string& msg);

  static void calculateUsedDiskSpace();
  static std::string logLineFormat();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#define LOGERROR(...)                                             \
  do {                                                            \
    if (::apache::geode::client::Log::enabled(                    \
            apache::geode::client::LogLevel::Error)) {            \
      ::apache::geode::client::Log::log(                          \
          ::apache::geode::client::LogLevel::Error, __VA_ARGS__); \
    }                                                             \
  } while (false)

#define LOGWARN(...)                                                \
  do {                                                              \
    if (::apache::geode::client::Log::enabled(                      \
            apache::geode::client::LogLevel::Warning)) {            \
      ::apache::geode::client::Log::log(                            \
          ::apache::geode::client::LogLevel::Warning, __VA_ARGS__); \
    }                                                               \
  } while (false)

#define LOGINFO(...)                                             \
  do {                                                           \
    if (::apache::geode::client::Log::enabled(                   \
            apache::geode::client::LogLevel::Info)) {            \
      ::apache::geode::client::Log::log(                         \
          ::apache::geode::client::LogLevel::Info, __VA_ARGS__); \
    }                                                            \
  } while (false)

#define LOGCONFIG(...)                                             \
  do {                                                             \
    if (::apache::geode::client::Log::enabled(                     \
            apache::geode::client::LogLevel::Config)) {            \
      ::apache::geode::client::Log::log(                           \
          ::apache::geode::client::LogLevel::Config, __VA_ARGS__); \
    }                                                              \
  } while (false)

#define LOGFINE(...)                                             \
  do {                                                           \
    if (::apache::geode::client::Log::enabled(                   \
            apache::geode::client::LogLevel::Fine)) {            \
      ::apache::geode::client::Log::log(                         \
          ::apache::geode::client::LogLevel::Fine, __VA_ARGS__); \
    }                                                            \
  } while (false)

#define LOGFINER(...)                                             \
  do {                                                            \
    if (::apache::geode::client::Log::enabled(                    \
            apache::geode::client::LogLevel::Finer)) {            \
      ::apache::geode::client::Log::log(                          \
          ::apache::geode::client::LogLevel::Finer, __VA_ARGS__); \
    }                                                             \
  } while (false)

#define LOGFINEST(...)                                             \
  do {                                                             \
    if (::apache::geode::client::Log::enabled(                     \
            apache::geode::client::LogLevel::Finest)) {            \
      ::apache::geode::client::Log::log(                           \
          ::apache::geode::client::LogLevel::Finest, __VA_ARGS__); \
    }                                                              \
  } while (false)

#define LOGDEBUG(...)                                             \
  do {                                                            \
    if (::apache::geode::client::Log::enabled(                    \
            apache::geode::client::LogLevel::Debug)) {            \
      ::apache::geode::client::Log::log(                          \
          ::apache::geode::client::LogLevel::Debug, __VA_ARGS__); \
    }                                                             \
  } while (false)

#endif  // GEODE_LOG_H_
