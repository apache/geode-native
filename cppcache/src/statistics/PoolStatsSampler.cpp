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
#include <chrono>
#include <thread>

#include "PoolStatsSampler.hpp"
#include "GeodeStatisticsFactory.hpp"
#include "../ReadWriteLock.hpp"
#include "../CacheImpl.hpp"
#include "../ThinClientPoolDM.hpp"
#include "../ClientHealthStats.hpp"

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
    : m_sampleRate(sampleRate),
      m_distMan(distMan),
      m_statisticsFactory(
          cache->getStatisticsManager()->getStatisticsFactory()) {
  m_running = false;
  m_stopRequested = false;
  m_adminRegion = AdminRegion::create(cache, distMan);
}

PoolStatsSampler::~PoolStatsSampler() {
  // _GEODE_SAFE_DELETE(m_adminRegion);
}

int32_t PoolStatsSampler::svc() {
  DistributedSystemImpl::setThreadName(NC_PSS_Thread);
  // ACE_Guard < ACE_Recursive_Thread_Mutex > _guard( m_lock );
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
  return 0;
}

void PoolStatsSampler::start() {
  if (!m_running) {
    m_running = true;
    this->activate();
  }
}

void PoolStatsSampler::stop() {
  // ACE_Guard < ACE_Recursive_Thread_Mutex > _guard( m_lock );
  m_stopRequested = true;
  this->wait();
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
      static int numCPU = ACE_OS::num_processors();
      auto obj = ClientHealthStats::create(gets, puts, misses, numListeners,
                                           numThreads, cpuTime, numCPU);
      const auto memId = m_distMan->getMembershipId();
      clientId = memId->getDSMemberIdForThinClientUse();
      auto keyPtr = CacheableString::create(clientId.c_str());
      m_adminRegion->put(keyPtr, obj);
    }
  } catch (const AllConnectionsInUseException&) {
    LOGDEBUG("All connection are in use, trying again.");
  } catch (const NotConnectedException& ex) {
    try {
      std::rethrow_if_nested(ex);
    } catch (const NoAvailableLocatorsException&) {
      LOGDEBUG("No locators available, trying again.");
    } catch (...) {
      LOGDEBUG("Not connected to geode, trying again.");
    }
  } catch (...) {
    LOGDEBUG("Exception occurred, trying again.");
  }
}
}  // namespace statistics
}  // namespace geode
}  // namespace apache
