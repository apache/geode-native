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

#include <algorithm>
#include <chrono>
#include <regex>
#include <thread>
#include <utility>
#include <vector>

#include <ace/ACE.h>
#include <ace/Dirent_Selector.h>
#include <ace/INET_Addr.h>
#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_sys_utsname.h>
#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>
#include <boost/range/adaptors.hpp>

#include <geode/CacheFactory.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "../CacheImpl.hpp"
#include "../ClientHealthStats.hpp"
#include "../ClientProxyMembershipID.hpp"
#include "../CppCacheLibrary.hpp"
#include "../DistributedSystem.hpp"
#include "../TcrConnectionManager.hpp"
#include "../util/Log.hpp"
#include "GeodeStatisticsFactory.hpp"
#include "StatArchiveWriter.hpp"

namespace apache {
namespace geode {
namespace statistics {
namespace globals {

boost::filesystem::path g_statFileWithExt;

}  // namespace globals
}  // namespace statistics
}  // namespace geode
}  // namespace apache

namespace {

// extern "C" {

int selector(const dirent* d) {
  std::string inputname(d->d_name);
  std::string filebasename = ACE::basename(
      apache::geode::statistics::globals::g_statFileWithExt.c_str());
  size_t actualHyphenPos = filebasename.find_last_of('.');
  if (strcmp(filebasename.c_str(), d->d_name) == 0) return 1;
  size_t fileExtPos = inputname.find_last_of('.');
  std::string extName = inputname.substr(fileExtPos + 1, inputname.length());
  if (strcmp(extName.c_str(), "gfs") != 0) {
    return 0;
  }
  if (fileExtPos != std::string::npos) {
    std::string tempname = inputname.substr(0, fileExtPos);
    size_t fileHyphenPos = tempname.find_last_of('-');
    if (fileHyphenPos != std::string::npos) {
      std::string buff1 = tempname.substr(0, fileHyphenPos);
      if (filebasename.find(buff1) == std::string::npos) {
        return 0;
      }
      if (fileHyphenPos != actualHyphenPos) {
        return 0;
      }
      std::string buff = tempname.substr(fileHyphenPos + 1,
                                         tempname.length() - fileHyphenPos - 1);
      for (std::string::iterator iter = buff.begin(); iter != buff.end();
           ++iter) {
        if (*iter < '0' || *iter > '9') {
          return 0;
        }
      }
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

int comparator(const dirent** d1, const dirent** d2) {
  if (strlen((*d1)->d_name) < strlen((*d2)->d_name)) {
    return -1;
  } else if (strlen((*d1)->d_name) > strlen((*d2)->d_name)) {
    return 1;
  }
  int diff = std::strcmp((*d1)->d_name, (*d2)->d_name);
  if (diff < 0) {
    return -1;
  } else if (diff > 0) {
    return 1;
  } else {
    return 0;
  }
}

}  // namespace

namespace apache {
namespace geode {
namespace statistics {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

using client::Exception;

static const boost::filesystem::path GFS_EXTENSION(".gfs");

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
  m_cache = cache;
  m_samplerStats = std::unique_ptr<StatSamplerStats>(
      new StatSamplerStats(statMngr->getStatisticsFactory()));
  m_statMngr = statMngr;

  initStatDiskSpaceEnabled();
}

HostStatSampler::HostStatSampler(boost::filesystem::path filePath,
                                 std::chrono::milliseconds sampleRate,
                                 size_t statFileLimit,
                                 size_t statDiskSpaceLimit)
    : m_adminError(false),
      m_running(false),
      m_stopRequested(false),
      m_isStatDiskSpaceEnabled(statDiskSpaceLimit != 0),
      m_archiveFileName(std::move(filePath)),
      m_archiveFileSizeLimit(
          std::min(statFileLimit * mebibyte, MAX_STATS_FILE_LIMIT)),
      m_archiveDiskSpaceLimit(statDiskSpaceLimit * mebibyte),
      m_spaceUsed(0),
      m_sampleRate(sampleRate),
      m_pid(boost::this_process::get_id()),
      m_startTime(system_clock::now()),
      m_rollIndex(0) {
  if (m_isStatDiskSpaceEnabled) {
    if (m_archiveFileSizeLimit == 0 ||
        m_archiveFileSizeLimit > m_archiveDiskSpaceLimit) {
      m_archiveFileSizeLimit = m_archiveDiskSpaceLimit;
    }
  }
}

void HostStatSampler::initStatDiskSpaceEnabled() {
  if (m_isStatDiskSpaceEnabled) {
    // TODO does this need to be global?
    globals::g_statFileWithExt = initStatFileWithExt();

    initRollIndex();

    FILE* existingFile = fopen(globals::g_statFileWithExt.c_str(), "r");
    if (existingFile != nullptr && m_archiveFileSizeLimit > 0) {
      fclose(existingFile);
      existingFile = nullptr;
      changeArchive(globals::g_statFileWithExt);
    } else {
      writeGfs();
    }
    if (existingFile != nullptr) {
      fclose(existingFile);
      existingFile = nullptr;
    }
  }
}

void HostStatSampler::initRollIndex() {
  const std::regex statsFilter(m_archiveFileName.stem().string() +
                               R"(-([\d]+))" +
                               m_archiveFileName.extension().string());

  auto dir = m_archiveFileName.parent_path();
  if (dir.empty()) {
    dir = ".";
  }

  for (auto& entry :
       boost::make_iterator_range(boost::filesystem::directory_iterator(dir),
                                  {}) |
           boost::adaptors::filtered(
               static_cast<bool (*)(const boost::filesystem::path&)>(
                   &boost::filesystem::is_regular_file))) {
    std::smatch match;
    if (std::regex_match(entry.path().filename().string(), match,
                         statsFilter)) {
      m_rollIndex = std::max(m_rollIndex, std::stoi(match[1].str()) + 1);
    }
  }
}

boost::filesystem::path HostStatSampler::initStatFileWithExt() {
  return chkForGFSExt(createArchiveFilename());
}

HostStatSampler::~HostStatSampler() noexcept = default;

const boost::filesystem::path& HostStatSampler::createArchiveFilename() {
  if (!m_isStatDiskSpaceEnabled) {
    const auto pid = std::to_string(boost::this_process::get_id());

    if (!m_archiveFileName.has_extension()) {
      m_archiveFileName += "-" + pid;
    } else {
      m_archiveFileName = m_archiveFileName.parent_path() /
                          m_archiveFileName.stem() += "-" + pid;
    }
    m_archiveFileName += GFS_EXTENSION;
  }

  return m_archiveFileName;
}

const boost::filesystem::path& HostStatSampler::getArchiveFilename() const {
  return m_archiveFileName;
}

size_t HostStatSampler::getArchiveFileSizeLimit() const {
  return m_archiveFileSizeLimit;
}

size_t HostStatSampler::getArchiveDiskSpaceLimit() const {
  return m_archiveDiskSpaceLimit;
}

std::chrono::milliseconds HostStatSampler::getSampleRate() const {
  return m_sampleRate;
}

void HostStatSampler::accountForTimeSpentWorking(int64_t nanosSpentWorking) {
  m_samplerStats->tookSample(nanosSpentWorking);
}

std::recursive_mutex& HostStatSampler::getStatListMutex() {
  return m_statMngr->getListMutex();
}

std::vector<Statistics*>& HostStatSampler::getStatistics() {
  return m_statMngr->getStatsList();
}

std::vector<Statistics*>& HostStatSampler::getNewStatistics() {
  return m_statMngr->getNewlyAddedStatsList();
}

int64_t HostStatSampler::getSystemId() { return m_pid; }

system_clock::time_point HostStatSampler::getSystemStartTime() {
  return m_startTime;
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
    m_stopRequested = true;
    return;
  }

  filename = chkForGFSExt(filename);

  if (m_archiver) {
    m_archiver->closeFile();
  }

  rollArchive(filename);

  m_archiver.reset(new StatArchiveWriter(filename.string(), this, m_cache));
}

boost::filesystem::path HostStatSampler::chkForGFSExt(
    const boost::filesystem::path& filename) const {
  if (filename.extension() == GFS_EXTENSION) {
    return filename;
  }

  auto tmp = filename;
  if (m_isStatDiskSpaceEnabled) {
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
    newFilename += std::to_string(m_rollIndex++);
    newFilename += extension;

    if (boost::filesystem::exists(newFilename)) {
      continue;
    }

    boost::filesystem::rename(filename, newFilename);
    break;
  }
}

void HostStatSampler::start() {
  if (!m_running.exchange(true)) {
    m_thread = std::thread(&HostStatSampler::svc, this);
  }
}

void HostStatSampler::stop() {
  m_stopRequested = true;
  m_thread.join();
}

bool HostStatSampler::isRunning() const { return m_running; }

void HostStatSampler::putStatsInAdminRegion() {
  try {
    // Get Values of gets, puts,misses,listCalls,numThreads
    static bool initDone = false;
    static std::string clientId = "";
    auto adminRgn = m_statMngr->getAdminRegion();
    if (adminRgn == nullptr) return;
    auto conn_man = adminRgn->getConnectionManager();
    if (conn_man->isNetDown()) {
      return;
    }
    client::TryReadGuard _guard(adminRgn->getRWLock(), adminRgn->isDestroyed());
    if (!adminRgn->isDestroyed()) {
      if (conn_man->getNumEndPoints() > 0) {
        if (!initDone) {
          adminRgn->init();
          initDone = true;
        }
        int puts = 0, gets = 0, misses = 0, numListeners = 0, numThreads = 0,
            creates = 0;
        int64_t cpuTime = 0;
        auto gf = m_statMngr->getStatisticsFactory();
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
          ACE_TCHAR hostName[256];
          ACE_OS::hostname(hostName, sizeof(hostName) - 1);
          ACE_INET_Addr driver("", hostName, "tcp");

          uint16_t hostPort = 0;

          auto memId = conn_man->getCacheImpl()
                           ->getClientProxyMembershipIDFactory()
                           .create(hostName, driver, hostPort,
                                   m_durableClientId, m_durableTimeout);
          clientId = memId->getDSMemberIdForThinClientUse();
        }

        auto keyPtr = client::CacheableString::create(clientId.c_str());
        adminRgn->put(keyPtr, obj);
      }
    }
  } catch (const Exception&) {
    m_adminError = true;
  }
}

void HostStatSampler::writeGfs() {
  const auto& archiveFilename = createArchiveFilename();
  changeArchive(archiveFilename);
}

void HostStatSampler::forceSample() {
  std::lock_guard<decltype(m_samplingLock)> guard(m_samplingLock);

  if (m_archiver) {
    m_archiver->sample();
    m_archiver->flush();
  }
}

void HostStatSampler::doSample(const boost::filesystem::path& archiveFilename) {
  std::lock_guard<decltype(m_samplingLock)> guard(m_samplingLock);

  if (!m_adminError) {
    putStatsInAdminRegion();
  }

  if (m_archiver) {
    m_archiver->sample();

    if (m_archiveFileSizeLimit != 0) {
      auto size = m_archiver->getSampleSize();
      auto bytesWritten = m_archiver->bytesWritten();
      if (bytesWritten > (m_archiveFileSizeLimit - size)) {
        // roll the archive
        changeArchive(archiveFilename);
      }
    }
    m_spaceUsed += m_archiver->bytesWritten();
    // delete older stat files if disk limit is about to be exceeded.
    if ((m_archiveDiskSpaceLimit != 0) &&
        (m_spaceUsed >=
         (m_archiveDiskSpaceLimit - m_archiver->getSampleSize()))) {
      checkDiskLimit();
    }

    // It will flush the contents to the archive file, in every
    // sample run.

    m_archiver->flush();
  }
}

void HostStatSampler::checkDiskLimit() {
  // TODO remove
  globals::g_statFileWithExt = initStatFileWithExt();

  std::vector<std::pair<std::string, size_t> > fileInfo;
  m_spaceUsed = 0;
  char fullpath[512] = {0};
  std::string dirname = ACE::dirname(m_archiveFileName.c_str());
  ACE_stat statBuf = {};
  ACE_Dirent_Selector sds;
  int status = sds.open(dirname.c_str(), selector, comparator);
  if (status != -1) {
    for (int i = 1; i < sds.length(); i++) {
      std::snprintf(fullpath, 512, "%s%c%s", dirname.c_str(),
                    ACE_DIRECTORY_SEPARATOR_CHAR, sds[i]->d_name);
      ACE_OS::stat(fullpath, &statBuf);
      fileInfo.emplace_back(fullpath, statBuf.st_size);
      m_spaceUsed += fileInfo[i - 1].second;
    }
    if (m_archiver) {
      m_spaceUsed += m_archiver->bytesWritten();
    }
    sds.close();
  }
  int fileIndex = 0;
  while ((m_spaceUsed > m_archiveDiskSpaceLimit)) {
    auto fileSize = fileInfo[fileIndex].second;
    if (ACE_OS::unlink(fileInfo[fileIndex].first.c_str()) == 0) {
      m_spaceUsed -= fileSize;
    } else {
      LOGWARN("%s\t%s\n", "Could not delete",
              fileInfo[fileIndex].first.c_str());
    }
    fileIndex++;
  }
}

void HostStatSampler::svc(void) {
  client::DistributedSystemImpl::setThreadName("NC HSS Thread");
  try {
    // createArchiveFileName instead of getArchiveFileName here because
    // for the first time the sampler needs to add the pid to the filename
    // passed to it.
    auto archiveFilename = createArchiveFilename();
    if (!m_isStatDiskSpaceEnabled) {
      changeArchive(archiveFilename);
    }
    auto samplingRate = milliseconds(getSampleRate());
    bool gotexception = false;
    int waitTime = 0;
    while (!m_stopRequested) {
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
        while (!m_stopRequested && sleepDuration > milliseconds::zero()) {
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
    m_samplerStats->close();
    if (m_archiver != nullptr) {
      m_archiver->close();
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
  m_running = false;
}

const boost::filesystem::path& HostStatSampler::getStatFileWithExt() {
  return globals::g_statFileWithExt;
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
