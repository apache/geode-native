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
  HostStatSampler(std::string filePath,
                  std::chrono::milliseconds sampleIntervalMs,
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
  boost::filesystem::path getArchiveFilename();
  /**
   * Gets the archive size limit in bytes.
   */
  size_t getArchiveFileSizeLimit();
  /**
   * Gets the archive disk space limit in bytes.
   */
  size_t getArchiveDiskSpaceLimit();
  /**
   * Gets the sample rate in milliseconds
   */
  std::chrono::milliseconds getSampleRate();
  /**
   * Returns true if sampling is enabled.
   */
  bool isSamplingEnabled();
  /**
   * Called when this sampler has spent some time working and wants
   * it to be accounted for.
   */
  void accountForTimeSpentWorking(int64_t nanosSpentWorking);

  /**
   * Returns true if the specified statistic resource still exists.
   */
  bool statisticsExists(int64_t id);
  /**
   * Returns the statistics resource instance given its id.
   */
  Statistics* findStatistics(int64_t id);

  /**
   * Returns the number of statistics object the manager has.
   */
  int32_t getStatisticsModCount();
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
  std::string getSystemDirectoryPath();
  /**
   * Returns a description of the product that the stats are on
   */
  std::string getProductDescription();
  /**
   * If the size of the archive file exceeds the size limit then the sampler
   * starts writing in a new file. The path of the new file need to be
   * obtained from the manager.
   */
  void changeArchive(std::string);

  void checkListeners();

  void writeGfs();

  void forceSample();

  void doSample(std::string& archivefilename);
  /**
   * If the total size of all the archive files exceeds the archive disk space
   * limit then the older
   * files are deleted.
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
  bool isRunning();

 private:
  std::recursive_mutex m_samplingLock;
  bool m_adminError;
  std::thread m_thread;
  std::atomic<bool> m_running;
  std::atomic<bool> m_stopRequested;
  std::atomic<bool> m_isStatDiskSpaceEnabled;
  std::unique_ptr<StatArchiveWriter> m_archiver;
  std::unique_ptr<StatSamplerStats> m_samplerStats;
  const char* m_durableClientId;
  std::chrono::seconds m_durableTimeout;

  boost::filesystem::path m_archiveFileName;
  size_t m_archiveFileSizeLimit;
  size_t m_archiveDiskSpaceLimit;
  std::chrono::milliseconds m_sampleRate;
  StatisticsManager* m_statMngr;
  CacheImpl* m_cache;

  int64_t m_pid;
  system_clock::time_point m_startTime;

  /**
   * For testing only.
   */
  explicit HostStatSampler(std::string filePath, size_t statFileLimit,
                           size_t statDiskSpaceLimit);

  boost::filesystem::path initStatFileWithExt();
  /**
   * The archiveFile, after it exceeds archiveFileSizeLimit should be rolled
   * to a new file name. This integer rollIndex will be used to format the
   * file name into which the current archiveFile will be renamed.
   */
  int32_t m_rollIndex;
  /**
   * This function rolls the existing archive file
   */
  int32_t rollArchive(std::string filename);
  /**
   * This function check whether the filename has gfs ext or not
   * If it is not there it adds and then returns the new filename.
   */
  boost::filesystem::path chkForGFSExt(
      const boost::filesystem::path& filename) const;

  /**
   * Initialize any special sampler stats. Like ProcessStats, HostStats
   */
  void initSpecialStats();
  /**
   * Collect samples of the special tests.
   */
  void sampleSpecialStats();
  /**
   * Closes down anything initialied by initSpecialStats.
   */
  void closeSpecialStats();
  /**
   * Update New Stats in Admin Region.
   */
  void putStatsInAdminRegion();

  static const char* NC_HSS_Thread;

  friend TestableHostStatSampler;
};

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_HOSTSTATSAMPLER_H_
