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

#include <map>
#include <regex>
#include <string>
#include <util/Log.hpp>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include <geode/AuthenticatedView.hpp>
#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using apache::geode::client::CacheClosedException;
using apache::geode::client::CacheFactory;
using apache::geode::client::LogLevel;
using apache::geode::client::RegionShortcut;

namespace {

const auto __1K__ = 1024;
const auto __4K__ = 4 * __1K__;
const auto __1M__ = (__1K__ * __1K__);
const auto __1G__ = (__1K__ * __1K__ * __1K__);

const auto LENGTH_OF_BANNER = 16;

auto testLogFileName = std::string("LoggingTest.log");

const char* __1KStringLiteral =
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

class LoggingTest : public testing::Test {
  void scrubTestLogFiles() {
    auto testFileNames = {testLogFileName.c_str(), "geode-native.log"};

    for (auto name : testFileNames) {
      // Close logger, just in case
      apache::geode::client::Log::close();

      if (boost::filesystem::exists(name)) {
        boost::filesystem::remove(name);
      }

      std::map<int32_t, boost::filesystem::path> rolledFiles;
      LoggingTest::findRolledFiles(boost::filesystem::current_path().string(),
                                   name, rolledFiles);
      for (auto& item : rolledFiles) {
        boost::filesystem::remove(item.second);
      }
    }
  }

  virtual void SetUp() {
    // scrubTestLogFiles();
  }

  virtual void TearDown() { scrubTestLogFiles(); }

 public:
  static void writeRolledLogFile(const boost::filesystem::path& logdir,
                                 int32_t rollIndex) {
    auto rolledPath =
        logdir / boost::filesystem::path("LoggingTest-" +
                                         std::to_string(rollIndex) + ".log");
    auto rolledFile = fopen(rolledPath.string().c_str(), "w");
    fwrite("Test", 1, 4, rolledFile);
    fclose(rolledFile);
  }

  static int numOfLinesInFile(const char* fname) {
    char line[2048];
    char* read;
    int ln_cnt = 0;
    FILE* fp = fopen(fname, "r");
    if (fp == nullptr) {
      return 0;
    }
    while (!!(read = fgets(line, sizeof line, fp))) {
      ++ln_cnt;
    }

    if (!feof(fp)) {
      fclose(fp);
      return -2;
    }
    fclose(fp);
    return ln_cnt;
  }

  static int expected(LogLevel level) {
    int expected = static_cast<int>(level);
    if (level >= LogLevel::Default) {
      expected--;
    }
    return expected;
  }

  static int expectedWithBanner(LogLevel level) {
    int expected = LoggingTest::expected(level);
    if (level != LogLevel::None) {
      expected += LENGTH_OF_BANNER;
    }
    return expected;
  }

  static void verifyLineCountAtLevel(LogLevel level) {
    apache::geode::client::Log::init(level, testLogFileName);

    LOGERROR("Error Message");
    LOGWARN("Warning Message");
    LOGINFO("Info Message");
    LOGCONFIG("Config Message");
    LOGFINE("Fine Message");
    LOGFINER("Finer Message");
    LOGFINEST("Finest Message");
    LOGDEBUG("Debug Message");

    int lines = LoggingTest::numOfLinesInFile(testLogFileName.c_str());

    ASSERT_TRUE(lines == LoggingTest::expectedWithBanner(level));

    apache::geode::client::Log::close();
    boost::filesystem::remove(testLogFileName.c_str());
  }

  static void findRolledFiles(
      const std::string& logFilePath, const boost::filesystem::path& filename,
      std::map<int32_t, boost::filesystem::path>& rolledFiles) {
    const auto basePath =
        boost::filesystem::absolute(boost::filesystem::path(logFilePath)) /
        filename;
    const auto filterstring = basePath.stem().string() + "-(\\d+)\\.log$";
    const std::regex my_filter(filterstring);

    rolledFiles.clear();

    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator i(
             basePath.parent_path().string());
         i != end_itr; ++i) {
      if (boost::filesystem::is_regular_file(i->status())) {
        std::string filename = i->path().filename().string();
        std::regex testPattern(filterstring);
        std::match_results<std::string::const_iterator> testMatches;
        if (std::regex_search(std::string::const_iterator(filename.begin()),
                              filename.cend(), testMatches, testPattern)) {
          auto index = std::atoi(
              std::string(testMatches[1].first, testMatches[1].second).c_str());
          rolledFiles[index] = i->path();
        }
      }
    }
  }

  static size_t calculateUsedDiskSpace(const std::string& logFilePath) {
    std::map<int32_t, boost::filesystem::path> rolledLogFiles{};
    findRolledFiles(boost::filesystem::current_path().string(), testLogFileName,
                    rolledLogFiles);

    auto usedSpace = boost::filesystem::file_size(logFilePath);
    for (auto const& item : rolledLogFiles) {
      usedSpace += boost::filesystem::file_size(item.second);
    }

    return usedSpace;
  }
};

/**
 * Verify we can initialize the logger with any combination of level,
 * filename, file size limit, and disk space limit
 */
TEST_F(LoggingTest, logInit) {
  // Check all valid levels
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::None, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Error, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Warning, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Info, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Default, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Fine, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Finer, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Finest, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::All, testLogFileName.c_str(), 1, 4));
  apache::geode::client::Log::close();

  // Init with valid filename
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, "LoggingTest.log"));
  apache::geode::client::Log::close();

  // Init with legal filename with (), #, and space
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, "LoggingTest (#).log"));
  apache::geode::client::Log::close();
  boost::filesystem::remove("LoggingTest (#).log");

  // Init with invalid filename
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "#?$?%.log"),
               apache::geode::client::IllegalArgumentException);

  // Specify disk or file limit without a filename
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, nullptr, 4));
  apache::geode::client::Log::close();
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, nullptr, 0, 4));
  apache::geode::client::Log::close();

  // Specify a disk space limit smaller than the file size limit
  ASSERT_THROW(
      apache::geode::client::Log::init(apache::geode::client::LogLevel::Config,
                                       testLogFileName, __1K__, 4),
      apache::geode::client::IllegalArgumentException);

  // Specify a file size limit above max allowed
  ASSERT_THROW(
      apache::geode::client::Log::init(apache::geode::client::LogLevel::Config,
                                       testLogFileName, __1G__),
      apache::geode::client::IllegalArgumentException);

  // Specify a disk space limit above max allowed
  ASSERT_THROW(
      apache::geode::client::Log::init(apache::geode::client::LogLevel::Config,
                                       testLogFileName, 1, __1G__),
      apache::geode::client::IllegalArgumentException);

  // Init twice without closing
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::All, testLogFileName.c_str(), 1, 4));
  ASSERT_THROW(
      apache::geode::client::Log::init(apache::geode::client::LogLevel::All,
                                       testLogFileName.c_str(), 1, 4),
      apache::geode::client::IllegalStateException);
  apache::geode::client::Log::close();
}

TEST_F(LoggingTest, logToFileAtEachLevel) {
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName));
  LOGDEBUG("This is a debug string");
  LOGDEBUG("This is a formatted debug string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);
  ASSERT_FALSE(boost::filesystem::exists(testLogFileName));

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Finest, testLogFileName));
  LOGFINEST("This is a 'finest' string");
  LOGFINEST("This is a formatted 'finest' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Finer, testLogFileName));
  LOGFINER("This is a 'finer' string");
  LOGFINER("This is a formatted 'finer' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Fine, testLogFileName));
  LOGFINE("This is a 'fine' string");
  LOGFINE("This is a formatted 'fine' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, testLogFileName));
  LOGCONFIG("This is a 'config' string");
  LOGCONFIG("This is a formatted 'config' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Info, testLogFileName));
  LOGINFO("This is a 'finer' string");
  LOGINFO("This is a formatted 'finer' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Warning, testLogFileName));
  LOGWARN("This is a 'warning' string");
  LOGWARN("This is a formatted 'warning' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Error, testLogFileName));
  LOGERROR("This is a 'error' string");
  LOGERROR("This is a formatted 'error' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);
}

TEST_F(LoggingTest, verifyFileSizeLimit) {
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName, 1, 5));
  for (auto i = 0; i < 4 * __1K__; i++) {
    LOGDEBUG(__1KStringLiteral);
  }
  apache::geode::client::Log::close();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  // Check for 'rolled' log files.  With a 1MB file size limit and each logged
  // string having a length of 1K chars, we should have at least one less
  // rolled log file than the number of strings logged, i.e. 3 rolled files
  // for 4K strings in this case.  spdlog rolled files look like
  // <<basename>>.<<#>>.<<extension>>, so for LoggingTest.log we should find
  // LoggingTest.1.log, LoggingTest.2.log, etc.
  auto base = boost::filesystem::path(testLogFileName).stem();
  auto ext = boost::filesystem::path(testLogFileName).extension();

  // File size limit is treated as a "soft" limit.  If the last message in the
  // log puts the file size over the limit, the file is rolled and the message
  // is preserved intact, rather than truncated or split across files.  We'll
  // assume the file size never exceeds 110% of the specified limit.
  auto adjustedFileSizeLimit =
      static_cast<uint32_t>(static_cast<uint64_t>(__1M__) * 11 / 10);

  for (auto i = 0; i < 4; i++) {
    auto rolledLogFileName =
        base.string() + "-" + std::to_string(i) + ext.string();

    ASSERT_TRUE(boost::filesystem::exists(rolledLogFileName));
    ASSERT_TRUE(adjustedFileSizeLimit >
                boost::filesystem::file_size(rolledLogFileName));
  }
}

TEST_F(LoggingTest, verifyDiskSpaceLimit) {
  const int NUMBER_OF_ITERATIONS = 4 * __1K__;
  const int DISK_SPACE_LIMIT = 2 * __1M__;

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName, 1, 2));
  for (auto i = 0; i < NUMBER_OF_ITERATIONS; i++) {
    LOGDEBUG(__1KStringLiteral);
  }
  apache::geode::client::Log::close();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  auto size = boost::filesystem::file_size(testLogFileName);
  auto numRolledFilesFound = 0;
  auto base = boost::filesystem::path(testLogFileName).stem();
  auto ext = boost::filesystem::path(testLogFileName).extension();

  // We wrote 4x the log file limit, and 2x the disk space limit, so
  // there should be one 'rolled' file.  Its name should be of the form
  // <base>-n.log, where n is some reasonable number.
  std::map<int32_t, boost::filesystem::path> rolledFiles;
  LoggingTest::findRolledFiles(boost::filesystem::current_path().string(),
                               testLogFileName, rolledFiles);
  ASSERT_TRUE(rolledFiles.size() == 1);

  auto rolledFile = rolledFiles.begin()->second;
  size += boost::filesystem::file_size(rolledFile);

  ASSERT_TRUE(size <= DISK_SPACE_LIMIT);
}

TEST_F(LoggingTest, verifyWithExistingRolledFile) {
  LoggingTest::writeRolledLogFile(boost::filesystem::current_path(), 11);
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName, 1, 5));
  for (auto i = 0; i < 2 * __1K__; i++) {
    LOGDEBUG(__1KStringLiteral);
  }
  apache::geode::client::Log::close();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  // Check for 'rolled' log files.  With a 1MB file size limit and each logged
  // string having a length of 1K chars, we should have at least one less
  // rolled log file than the number of strings logged, i.e. 3 rolled files
  // for 4K strings in this case.  spdlog rolled files look like
  // <<basename>>.<<#>>.<<extension>>, so for LoggingTest.log we should find
  // LoggingTest.1.log, LoggingTest.2.log, etc.
  auto base = boost::filesystem::path(testLogFileName).stem();
  auto ext = boost::filesystem::path(testLogFileName).extension();

  // File size limit is treated as a "soft" limit.  If the last message in the
  // log puts the file size over the limit, the file is rolled and the message
  // is preserved intact, rather than truncated or split across files.  We'll
  // assume the file size never exceeds 110% of the specified limit.
  auto adjustedFileSizeLimit =
      static_cast<uint32_t>(static_cast<uint64_t>(__1M__) * 11 / 10);

  auto rolledLogFileName =
      base.string() + "-" + std::to_string(12) + ext.string();

  ASSERT_TRUE(boost::filesystem::exists(rolledLogFileName));
  ASSERT_TRUE(adjustedFileSizeLimit >
              boost::filesystem::file_size(rolledLogFileName));
}

void verifyWithPath(const boost::filesystem::path& path, int32_t fileSizeLimit,
                    int64_t diskSpaceLimit) {
  auto relativePath = path / boost::filesystem::path(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, relativePath.string(),
      fileSizeLimit, diskSpaceLimit));
  for (auto i = 0; i < ((3 * fileSizeLimit) / 2) * __1K__; i++) {
    LOGDEBUG(__1KStringLiteral);
  }
  apache::geode::client::Log::close();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(relativePath));

  // Check for 'rolled' log files.  With a 1MB file size limit and each logged
  // string having a length of 1K chars, we should have at least one less
  // rolled log file than the number of strings logged, i.e. 3 rolled files
  // for 4K strings in this case.  spdlog rolled files look like
  // <<basename>>.<<#>>.<<extension>>, so for LoggingTest.log we should find
  // LoggingTest.1.log, LoggingTest.2.log, etc.
  auto base = boost::filesystem::path(relativePath).stem();
  auto ext = boost::filesystem::path(relativePath).extension();

  // File size limit is treated as a "soft" limit.  If the last message in the
  // log puts the file size over the limit, the file is rolled and the message
  // is preserved intact, rather than truncated or split across files.  We'll
  // assume the file size never exceeds 110% of the specified limit.
  auto adjustedFileSizeLimit = static_cast<uint32_t>(
      static_cast<uint64_t>(__1M__ * fileSizeLimit) * 11 / 10);

  auto rolledLogFileName =
      relativePath.parent_path() /
      boost::filesystem::path(base.string() + "-" + std::to_string(0) +
                              ext.string());

  if (fileSizeLimit == diskSpaceLimit) {
    // If the limits are equal, we should *never* roll logs, just delete the
    // current file and start over
    ASSERT_FALSE(boost::filesystem::exists(rolledLogFileName));
  } else {
    ASSERT_TRUE(boost::filesystem::exists(rolledLogFileName));
    ASSERT_TRUE(adjustedFileSizeLimit >
                boost::filesystem::file_size(rolledLogFileName));
  }
  ASSERT_TRUE(adjustedFileSizeLimit >
              boost::filesystem::file_size(relativePath));
}

TEST_F(LoggingTest, verifyWithRelativePathFromCWD) {
  auto relativePath = boost::filesystem::path("foo/bar");

  verifyWithPath(relativePath, 1, 5);

  boost::filesystem::remove_all(boost::filesystem::path("foo"));
}

TEST_F(LoggingTest, verifyWithAbsolutePath) {
  auto absolutePath =
      boost::filesystem::absolute(boost::filesystem::path("foo/bar"));

  verifyWithPath(absolutePath, 1, 5);

  boost::filesystem::remove_all(boost::filesystem::path("foo"));
}

TEST_F(LoggingTest, setLimitsEqualAndRoll) {
  verifyWithPath(boost::filesystem::path(), 1, 1);
}

// Logger is supposed to tack the '.log' extension on any file that doesn't
// already have it.
TEST_F(LoggingTest, verifyExtension) {
  apache::geode::client::Log::init(LogLevel::All, "foo");
  LOGINFO("...");
  apache::geode::client::Log::close();
  ASSERT_TRUE(LoggingTest::numOfLinesInFile("foo.log") > 0);
  boost::filesystem::remove("foo.log");

  apache::geode::client::Log::init(LogLevel::All, "foo.txt");
  LOGINFO("...");
  apache::geode::client::Log::close();
  ASSERT_TRUE(LoggingTest::numOfLinesInFile("foo.txt.log") > 0);
  boost::filesystem::remove("foo.txt.log");
}

// Old version of logger didn't distinguish between rolled log file and
// filename containing '-', so would crash in an atoi() call if you used
// '-' in your log file name.
TEST_F(LoggingTest, verifyFilenameWithDash) {
  apache::geode::client::Log::init(LogLevel::All, "foo-bar.log");
  LOGINFO("...");
  apache::geode::client::Log::close();
  ASSERT_TRUE(LoggingTest::numOfLinesInFile("foo-bar.log") > 0);
  boost::filesystem::remove("foo-bar.log");
}

TEST_F(LoggingTest, countLinesAllLevels) {
  for (LogLevel level : {
           LogLevel::Error,
           LogLevel::Warning,
           LogLevel::Info,
           LogLevel::Default,
           LogLevel::Config,
           LogLevel::Fine,
           LogLevel::Finer,
           LogLevel::Finest,
           LogLevel::Debug,
       }) {
    apache::geode::client::Log::init(level, testLogFileName);

    LOGERROR("Error Message");
    LOGWARN("Warning Message");
    LOGINFO("Info Message");
    LOGCONFIG("Config Message");
    LOGFINE("Fine Message");
    LOGFINER("Finer Message");
    LOGFINEST("Finest Message");
    LOGDEBUG("Debug Message");

    int lines = LoggingTest::numOfLinesInFile(testLogFileName.c_str());

    ASSERT_TRUE(lines == LoggingTest::expectedWithBanner(level));

    apache::geode::client::Log::close();
    boost::filesystem::remove(testLogFileName);
  }
}

TEST_F(LoggingTest, countLinesConfigOnwards) {
  verifyLineCountAtLevel(LogLevel::Config);
}

TEST_F(LoggingTest, countLinesInfoOnwards) {
  verifyLineCountAtLevel(LogLevel::Info);
}

TEST_F(LoggingTest, countLinesWarningOnwards) {
  verifyLineCountAtLevel(LogLevel::Warning);
}

TEST_F(LoggingTest, countLinesErrorOnly) {
  verifyLineCountAtLevel(LogLevel::Error);
}

TEST_F(LoggingTest, countLinesNone) { verifyLineCountAtLevel(LogLevel::None); }

TEST_F(LoggingTest, verifyDiskSpaceNotLeaked) {
  const int NUMBER_OF_ITERATIONS = 4 * __1K__;
  const int DISK_SPACE_LIMIT = 2 * __1M__;

  // Start/stop logger several times, make sure it's picking up any/all
  // existing logs in its disk space calculations.
  for (auto j = 0; j < 5; j++) {
    ASSERT_NO_THROW(apache::geode::client::Log::init(
        apache::geode::client::LogLevel::Debug, testLogFileName, 1, 2));
    for (auto i = 0; i < NUMBER_OF_ITERATIONS; i++) {
      LOGDEBUG(__1KStringLiteral);
    }
    apache::geode::client::Log::close();

    // Original file should still be around
    ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

    // We wrote 4x the log file limit, and 2x the disk space limit, so
    // there should be one 'rolled' file.  Its name should be of the form
    // <base>-n.log, where n is some reasonable number.
    auto usedSpace = calculateUsedDiskSpace(testLogFileName);
    ASSERT_TRUE(usedSpace < DISK_SPACE_LIMIT);
  }
}
}  // namespace
