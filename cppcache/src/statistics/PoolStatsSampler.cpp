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

#include "PoolStatsSampler.hpp"

#include <string>

#include "../AdminRegion.hpp"
#include "../CacheImpl.hpp"
#include "../ClientHealthStats.hpp"
#include "../ReadWriteLock.hpp"
#include "../ThinClientPoolDM.hpp"
#include "GeodeStatisticsFactory.hpp"

namespace apache {
namespace geode {
namespace statistics {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

const char* PoolStatsSampler::NC_PSS_Thread = "NC PSS Thread";

PoolStatsSampler::PoolStatsSampler(milliseconds sampleRate, CacheImpl* cache,
                                   ThinClientPoolDM* distMan)
    : m_running(false),
      m_stopRequested(false),
      m_sampleRate(sampleRate),
      m_adminRegion(AdminRegion::create(cache, distMan)),
      m_distMan(distMan),
      m_statisticsFactory(
          cache->getStatisticsManager().getStatisticsFactory()) {}

void PoolStatsSampler::svc() {
  client::DistributedSystemImpl::setThreadName(NC_PSS_Thread);
  while (!m_stopRequested) {
    auto sampleStart = high_resolution_clock::now();
    putStatsInAdminRegion();
    nanoseconds spentWorking = high_resolution_clock::now() - sampleStart;
    auto sleepDuration =
        m_sampleRate - duration_cast<milliseconds>(spentWorking);
    static const auto wakeInterval = milliseconds(100);
    while (!m_stopRequested && sleepDuration > milliseconds::zero()) {
      std::this_thread::sleep_for(sleepDuration > wakeInterval ? wakeInterval
                                                               : sleepDuration);
      sleepDuration -= wakeInterval;
    }
  }
}

void PoolStatsSampler::start() {
  if (!m_running.exchange(true)) {
    m_thread = std::thread(&PoolStatsSampler::svc, this);
  }
}

void PoolStatsSampler::stop() {
  m_stopRequested = true;
  m_thread.join();
}

bool PoolStatsSampler::isRunning() { return m_running; }

void PoolStatsSampler::putStatsInAdminRegion() {
  // Get Values of gets, puts,misses,listCalls,numThread
  try {
    static std::string clientId = "";
    if (!m_adminRegion->isDestroyed()) {
      int puts = 0, gets = 0, misses = 0, numListeners = 0, numThreads = 0,
          creates = 0;
      int64_t cpuTime = 0;
      if (m_statisticsFactory) {
        if (const auto cacheStatType =
                m_statisticsFactory->findType("CachePerfStats")) {
          if (const auto cachePerfStats =
                  m_statisticsFactory->findFirstStatisticsByType(
                      cacheStatType)) {
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
      const auto memId = m_distMan->getMembershipId();
      clientId = memId->getDSMemberIdForThinClientUse();
      auto keyPtr = client::CacheableString::create(clientId.c_str());
      m_adminRegion->put(keyPtr, obj);
    }
  } catch (const client::AllConnectionsInUseException&) {
    LOG_DEBUG("All connection are in use, trying again.");
  } catch (const client::NotConnectedException& ex) {
    try {
      std::rethrow_if_nested(ex);
    } catch (const client::NoAvailableLocatorsException&) {
      LOG_DEBUG("No locators available, trying again.");
    } catch (...) {
      LOG_DEBUG("Not connected to geode, trying again.");
    }
  } catch (...) {
    LOG_DEBUG("Exception occurred, trying again.");
  }
}
}  // namespace statistics
}  // namespace geode
}  // namespace apache
