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

// spdlog headers are incompatible with managed code, so all spdlog
// references here are defined out for clicache code.
#ifndef _MANAGED
#include <spdlog/spdlog.h>
#endif  // !_MANAGED

#include <cstdarg>
#include <cstdio>
#include <string>

#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

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

#ifndef _MANAGED
  static const std::shared_ptr<spdlog::logger>& getCurrentLogger();
#endif  // !_MANAGED

 private:
  static void validateSizeLimits(int64_t fileSizeLimit, int64_t diskSpaceLimit);
  static void setSizeLimits(int32_t logFileLimit, int64_t logDiskSpaceLimit,
                            int32_t& adjustedFileLimit,
                            int64_t& adjustedDiskLimit);
  static uint32_t calculateMaxFilesForSpaceLimit(uint64_t logDiskSpaceLimit,
                                                 uint32_t logFileSizeLimit);
  static void writeBanner();
  static void logInternal(LogLevel level, const std::string& msg);
  static void createLoggerObject(int32_t fileSizeLimit = 0,
                                 int32_t maxFiles = 0,
                                 const std::string& path = "-");

  static std::string logLineFormat();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#ifdef _MANAGED
#define LOG_ERROR(...)
#define LOG_WARN(...)
#define LOG_INFO(...)
#define LOG_CONFIG(...)
#define LOG_FINE(...)
#define LOG_FINER(...)
#define LOG_FINEST(...)
#define LOG_DEBUG(...)
#else
#define LOG_ERROR(...)                                             \
  do {                                                             \
    using ::apache::geode::client::Log;                            \
    using ::apache::geode::client::LogLevel;                       \
    if (Log::enabled(LogLevel::Error)) {                           \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::err, \
                                   __VA_ARGS__);                   \
    }                                                              \
  } while (false)

#define LOG_WARN(...)                                               \
  do {                                                              \
    using ::apache::geode::client::Log;                             \
    using ::apache::geode::client::LogLevel;                        \
    if (Log::enabled(LogLevel::Warning)) {                          \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::warn, \
                                   __VA_ARGS__);                    \
    }                                                               \
  } while (false)

#define LOG_INFO(...)                                               \
  do {                                                              \
    using ::apache::geode::client::Log;                             \
    using ::apache::geode::client::LogLevel;                        \
    if (Log::enabled(LogLevel::Info)) {                             \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::info, \
                                   __VA_ARGS__);                    \
    }                                                               \
  } while (false)

#define LOG_CONFIG(...)                                             \
  do {                                                              \
    using ::apache::geode::client::Log;                             \
    using ::apache::geode::client::LogLevel;                        \
    if (Log::enabled(LogLevel::Config)) {                           \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::info, \
                                   __VA_ARGS__);                    \
    }                                                               \
  } while (false)

#define LOG_FINE(...)                                                \
  do {                                                               \
    using ::apache::geode::client::Log;                              \
    using ::apache::geode::client::LogLevel;                         \
    if (Log::enabled(LogLevel::Fine)) {                              \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::debug, \
                                   __VA_ARGS__);                     \
    }                                                                \
  } while (false)

#define LOG_FINER(...)                                               \
  do {                                                               \
    using ::apache::geode::client::Log;                              \
    using ::apache::geode::client::LogLevel;                         \
    if (Log::enabled(LogLevel::Finer)) {                             \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::debug, \
                                   __VA_ARGS__);                     \
    }                                                                \
  } while (false)

#define LOG_FINEST(...)                                              \
  do {                                                               \
    using ::apache::geode::client::Log;                              \
    using ::apache::geode::client::LogLevel;                         \
    if (Log::enabled(LogLevel::Finest)) {                            \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::debug, \
                                   __VA_ARGS__);                     \
    }                                                                \
  } while (false)

#define LOG_DEBUG(...)                                               \
  do {                                                               \
    using ::apache::geode::client::Log;                              \
    using ::apache::geode::client::LogLevel;                         \
    if (Log::enabled(LogLevel::Debug)) {                             \
      Log::getCurrentLogger()->log(spdlog::level::level_enum::debug, \
                                   __VA_ARGS__);                     \
    }                                                                \
  } while (false)
#endif  // !_MANAGED

#endif  // GEODE_LOG_H_
