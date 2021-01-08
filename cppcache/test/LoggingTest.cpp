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

#include <util/Log.hpp>

#include <gtest/gtest.h>

#include <geode/AuthenticatedView.hpp>
#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "boost/filesystem.hpp"

using apache::geode::client::CacheClosedException;
using apache::geode::client::CacheFactory;
using apache::geode::client::LogLevel;
using apache::geode::client::RegionShortcut;

namespace {

const int __1K__ = 1024;
const int __4K__ = 4 * __1K__;
const int __1M__ = (__1K__ * __1K__);
const int __1G__ = (__1K__ * __1K__ * __1K__);

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
    if (boost::filesystem::exists(testLogFileName)) {
      boost::filesystem::remove(testLogFileName);
    }

    auto base = boost::filesystem::path(testLogFileName).stem();
    auto ext = boost::filesystem::path(testLogFileName).extension();
    for (auto i = 0;; i++) {
      auto rolledLogFileName =
          base.string() + "-" + std::to_string(i) + ext.string();
      if (boost::filesystem::exists(rolledLogFileName)) {
        boost::filesystem::remove(rolledLogFileName);
      } else {
        break;
      }
    }
  }

  virtual void SetUp() { scrubTestLogFiles(); }

  virtual void TearDown() { scrubTestLogFiles(); }
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

  // Init with invalid filename
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "#?$?%.log"),
               apache::geode::client::IllegalArgumentException);

  // Specify disk or file limit without a filename
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "", 4),
               apache::geode::client::IllegalArgumentException);
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "", 0, 4),
               apache::geode::client::IllegalArgumentException);

  // Specify a disk space limit smaller than the file size limit
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "", __1K__, 4),
               apache::geode::client::IllegalArgumentException);

  // Specify a file size limit above max allowed
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "", __1G__),
               apache::geode::client::IllegalArgumentException);

  // Specify a disk space limit above max allowed
  ASSERT_THROW(apache::geode::client::Log::init(
                   apache::geode::client::LogLevel::Config, "", 1, __1G__),
               apache::geode::client::IllegalArgumentException);
}

TEST_F(LoggingTest, logToFileAtEachLevel) {
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName));
  apache::geode::client::Log::debug("This is a debug string");
  LOGDEBUG("This is a formatted debug string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);
  ASSERT_FALSE(boost::filesystem::exists(testLogFileName));

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Finest, testLogFileName));
  apache::geode::client::Log::finest("This is a 'finest' string");
  LOGFINEST("This is a formatted 'finest' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Finer, testLogFileName));
  apache::geode::client::Log::finer("This is a 'finer' string");
  LOGFINER("This is a formatted 'finer' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Fine, testLogFileName));
  apache::geode::client::Log::fine("This is a 'fine' string");
  LOGFINE("This is a formatted 'fine' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Config, testLogFileName));
  apache::geode::client::Log::config("This is a 'config' string");
  LOGCONFIG("This is a formatted 'config' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Info, testLogFileName));
  apache::geode::client::Log::info("This is a 'finer' string");
  LOGINFO("This is a formatted 'finer' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Warning, testLogFileName));
  apache::geode::client::Log::warning("This is a 'warning' string");
  LOGWARN("This is a formatted 'warning' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);

  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Error, testLogFileName));
  apache::geode::client::Log::error("This is a 'error' string");
  LOGERROR("This is a formatted 'error' string (%d)", __1K__);
  apache::geode::client::Log::close();
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));
  ASSERT_TRUE(boost::filesystem::file_size(testLogFileName) > 0);
  boost::filesystem::remove(testLogFileName);
}

TEST_F(LoggingTest, verifyFileSizeLimit) {
  ASSERT_NO_THROW(apache::geode::client::Log::init(
      apache::geode::client::LogLevel::Debug, testLogFileName, 1));
  for (auto i = 0; i < 4 * __1K__; i++) {
    apache::geode::client::Log::debug(__1KStringLiteral);
  }
  apache::geode::client::Log::close();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  // Check for 'rolled' log files.  With a 1MB file size limit and each logged
  // string having a length of 1K chars, we should have at least one less
  // rolled log file than the number of strings logged, i.e. 3 rolled files
  // for 4 strings in this case.  spdlog rolled files look like
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
    apache::geode::client::Log::debug(__1KStringLiteral);
  }
  apache::geode::client::Log::close();

  // Original file should still be around
  ASSERT_TRUE(boost::filesystem::exists(testLogFileName));

  auto size = boost::filesystem::file_size(testLogFileName);
  auto base = boost::filesystem::path(testLogFileName).stem();
  auto ext = boost::filesystem::path(testLogFileName).extension();

  // We wrote 4x the log file limit, and 2x the disk space limit, so
  // there should be one 'rolled' file, and it should be named <test log
  // basename>-0.log.
  auto rolledLogFileName =
      base.string() + "-" + std::to_string(0) + ext.string();
  ASSERT_TRUE(boost::filesystem::exists(rolledLogFileName));

  // Scan through a ton of file names to make sure we catch any strays
  // (logger has had bugs with filename weirdness here, like deleting
  // whatever-0.log and writing to whatever-5.log).
  for (auto i = 0; i < NUMBER_OF_ITERATIONS; i++) {
    auto rolledLogFileName =
        base.string() + "-" + std::to_string(i) + ext.string();
    if (boost::filesystem::exists(rolledLogFileName)) {
      size += boost::filesystem::file_size(rolledLogFileName);
    } else {
      break;
    }
  }

  ASSERT_TRUE(size <= DISK_SPACE_LIMIT);
}
}  // namespace
