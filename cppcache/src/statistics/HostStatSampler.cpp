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

#include "HostStatSampler.hpp"

#include <chrono>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/thread/lock_types.hpp>

#include <geode/CacheFactory.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "../AdminRegion.hpp"
#include "../CacheImpl.hpp"
#include "../ClientHealthStats.hpp"
#include "../ClientProxyMembershipID.hpp"
#include "../CppCacheLibrary.hpp"
#include "../TcrConnectionManager.hpp"
#include "GeodeStatisticsFactory.hpp"
#include "StatArchiveWriter.hpp"

namespace apache {
namespace geode {
namespace statistics {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

using client::Exception;

constexpr auto GFS_EXTENSION = ".gfs";

constexpr size_t kibibyte = 1024;
constexpr size_t mebibyte = kibibyte * 1024;
constexpr size_t gibibyte = mebibyte * 1024;

constexpr size_t MAX_STATS_FILE_LIMIT = 1 * gibibyte;

HostStatSampler::HostStatSampler(boost::filesystem::path filePath,
                                 std::chrono::milliseconds sampleRate,
                                 StatisticsManager* statMngr, CacheImpl* cache,
                                 size_t statFileLimit,
                                 size_t statDiskSpaceLimit)

    : HostStatSampler(std::move(filePath), sampleRate, statFileLimit,
                      statDiskSpaceLimit) {
  cache_ = cache;
  samplerStats_ = std::unique_ptr<StatSamplerStats>(
      new StatSamplerStats(statMngr->getStatisticsFactory()));
  statsMgr_ = statMngr;

  initStatDiskSpaceEnabled();
}

HostStatSampler::HostStatSampler(boost::filesystem::path filePath,
                                 std::chrono::milliseconds sampleRate,
                                 size_t statFileLimit,
                                 size_t statDiskSpaceLimit)
    : adminError_(false),
      running_(false),
      stopRequested_(false),
      isStatDiskSpaceEnabled_(statDiskSpaceLimit != 0),
      archiveFileName_(std::move(filePath)),
      archiveFileSizeLimit_(
          (std::min)(statFileLimit * mebibyte, MAX_STATS_FILE_LIMIT)),
      archiveDiskSpaceLimit_(statDiskSpaceLimit * mebibyte),
      spaceUsed_(0),
      sampleRate_(sampleRate),
      pid_(boost::this_process::get_id()),
      startTime_(system_clock::now()),
      rollIndex_(0) {
  if (isStatDiskSpaceEnabled_) {
    if (archiveFileSizeLimit_ == 0 ||
        archiveFileSizeLimit_ > archiveDiskSpaceLimit_) {
      archiveFileSizeLimit_ = archiveDiskSpaceLimit_;
    }
  }
}

void HostStatSampler::initStatDiskSpaceEnabled() {
  if (isStatDiskSpaceEnabled_) {
    initStatFileWithExt();

    initRollIndex();

    auto exists = boost::filesystem::exists(archiveFileName_);
    if (exists && archiveFileSizeLimit_ > 0) {
      changeArchive(archiveFileName_);
    } else {
      writeGfs();
    }
  }
}

void HostStatSampler::initRollIndex() {
  forEachIndexStatFile(
      [&](const int32_t index, const boost::filesystem::path&) {
        rollIndex_ = (std::max)(rollIndex_, index + 1);
      });
}

boost::filesystem::path HostStatSampler::initStatFileWithExt() {
  return chkForGFSExt(createArchiveFilename());
}

HostStatSampler::~HostStatSampler() noexcept = default;

const boost::filesystem::path& HostStatSampler::createArchiveFilename() {
  if (!isStatDiskSpaceEnabled_) {
    const auto pid = std::to_string(boost::this_process::get_id());

    if (!archiveFileName_.has_extension()) {
      archiveFileName_ += "-" + pid;
    } else {
      archiveFileName_ =
          archiveFileName_.parent_path() / archiveFileName_.stem() += "-" + pid;
    }
    archiveFileName_ += GFS_EXTENSION;
  }

  return archiveFileName_;
}

const boost::filesystem::path& HostStatSampler::getArchiveFilename() const {
  return archiveFileName_;
}

size_t HostStatSampler::getArchiveFileSizeLimit() const {
  return archiveFileSizeLimit_;
}

size_t HostStatSampler::getArchiveDiskSpaceLimit() const {
  return archiveDiskSpaceLimit_;
}

std::chrono::milliseconds HostStatSampler::getSampleRate() const {
  return sampleRate_;
}

void HostStatSampler::accountForTimeSpentWorking(int64_t nanosSpentWorking) {
  samplerStats_->tookSample(nanosSpentWorking);
}

std::recursive_mutex& HostStatSampler::getStatListMutex() {
  return statsMgr_->getListMutex();
}

std::vector<Statistics*>& HostStatSampler::getStatistics() {
  return statsMgr_->getStatsList();
}

std::vector<Statistics*>& HostStatSampler::getNewStatistics() {
  return statsMgr_->getNewlyAddedStatsList();
}

int64_t HostStatSampler::getSystemId() { return pid_; }

system_clock::time_point HostStatSampler::getSystemStartTime() {
  return startTime_;
}

const std::string& HostStatSampler::getSystemDirectoryPath() {
  return client::CppCacheLibrary::getProductDir();
}

const std::string& HostStatSampler::getProductDescription() const {
  return client::CacheFactory::getProductDescription();
}

void HostStatSampler::changeArchive(boost::filesystem::path filename) {
  if (filename.empty()) {
    // terminate the sampling thread
    stopRequested_ = true;
    return;
  }

  filename = chkForGFSExt(filename);

  if (archiver_) {
    archiver_->closeFile();
  }

  rollArchive(filename);

  archiver_.reset(new StatArchiveWriter(filename.string(), this, cache_));
}

boost::filesystem::path HostStatSampler::chkForGFSExt(
    const boost::filesystem::path& filename) const {
  if (filename.extension() == GFS_EXTENSION) {
    return filename;
  }

  auto tmp = filename;
  if (isStatDiskSpaceEnabled_) {
    return tmp += GFS_EXTENSION;
  }
  return tmp.replace_extension(GFS_EXTENSION);
}

void HostStatSampler::rollArchive(const boost::filesystem::path& filename) {
  if (!boost::filesystem::exists(filename) ||
      boost::filesystem::is_empty(filename)) {
    return;
  }

  const auto extension = filename.extension();
  if (extension.empty()) {
    throw client::IllegalArgumentException("Missing extension.");
  }

  while (true) {
    auto newFilename = filename.parent_path() / filename.stem();
    newFilename += "-";
    newFilename += std::to_string(rollIndex_++);
    newFilename += extension;

    if (boost::filesystem::exists(newFilename)) {
      continue;
    }

    boost::filesystem::rename(filename, newFilename);
    break;
  }
}

void HostStatSampler::start() {
  if (!running_.exchange(true)) {
    thread_ = std::thread(&HostStatSampler::svc, this);
  }
}

void HostStatSampler::stop() {
  stopRequested_ = true;
  thread_.join();
}

bool HostStatSampler::isRunning() const { return running_; }

void HostStatSampler::putStatsInAdminRegion() {
  try {
    // Get Values of gets, puts,misses,listCalls,numThreads
    static bool initDone = false;
    static std::string clientId = "";
    auto adminRgn = statsMgr_->getAdminRegion();
    if (adminRgn == nullptr) return;
    auto conn_man = adminRgn->getConnectionManager();
    if (conn_man->isNetDown()) {
      return;
    }

    boost::shared_lock<boost::shared_mutex> guard{adminRgn->getMutex()};
    if (!adminRgn->isDestroyed()) {
      if (conn_man->getNumEndPoints() > 0) {
        if (!initDone) {
          adminRgn->init();
          initDone = true;
        }
        int puts = 0, gets = 0, misses = 0, numListeners = 0, numThreads = 0,
            creates = 0;
        int64_t cpuTime = 0;
        auto gf = statsMgr_->getStatisticsFactory();
        if (gf) {
          const auto cacheStatType = gf->findType("CachePerfStats");
          if (cacheStatType) {
            Statistics* cachePerfStats =
                gf->findFirstStatisticsByType(cacheStatType);
            if (cachePerfStats) {
              puts = cachePerfStats->getInt("puts");
              gets = cachePerfStats->getInt("gets");
              misses = cachePerfStats->getInt("misses");
              creates = cachePerfStats->getInt("creates");
              numListeners =
                  cachePerfStats->getInt("cacheListenerCallsCompleted");
              puts += creates;
            }
          }
        }
        static auto numCPU = std::thread::hardware_concurrency();
        auto obj = client::ClientHealthStats::create(
            gets, puts, misses, numListeners, numThreads, cpuTime, numCPU);
        if (clientId.empty()) {
          auto memId = conn_man->getCacheImpl()
                           ->getClientProxyMembershipIDFactory()
                           .create(durableClientId_, durableTimeout_);
          clientId = memId->getDSMemberIdForThinClientUse();
        }

        auto keyPtr = client::CacheableString::create(clientId.c_str());
        adminRgn->put(keyPtr, obj);
      }
    }
  } catch (const Exception&) {
    adminError_ = true;
  }
}

void HostStatSampler::writeGfs() {
  const auto& archiveFilename = createArchiveFilename();
  changeArchive(archiveFilename);
}

void HostStatSampler::forceSample() {
  std::lock_guard<decltype(samplingMutex_)> guard(samplingMutex_);

  if (archiver_) {
    archiver_->sample();
    archiver_->flush();
  }
}

void HostStatSampler::doSample(const boost::filesystem::path& archiveFilename) {
  std::lock_guard<decltype(samplingMutex_)> guard(samplingMutex_);

  if (!adminError_) {
    putStatsInAdminRegion();
  }

  if (archiver_) {
    archiver_->sample();

    if (archiveFileSizeLimit_ != 0) {
      auto size = archiver_->getSampleSize();
      auto bytesWritten = archiver_->bytesWritten();
      if (bytesWritten > (archiveFileSizeLimit_ - size)) {
        // roll the archive
        changeArchive(archiveFilename);
      }
    }
    spaceUsed_ += archiver_->bytesWritten();
    // delete older stat files if disk limit is about to be exceeded.
    if ((archiveDiskSpaceLimit_ != 0) &&
        (spaceUsed_ >= (archiveDiskSpaceLimit_ - archiver_->getSampleSize()))) {
      checkDiskLimit();
    }

    // It will flush the contents to the archive file, in every
    // sample run.

    archiver_->flush();
  }
}

template <typename _Function>
void HostStatSampler::forEachIndexStatFile(_Function function) const {
  const boost::regex statsFilter(archiveFileName_.stem().string() +
                                 R"(-([\d]+))" +
                                 archiveFileName_.extension().string());

  auto dir = archiveFileName_.parent_path();
  if (dir.empty()) {
    dir = boost::filesystem::current_path();
  }

  for (const auto& entry :
       boost::make_iterator_range(boost::filesystem::directory_iterator(dir),
                                  {}) |
           boost::adaptors::filtered(
               static_cast<bool (*)(const boost::filesystem::path&)>(
                   &boost::filesystem::is_regular_file))) {
    boost::smatch match;
    const auto& file = entry.path();
    const auto filename = file.filename();
    const auto& filenameStr = filename.string();
    if (boost::regex_match(filenameStr, match, statsFilter)) {
      const auto index = std::stoi(match[1].str());
      function(index, file);
    }
  }
}

void HostStatSampler::checkDiskLimit() {
  spaceUsed_ = 0;

  std::map<int32_t, std::pair<boost::filesystem::path, size_t>> indexedFiles;
  forEachIndexStatFile(
      [&](const int32_t index, const boost::filesystem::path& file) {
        const auto size = boost::filesystem::file_size(file);
        indexedFiles.emplace(index, std::make_pair(file, size));
        spaceUsed_ += size;
      });

  if (archiver_) {
    spaceUsed_ += archiver_->bytesWritten();
  }

  for (const auto& i : indexedFiles) {
    if (spaceUsed_ > archiveDiskSpaceLimit_) {
      const auto& file = i.second.first;
      const auto size = i.second.second;
      try {
        boost::filesystem::remove(file);
        spaceUsed_ -= size;
      } catch (boost::filesystem::filesystem_error& e) {
        LOGWARN("Could not delete " + file.string() + ": " + e.what());
      }
    }
  }
}

void HostStatSampler::svc(void) {
  client::Log::setThreadName("NC HSS Thread");
  try {
    // createArchiveFileName instead of getArchiveFileName here because
    // for the first time the sampler needs to add the pid to the filename
    // passed to it.
    auto archiveFilename = createArchiveFilename();
    if (!isStatDiskSpaceEnabled_) {
      changeArchive(archiveFilename);
    }
    auto samplingRate = milliseconds(getSampleRate());
    bool gotexception = false;
    int waitTime = 0;
    while (!stopRequested_) {
      try {
        if (gotexception) {
          std::this_thread::sleep_for(std::chrono::seconds(1));

          waitTime++;
          if (waitTime < 60) {  // sleep for minute and then try to recreate
            continue;
          }
          waitTime = 0;
          gotexception = false;
          changeArchive(archiveFilename);
        }

        auto sampleStart = high_resolution_clock::now();

        doSample(archiveFilename);

        nanoseconds spentWorking = high_resolution_clock::now() - sampleStart;
        // updating the sampler statistics
        accountForTimeSpentWorking(spentWorking.count());

        // TODO: replace with condition on m_stopRequested
        auto sleepDuration =
            samplingRate - duration_cast<milliseconds>(spentWorking);
        static const auto wakeInterval = milliseconds(100);
        while (!stopRequested_ && sleepDuration > milliseconds::zero()) {
          std::this_thread::sleep_for(
              sleepDuration > wakeInterval ? wakeInterval : sleepDuration);
          sleepDuration -= wakeInterval;
        }
      } catch (Exception& e) {
        // log the exception and let the thread exit.
        LOGERROR("Exception in statistics sampler thread: %s: %s",
                 e.getName().c_str(), e.what());
        // now close current archiver and see if we can start new one
        gotexception = true;
      } catch (...) {
        LOGERROR("Unknown Exception in statistics sampler thread: ");
        gotexception = true;
      }
    }
    samplerStats_->close();
    if (archiver_ != nullptr) {
      archiver_->close();
    }
  } catch (Exception& e) {
    // log the exception and let the thread exit.
    LOGERROR("Exception in statistics sampler thread: %s: %s",
             e.getName().c_str(), e.what());
  } /* catch (...) {
       // log the exception and let the thread exit.
       LOGERROR("Exception in sampler thread ");
       closeSpecialStats();
   }*/
  running_ = false;
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
