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

#include <boost/process/environment.hpp>

#include <gtest/gtest.h>

#include "statistics/HostStatSampler.hpp"

using ::testing::Eq;
using ::testing::StrEq;

using apache::geode::statistics::HostStatSampler;

class TestableHostStatSampler : public HostStatSampler {
 public:
  explicit TestableHostStatSampler(std::string filePath, size_t statFileLimit,
                                   size_t statDiskSpaceLimit)
      : HostStatSampler(filePath, statFileLimit, statDiskSpaceLimit) {}
  boost::filesystem::path chkForGFSExt(boost::filesystem::path filename) const {
    return HostStatSampler::chkForGFSExt(filename);
  }
};

TEST(HostStatSamplerTest, chkForGFSExtWithoutDiskSpaceLimit) {
  const TestableHostStatSampler hostStatSampler("stats.gfs", 0, 0);

  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.gfs"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.gfs"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.ext"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.ext"), Eq("/tmp/x.gfs"));
}

TEST(HostStatSamplerTest, chkForGFSExtWithDiskSpaceLimit) {
  const TestableHostStatSampler hostStatSampler("stats.gfs", 0, 1);

  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.gfs"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.gfs"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x"), Eq("x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x"), Eq("/tmp/x.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("x.ext"), Eq("x.ext.gfs"));
  EXPECT_THAT(hostStatSampler.chkForGFSExt("/tmp/x.ext"), Eq("/tmp/x.ext.gfs"));
}

TEST(HostStatSamplerTest, createArchiveFilenameWithoutDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler("stats.gfs", 0, 0);

  EXPECT_THAT(
      hostStatSampler.createArchiveFilename(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest,
     createArchiveFilenameWithAlternativeExtensionWithoutDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler("stats.ext", 0, 0);

  EXPECT_THAT(
      hostStatSampler.createArchiveFilename(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest,
     createArchiveFilenameWithoutExtensionOrDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler("stats", 0, 0);

  EXPECT_THAT(
      hostStatSampler.createArchiveFilename(),
      Eq("stats-" + std::to_string(boost::this_process::get_id()) + ".gfs"));
}

TEST(HostStatSamplerTest, createArchiveFilenameWithDiskSpaceLimit) {
  TestableHostStatSampler hostStatSampler("stats.gfs", 0, 1);

  EXPECT_THAT(hostStatSampler.createArchiveFilename(), Eq("stats.gfs"));
}
