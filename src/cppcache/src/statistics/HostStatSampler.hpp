#pragma once

#ifndef GEODE_STATISTICS_HOSTSTATSAMPLER_H_
#define GEODE_STATISTICS_HOSTSTATSAMPLER_H_

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

#include <string>
#include <vector>
#include <chrono>
#include <ace/Task.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <geode/geode_globals.hpp>
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticDescriptor.hpp>
#include "StatisticsManager.hpp"
#include <geode/statistics/StatisticsType.hpp>
#include "StatSamplerStats.hpp"
#include "StatArchiveWriter.hpp"
#include <geode/ExceptionTypes.hpp>

#include <NonCopyable.hpp>

using namespace apache::geode::client;

/** @file
 */
#ifndef GEMFIRE_MAX_STATS_FILE_LIMIT
#define GEMFIRE_MAX_STATS_FILE_LIMIT (1024 * 1024 * 1024)
#endif

#ifndef GEMFIRE_MAX_STAT_DISK_LIMIT
#define GEMFIRE_MAX_STAT_DISK_LIMIT (1024LL * 1024LL * 1024LL * 1024LL)
#endif
namespace apache {
namespace geode {
namespace statistics {

using std::chrono::system_clock;

class StatArchiveWriter;
class StatisticsManager;
/**
 * HostStatSampler implements a thread which will monitor, sample and archive
 * statistics. It only has the common functionalities which any sampler needs.
 */

/* adongre
 * CID 28733: Other violation (MISSING_COPY)
 * Class "apache::geode::statistics::OsStatisticsImpl" owns resources that are
 * managed in its constructor and destructor but has no user-written copy
 * constructor.
 *
 * CID 28719: Other violation (MISSING_ASSIGN) Class
 * "apache::geode::statistics::HostStatSampler"
 * owns resources that are managed in its constructor and destructor but has no
 * user-written assignment operator.
 *
 * FIX : Make the class NonCopyable
 */

class CPPCACHE_EXPORT HostStatSampler : public ACE_Task_Base,
                                        private NonCopyable,
                                        private NonAssignable {
 public:
  /*
   * Constructor:
   */
  HostStatSampler(const char* filePath, int64_t sampleIntervalMs,
                  StatisticsManager* statMngr, Cache* cache,
                  const char* durableClientId, const uint32_t durableTimeout,
                  int64_t statFileLimit = 0, int64_t statDiskSpaceLimit = 0);

  /**
   * Adds the pid to the archive file passed to it.
   */
  std::string createArchiveFileName();
  /**
   * Returns the archiveFileName
   */
  std::string getArchiveFileName();
  /**
   * Gets the archive size limit in bytes.
   */
  int64_t getArchiveFileSizeLimit();
  /**
   * Gets the archive disk space limit in bytes.
   */
  int64_t getArchiveDiskSpaceLimit();
  /**
   * Gets the sample rate in milliseconds
   */
  int64_t getSampleRate();
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
  ACE_Recursive_Thread_Mutex& getStatListMutex();
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
  int32_t svc(void);

  /**
   * Method to know whether the sampling thread is running or not.
   */
  bool isRunning();

  ~HostStatSampler();

 private:
  ACE_Recursive_Thread_Mutex m_samplingLock;
  bool m_adminError;
  // Related to ACE Thread.
  bool m_running;
  bool m_stopRequested;
  volatile bool m_isStatDiskSpaceEnabled;
  StatArchiveWriter* m_archiver;
  StatSamplerStats* m_samplerStats;
  const char* m_durableClientId;
  uint32_t m_durableTimeout;

  std::string m_archiveFileName;
  int64_t m_archiveFileSizeLimit;
  int64_t m_archiveDiskSpaceLimit;
  int64_t m_sampleRate;
  StatisticsManager* m_statMngr;
  Cache* m_cache;

  int64_t m_pid;
  system_clock::time_point m_startTime;

  std::string initStatFileWithExt();
  /**
   * The archiveFile, after it exceeds archiveFileSizeLimit should be rolled
   * to a new file name. This integer rollIndex will be used to format the
   * file name into which the current archiveFile will be renamed.
   */
  int32_t rollIndex;
  /**
   * This function rolls the existing archive file
   */
  int32_t rollArchive(std::string filename);
  /**
   * This function check whether the filename has gfs ext or not
   * If it is not there it adds and then returns the new filename.
   */
  std::string chkForGFSExt(std::string filename);

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
};
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_HOSTSTATSAMPLER_H_
