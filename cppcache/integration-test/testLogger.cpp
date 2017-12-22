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

#define ROOT_NAME "testLogger"

#include "fw_helper.hpp"
#include <geode/ExceptionTypes.hpp>

#ifndef WIN32
#include <unistd.h>
#endif

#define LENGTH_OF_BANNER 16

using namespace apache::geode::client;

int numOfLinesInFile(const char* fname) {
  char line[2048];
  char* read;
  int ln_cnt = 0;
  FILE* fp = fopen(fname, "r");
  if (fp == nullptr) {
    return -1;
  }
  while (!!(read = fgets(line, sizeof line, fp))) {
    printf("%d:%s", ++ln_cnt, line);
  }

  if (!feof(fp)) {
    fclose(fp);
    return -2;
  }
  fclose(fp);
  return ln_cnt;
}

void testLogFnError() {
  LogFn logFn("TestLogger::testLogFnError", LogLevel::Error);
  Log::error("...");
}

void testLogFnWarning() {
  LogFn logFn("TestLogger::testLogFnWarning", LogLevel::Warning);
  Log::warning("...");
}

void testLogFnInfo() {
  LogFn logFn("TestLogger::testLogFnInfo", LogLevel::Info);
  Log::info("...");
}

void testLogFnConfig() {
  LogFn logFn("TestLogger::testLogFnConfig", LogLevel::Config);
  Log::config("...");
}

void testLogFnFine() {
  LogFn logFn("TestLogger::testLogFnFine", LogLevel::Fine);
  Log::fine("...");
}

void testLogFnFiner() {
  LogFn logFn("TestLogger::testLogFnFiner", LogLevel::Finer);
  Log::finer("...");
}

void testLogFnFinest() {
  LogFn logFn("TestLogger::testLogFnFinest");
  Log::finest("...");
}

void testLogFnDebug() {
  LogFn logFn("TestLogger::testLogFnDebug", LogLevel::Debug);
  Log::debug("...");
}

int expected(int level) {
  int expected = level;
  if (level != LogLevel::None) {
    expected += LENGTH_OF_BANNER;
  }
  if (level >= LogLevel::Default) {
    expected--;
  }
  return expected;
}

BEGIN_TEST(REINIT)
  {
    LOGINFO("Started logging");
    int exceptiongot = 0;
    Log::init(LogLevel::Debug, "logfile");
    try {
      Log::init(LogLevel::Debug, "logfile1");
    } catch (IllegalStateException& ex) {
      printf("Got Illegal state exception while calling init again\n");
      printf("Exception mesage = %s\n", ex.what());
      exceptiongot = 1;
    }
    ASSERT(exceptiongot == 1, "expected exceptiongot to be 1");
    Log::close();
    unlink("logfile.log");
  }
END_TEST(REINIT)

BEGIN_TEST(ALL_LEVEL)
  {
    for (LogLevel level = Error; level <= Debug;
         level = LogLevel(level + 1)) {
      Log::init(level, "all_logfile");

      Log::error("Error Message");
      Log::warning("Warning Message");
      Log::info("Info Message");
      Log::config("Config Message");
      Log::fine("Fine Message");
      Log::finer("Finer Message");
      Log::finest("Finest Message");
      Log::debug("Debug Message");

      int lines = numOfLinesInFile("all_logfile.log");

      printf("lines = %d expected = %d \n", lines, expected(level));

      ASSERT(lines == expected(level), "Wrong number of lines");

      Log::close();
      unlink("all_logfile.log");
    }
  }
END_TEST(ALL_LEVEL)

BEGIN_TEST(ALL_LEVEL_MACRO)
  {
    for (LogLevel level = Error; level <= Debug;
         level = LogLevel(level + 1)) {
      Log::init(level, "all_logfile");

      LOGERROR("Error Message");
      LOGWARN("Warning Message");
      LOGINFO("Info Message");
      LOGCONFIG("Config Message");
      LOGFINE("Fine Message");
      LOGFINER("Finer Message");
      LOGFINEST("Finest Message");
      LOGDEBUG("Debug Message");

      int lines = numOfLinesInFile("all_logfile.log");

      printf("lines = %d Level = %d, %d\n", lines, static_cast<int>(level),
             Log::logLevel());

      ASSERT(lines == expected(level), "Wrong number of lines");
      Log::close();

      unlink("all_logfile.log");
    }
  }
END_TEST(ALL_LEVEL_MACRO)

BEGIN_TEST(FILE_LIMIT)
  {
#ifdef _WIN32
// Fail to roll file over to timestamp file on windows.
#else
    for (LogLevel level = Error; level <= Debug;
         level = LogLevel(level + 1)) {
      if (level == Default) continue;
      Log::init(level, "logfile", 1);

      Log::error("Error Message");
      Log::warning("Warning Message");
      Log::info("Info Message");
      Log::config("Config Message");
      Log::fine("Fine Message");
      Log::finer("Finer Message");
      Log::finest("Finest Message");
      Log::debug("Debug Message");

      int lines = numOfLinesInFile("logfile.log");
      int expectedLines =
          level + LENGTH_OF_BANNER - (level >= Default ? 1 : 0);
      printf("lines = %d expectedLines = %d level = %d\n", lines, expectedLines,
             level);

      ASSERT(lines == expectedLines, "Wrong number of lines");

      Log::close();
      unlink("logfile.log");
    }
#endif
  }
END_TEST(FILE_LIMIT)

BEGIN_TEST(CONFIG_ONWARDS)
  {
    Log::init(LogLevel::Config, "logfile");

    Log::debug("Debug Message");
    Log::config("Config Message");
    Log::info("Info Message");
    Log::warning("Warning Message");
    Log::error("Error Message");

    int lines = numOfLinesInFile("logfile.log");
    printf("lines = %d\n", lines);
    // debug should not be printed
    ASSERT(lines == 4 + LENGTH_OF_BANNER,
           "Expected 4 + LENGTH_OF_BANNER lines.");
    Log::close();
    unlink("logfile.log");
  }
END_TEST(CONFIG_ONWARDS)

BEGIN_TEST(INFO_ONWARDS)
  {
    Log::init(LogLevel::Info, "logfile");

    Log::debug("Debug Message");
    Log::config("Config Message");
    Log::info("Info Message");
    Log::warning("Warning Message");
    Log::error("Error Message");

    int lines = numOfLinesInFile("logfile.log");
    printf("lines = %d\n", lines);
    // debug, config should not be printed
    ASSERT(lines == 3 + LENGTH_OF_BANNER,
           "Expected 3 + LENGTH_OF_BANNER lines.");
    Log::close();
    unlink("logfile.log");
  }
END_TEST(INFO_ONWARDS)

BEGIN_TEST(WARNING_ONWARDS)
  {
    Log::init(LogLevel::Warning, "logfile");

    Log::debug("Debug Message");
    Log::config("Config Message");
    Log::info("Info Message");
    Log::warning("Warning Message");
    Log::error("Error Message");

    int lines = numOfLinesInFile("logfile.log");
    printf("lines = %d\n", lines);
    // debug, config, info should not be printed
    ASSERT(lines == 2 + LENGTH_OF_BANNER,
           "Expected 2 + LENGTH_OF_BANNER lines.");
    Log::close();
    unlink("logfile.log");
  }
END_TEST(WARNING_ONWARDS)

BEGIN_TEST(ERROR_LEVEL)
  {
    Log::init(LogLevel::Error, "logfile");

    Log::debug("Debug Message");
    Log::config("Config Message");
    Log::info("Info Message");
    Log::warning("Warning Message");
    Log::error("Error Message");

    int lines = numOfLinesInFile("logfile.log");
    printf("lines = %d\n", lines);
    // debug, config, info and warning should not be printed
    ASSERT(lines == 1 + LENGTH_OF_BANNER, "Expected 1+LENGTH_OF_BANNER lines.");
    Log::close();
    unlink("logfile.log");
  }
END_TEST(ERROR_LEVEL)

BEGIN_TEST(NO_LOG)
  {
    Log::init(LogLevel::None, "logfile");

    Log::debug("Debug Message");
    Log::config("Config Message");
    Log::info("Info Message");
    Log::warning("Warning Message");
    Log::error("Error Message");

    int lines = numOfLinesInFile("logfile.log");
    printf("lines = %d\n", lines);
    // debug, config, info and warning and even error should not be printed
    // As the logfile is not there so -1 will be returned.
    ASSERT(lines == -1 || lines == 0, "Expected 0 or -1 lines.");
    Log::close();
    unlink("logfile.log");
  }
END_TEST(NO_LOG)

BEGIN_TEST(LOGFN)
  {
    for (LogLevel level = Error; level <= Debug;
         level = LogLevel(level + 1)) {
      Log::init(level, "logfile");

      testLogFnError();
      testLogFnWarning();
      testLogFnInfo();
      testLogFnConfig();
      testLogFnFine();
      testLogFnFiner();
      testLogFnFinest();
      testLogFnDebug();

      int lines = numOfLinesInFile("logfile.log");

      printf("lines = %d, level = %s\n", lines, Log::levelToChars(level));

      ASSERT(lines == 3 * expected(level) - 2 * LENGTH_OF_BANNER,
             "Wrong number of lines");
      Log::close();

      unlink("logfile.log");
    }
  }
END_TEST(LOGFN)
