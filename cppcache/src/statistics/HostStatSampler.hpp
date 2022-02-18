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

#ifndef GEODE_STATISTICS_HOSTSTATSAMPLER_H_
#define GEODE_STATISTICS_HOSTSTATSAMPLER_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/filesystem/path.hpp>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "StatArchiveWriter.hpp"
#include "StatSamplerStats.hpp"
#include "StatisticDescriptor.hpp"
#include "Statistics.hpp"
#include "StatisticsManager.hpp"
#include "StatisticsType.hpp"

class TestableHostStatSampler;

namespace apache {
namespace geode {
namespace statistics {

using apache::geode::client::CacheImpl;
using std::chrono::system_clock;

class StatArchiveWriter;
class StatisticsManager;

/**
 * HostStatSampler implements a thread which will monitor, sample and archive
 * statistics. It only has the common functionalities which any sampler needs.
 */
class HostStatSampler {
 public:
  HostStatSampler(boost::filesystem::path filePath,
                  std::chrono::milliseconds sampleRate,
                  StatisticsManager* statMngr, CacheImpl* cache,
                  size_t statFileLimit = 0, size_t statDiskSpaceLimit = 0);

  ~HostStatSampler() noexcept;

  HostStatSampler(const HostStatSampler&) = delete;

  HostStatSampler& operator=(const HostStatSampler&) = delete;

  /**
   * Adds the pid to the archive file passed to it.
   */
  const boost::filesystem::path& createArchiveFilename();
  /**
   * Returns the archiveFileName
   */
  const boost::filesystem::path& getArchiveFilename() const;
  /**
   * Gets the archive size limit in bytes.
   */
  size_t getArchiveFileSizeLimit() const;
  /**
   * Gets the archive disk space limit in bytes.
   */
  size_t getArchiveDiskSpaceLimit() const;
  /**
   * Gets the sample rate in milliseconds
   */
  std::chrono::milliseconds getSampleRate() const;
  /**
   * Called when this sampler has spent some time working and wants
   * it to be accounted for.
   */
  void accountForTimeSpentWorking(int64_t nanosSpentWorking);
  /**
   * Gets list mutex for synchronization
   */
  std::recursive_mutex& getStatListMutex();
  /**
   * Returns a vector of ptrs to all the current statistic resource instances.
   */
  std::vector<Statistics*>& getStatistics();
  /**
   * Returns a vector of ptrs to all the newly added statistics resource
   * instances.
   */
  std::vector<Statistics*>& getNewStatistics();
  /**
   * Returns a unique id for the sampler's system.
   */
  int64_t getSystemId();
  /**
   * Returns the time this sampler's system was started.
   */
  system_clock::time_point getSystemStartTime();
  /**
   * Returns the path to this sampler's system directory; if it has one.
   */
  const std::string& getSystemDirectoryPath();
  /**
   * Returns a description of the product that the stats are on
   */
  const std::string& getProductDescription() const;
  /**
   * If the size of the archive file exceeds the size limit then the sampler
   * starts writing in a new file. The path of the new file need to be
   * obtained from the manager.
   */
  void changeArchive(boost::filesystem::path);

  void writeGfs();

  void forceSample();

  void doSample(const boost::filesystem::path& archiveFilename);

  /**
   * If the total size of all the archive files exceeds the archive disk space
   * limit then the older files are deleted.
   */
  void checkDiskLimit();

  /**
   * Starts the main thread for this service.
   */
  void start();
  /**
   * Tell this service's main thread to terminate.
   */
  void stop();
  /**
   * The function executed by the thread
   */
  void svc(void);

  /**
   * Method to know whether the sampling thread is running or not.
   */
  bool isRunning() const;

 private:
  /**
   * For testing only.
   */
  explicit HostStatSampler(boost::filesystem::path filePath,
                           std::chrono::milliseconds sampleRate,
                           size_t statFileLimit, size_t statDiskSpaceLimit);

  boost::filesystem::path initStatFileWithExt();



  /**
   * This function rolls the existing archive file.
   * Create new file only if current file has some data, otherwise reuse it.
   */
  void rollArchive(const boost::filesystem::path& filename);

  /**
   * This function check whether the filename has gfs ext or not
   * If it is not there it adds and then returns the new filename.
   */
  boost::filesystem::path chkForGFSExt(
      const boost::filesystem::path& filename) const;

  /**
   * Update New Stats in Admin Region.
   */
  void putStatsInAdminRegion();

  void initStatDiskSpaceEnabled();


  friend TestableHostStatSampler;
  void initRollIndex();

  template <typename _Function>
  void forEachIndexStatFile(_Function function) const;

 private:
  std::recursive_mutex samplingMutex_;
  bool adminError_;
  std::thread thread_;
  std::atomic<bool> running_;
  std::atomic<bool> stopRequested_;
  std::atomic<bool> isStatDiskSpaceEnabled_;
  std::unique_ptr<StatArchiveWriter> archiver_;
  std::unique_ptr<StatSamplerStats> samplerStats_;
  const char* durableClientId_;
  std::chrono::seconds durableTimeout_;

  boost::filesystem::path archiveFileName_;
  size_t archiveFileSizeLimit_;
  size_t archiveDiskSpaceLimit_;
  size_t spaceUsed_ = 0;
  std::chrono::milliseconds sampleRate_;
  StatisticsManager* statsMgr_;
  CacheImpl* cache_;

  int64_t pid_;
  system_clock::time_point startTime_;

  /**
   * The archiveFile, after it exceeds archiveFileSizeLimit should be rolled
   * to a new file name. This integer rollIndex will be used to format the
   * file name into which the current archiveFile will be renamed.
   */
  int32_t rollIndex_;
};

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_HOSTSTATSAMPLER_H_
