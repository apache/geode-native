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

#include <gmock/gmock.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/process/environment.hpp>

#include <gtest/gtest.h>

#include "CacheImpl.hpp"
#include "statistics/HostStatSampler.hpp"
#include "statistics/StatArchiveWriter.hpp"

using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StrEq;

using apache::geode::statistics::HostStatSampler;
using apache::geode::statistics::StatArchiveWriter;

constexpr size_t kibibyte = 1024;
constexpr size_t mebibyte = kibibyte * 1024;
constexpr size_t gibibyte = mebibyte * 1024;

class TestableHostStatSampler : public HostStatSampler {
 public:
  explicit TestableHostStatSampler(std::string filePath,
                                   std::chrono::milliseconds sampleRate,
                                   size_t statFileLimit,
                                   size_t statDiskSpaceLimit)
      : HostStatSampler(filePath, sampleRate, statFileLimit,
                        statDiskSpaceLimit) {}

  boost::filesystem::path chkForGFSExt(boost::filesystem::path filename) const {
    return HostStatSampler::chkForGFSExt(filename);
  }

  boost::filesystem::path initStatFileWithExt() {
    return HostStatSampler::initStatFileWithExt();
  }

  void rollArchive(std::string filename) {
    HostStatSampler::rollArchive(filename);
  }

  void initStatDiskSpaceEnabled() {
    HostStatSampler::initStatDiskSpaceEnabled();
  }

  void initRollIndex() { HostStatSampler::initRollIndex(); }

  int32_t getRollIndex() { return HostStatSampler::rollIndex_; }

  size_t getSpaceUsed() { return HostStatSampler::spaceUsed_; }
};

TEST(HostStatSamplerTest,
     constructEmptyPathZeroSampleRateZeroFileLimitZeroDiskLimit) {
  const TestableHostStatSampler hostStatSampler(
      "", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(hostStatSampler.getArchiveFilename(), Eq(""));
  EXPECT_THAT(hostStatSampler.getSampleRate(),
              Eq(std::chrono::milliseconds::zero()));
  EXPECT_THAT(hostStatSampler.getArchiveFileSizeLimit(), Eq(0));
  EXPECT_THAT(hostStatSampler.getArchiveDiskSpaceLimit(), Eq(0));
}

TEST(HostStatSamplerTest, constructWithFileLimitOverMax) {
  const TestableHostStatSampler hostStatSampler(
      "", std::chrono::milliseconds::zero(), 1025, 0);

  EXPECT_THAT(hostStatSampler.getArchiveFileSizeLimit(), Eq(1 * gibibyte));
}

TEST(HostStatSamplerTest, constructWithDiskLimitLessThanFileLimit) {
  const TestableHostStatSampler hostStatSampler(
      "", std::chrono::milliseconds::zero(), 5, 4);

  EXPECT_THAT(hostStatSampler.getArchiveDiskSpaceLimit(), Eq(4 * mebibyte));
}

TEST(HostStatSamplerTest, chkForGFSExtWithoutDiskLimit) {
  const TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.gfs"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.gfs"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.ext"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.ext"), Eq("/tmp/x.gfs"));
}

TEST(HostStatSamplerTest, chkForGFSExtWithDiskSpaceLimit) {
  const TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 1);

  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.gfs"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.gfs"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.ext"), Eq("x.ext.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.ext"), Eq("/tmp/x.ext.gfs"));
}

TEST(HostStatSamplerTest, createArchiveFilenameWithoutDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(
      hostStatSampler.createArchiveFilename(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest,
     createArchiveFilenameWithAlternativeExtensionWithoutDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.ext", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(
      hostStatSampler.createArchiveFilename(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest,
     createArchiveFilenameWithoutExtensionOrDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(
      hostStatSampler.createArchiveFilename(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest, createArchiveFilenameWithDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 1);

  EXPECT_THAT(hostStatSampler.createArchiveFilename(), Eq("stats.gfs"));
}

TEST(HostStatSamplerTest, initStatFileWithExtWithoutDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(
      hostStatSampler.initStatFileWithExt(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest,
     initStatFileWithExtWithAlternativeExtensionWithoutDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.ext", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(
      hostStatSampler.initStatFileWithExt(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest, initStatFileWithExtWithoutExtensionOrDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats", std::chrono::milliseconds::zero(), 0, 0);

  EXPECT_THAT(
      hostStatSampler.initStatFileWithExt(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest, initStatFileWithExtWithDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 1);

  EXPECT_THAT(hostStatSampler.initStatFileWithExt(), Eq("stats.gfs"));
}

// TODO integration test

TEST(HostStatSamplerTest, DISABLED_changeArchive) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 0);

  //  const auto archiver = hostStatSampler.getArchiver();

  hostStatSampler.changeArchive("changed.gfs");

  // EXPECT_THAT(hostStatSampler.getArchiver(), Eq(archiver));
  EXPECT_THAT(hostStatSampler.isRunning(), IsTrue());
}

TEST(HostStatSamplerTest, rollArchiveFileWithNonexistentFile) {
  boost::filesystem::path file("nonexistent.gfs");
  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 0);

  boost::filesystem::remove(file);
  ASSERT_THAT(boost::filesystem::exists(file), IsFalse());

  hostStatSampler.rollArchive(file.string());

  EXPECT_THAT(boost::filesystem::exists(file), IsFalse());
}

TEST(HostStatSamplerTest, rollArchiveFileWithEmptyFile) {
  boost::filesystem::path file{"empty.gfs"};
  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 0);

  {
    boost::filesystem::remove(file);
    boost::filesystem::ofstream ofs{file};
  }

  ASSERT_THAT(boost::filesystem::exists(file), IsTrue());
  ASSERT_THAT(boost::filesystem::is_empty(file), IsTrue());

  hostStatSampler.rollArchive(file.string());

  EXPECT_THAT(boost::filesystem::exists(file), IsTrue());
  EXPECT_THAT(boost::filesystem::is_empty(file), IsTrue());

  boost::filesystem::remove(file);
}

TEST(HostStatSamplerTest, rollArchiveFile) {
  boost::filesystem::path file{"stats.gfs"};
  boost::filesystem::path file0{"stats-0.gfs"};
  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 0);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::ofstream ofs{file};
    ofs << "original content";
  }

  ASSERT_THAT(boost::filesystem::exists(file), IsTrue());
  ASSERT_THAT(boost::filesystem::is_empty(file), IsFalse());
  ASSERT_THAT(boost::filesystem::exists(file0), IsFalse());

  hostStatSampler.rollArchive(file.string());

  EXPECT_THAT(boost::filesystem::exists(file), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());
  EXPECT_THAT(boost::filesystem::is_empty(file0), IsFalse());

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
}

TEST(HostStatSamplerTest, rollArchiveFileWithDirectory) {
  auto file = boost::filesystem::temp_directory_path() / "stats.gfs";
  auto file0 = boost::filesystem::temp_directory_path() / "stats-0.gfs";
  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 0);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::ofstream ofs{file};
    ofs << "original content";
  }

  ASSERT_THAT(boost::filesystem::exists(file), IsTrue());
  ASSERT_THAT(boost::filesystem::is_empty(file), IsFalse());
  ASSERT_THAT(boost::filesystem::exists(file0), IsFalse());

  hostStatSampler.rollArchive(file.string());

  EXPECT_THAT(boost::filesystem::exists(file), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());
  EXPECT_THAT(boost::filesystem::is_empty(file0), IsFalse());

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
}

TEST(HostStatSamplerTest, rollArchiveFileWithoutExtensionThrows) {
  boost::filesystem::path file{"stats"};
  boost::filesystem::path file0{"stats-0"};
  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 0);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::ofstream ofs{file};
    ofs << "original content";
  }

  ASSERT_THAT(boost::filesystem::exists(file), IsTrue());
  ASSERT_THAT(boost::filesystem::is_empty(file), IsFalse());
  ASSERT_THAT(boost::filesystem::exists(file0), IsFalse());

  EXPECT_THROW(hostStatSampler.rollArchive(file.string()),
               apache::geode::client::IllegalArgumentException);

  EXPECT_THAT(boost::filesystem::exists(file), IsTrue());
  EXPECT_THAT(boost::filesystem::is_empty(file), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file0), IsFalse());

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
}

TEST(HostStatSamplerTest, rollArchiveFileNextFileExists) {
  boost::filesystem::path file{"stats.gfs"};
  boost::filesystem::path file0{"stats-0.gfs"};
  boost::filesystem::path file1{"stats-1.gfs"};
  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 0);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::remove(file1);
    boost::filesystem::ofstream ofs{file};
    ofs << "more content";
    boost::filesystem::ofstream ofs0{file0};
    ofs0 << "original content";
  }

  ASSERT_THAT(boost::filesystem::exists(file), IsTrue());
  ASSERT_THAT(boost::filesystem::is_empty(file), IsFalse());
  ASSERT_THAT(boost::filesystem::exists(file0), IsTrue());
  ASSERT_THAT(boost::filesystem::is_empty(file0), IsFalse());
  ASSERT_THAT(boost::filesystem::exists(file1), IsFalse());

  hostStatSampler.rollArchive(file.string());

  EXPECT_THAT(boost::filesystem::exists(file), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());
  EXPECT_THAT(boost::filesystem::is_empty(file0), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file1), IsTrue());
  EXPECT_THAT(boost::filesystem::is_empty(file1), IsFalse());

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
  boost::filesystem::remove(file1);
}

TEST(HostStatSamplerTest, initStatDiskSpaceEnabledWithZeroDiskLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 0);
  hostStatSampler.initStatDiskSpaceEnabled();

  EXPECT_THAT(hostStatSampler.getArchiveFilename(), Eq("stats.gfs"));
}

TEST(HostStatSamplerTest,
     DISABLED_initStatDiskSpaceEnabledWithNonzeronDiskLimit) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 1);
  hostStatSampler.initStatDiskSpaceEnabled();

  EXPECT_THAT(hostStatSampler.getArchiveFilename(), Eq("stats.gfs"));
}

TEST(
    HostStatSamplerTest,
    DISABLED_initStatDiskSpaceEnabledWithNonzeroFileLimitNonzeronDiskLimitAndExistingFile) {
  boost::filesystem::path file{"stats.gfs"};
  boost::filesystem::remove(file);
  { boost::filesystem::ofstream ofs{file}; }
  boost::filesystem::path file0{"stats-0.gfs"};
  boost::filesystem::remove(file0);

  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 1, 1);

  ASSERT_THAT(boost::filesystem::exists(file), IsTrue());

  hostStatSampler.initStatDiskSpaceEnabled();

  EXPECT_THAT(boost::filesystem::exists(file), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());
}

TEST(HostStatSamplerTest, initRollIndexNoFiles) {
  TestableHostStatSampler hostStatSampler(
      "stats.gfs", std::chrono::milliseconds::zero(), 0, 1);
  hostStatSampler.initRollIndex();

  EXPECT_THAT(hostStatSampler.getRollIndex(), Eq(0));
}

TEST(HostStatSamplerTest, initRollIndex2Files) {
  boost::filesystem::path file{"stats.gfs"};
  boost::filesystem::path file0{"stats-0.gfs"};

  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 1);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::ofstream ofs{file};
    boost::filesystem::ofstream ofs0{file0};
  }

  hostStatSampler.initRollIndex();

  EXPECT_THAT(hostStatSampler.getRollIndex(), Eq(1));

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
}

TEST(HostStatSamplerTest, checkDiskLimitUnderLimit) {
  boost::filesystem::path file{"stats.gfs"};
  boost::filesystem::path file0{"stats-0.gfs"};

  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 1);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::ofstream ofs{file};
    ofs << "more content";
    boost::filesystem::ofstream ofs0{file0};
    ofs0 << "original content";
  }

  EXPECT_THAT(boost::filesystem::exists(file), IsTrue());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());
  EXPECT_THAT(boost::filesystem::file_size(file0), Eq(16));

  hostStatSampler.checkDiskLimit();

  EXPECT_THAT(hostStatSampler.getSpaceUsed(), Eq(16));
  EXPECT_THAT(boost::filesystem::exists(file), IsTrue());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
}

TEST(HostStatSamplerTest, checkDiskLimitOverLimit) {
  boost::filesystem::path file{"stats.gfs"};
  boost::filesystem::path file0{"stats-0.gfs"};
  boost::filesystem::path file1{"stats-1.gfs"};

  TestableHostStatSampler hostStatSampler(
      file.string(), std::chrono::milliseconds::zero(), 0, 1);

  {
    boost::filesystem::remove(file);
    boost::filesystem::remove(file0);
    boost::filesystem::ofstream ofs{file};
    ofs << "more content";
    boost::filesystem::ofstream ofs0{file0};
    ofs0 << std::string(1 * mebibyte, 'a');
    boost::filesystem::ofstream ofs1{file1};
    ofs1 << std::string(1, 'a');
  }

  EXPECT_THAT(boost::filesystem::exists(file), IsTrue());
  EXPECT_THAT(boost::filesystem::exists(file0), IsTrue());
  EXPECT_THAT(boost::filesystem::file_size(file0), Eq(1 * mebibyte));
  EXPECT_THAT(boost::filesystem::exists(file1), IsTrue());
  EXPECT_THAT(boost::filesystem::file_size(file1), Eq(1));

  hostStatSampler.checkDiskLimit();

  EXPECT_THAT(hostStatSampler.getSpaceUsed(), Eq(1));
  EXPECT_THAT(boost::filesystem::exists(file), IsTrue());
  EXPECT_THAT(boost::filesystem::exists(file0), IsFalse());
  EXPECT_THAT(boost::filesystem::exists(file1), IsTrue());
  //  EXPECT_THAT(boost::filesystem::file_size(file1), Eq(1));

  boost::filesystem::remove(file);
  boost::filesystem::remove(file0);
  boost::filesystem::remove(file1);
}
