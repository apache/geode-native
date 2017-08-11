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
#include <ReadWriteLock.hpp>
#include <CacheImpl.hpp>
#include <ThinClientPoolDM.hpp>
#include "GeodeStatisticsFactory.hpp"
#include <ClientHealthStats.hpp>
#include "HostStatHelper.hpp"
#include <chrono>
#include <thread>

namespace apache {
namespace geode {
namespace statistics {

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

const char* PoolStatsSampler::NC_PSS_Thread = "NC PSS Thread";

PoolStatsSampler::PoolStatsSampler(int64_t sampleRate, CacheImpl* cache,
                                   ThinClientPoolDM* distMan)
    : m_sampleRate(sampleRate),
      m_distMan(distMan),
      m_statisticsFactory(cache->getDistributedSystem()
                              .getStatisticsManager()
                              ->getStatisticsFactory()) {
  m_running = false;
  m_stopRequested = false;
  m_adminRegion = AdminRegion::create(cache, distMan);
}

PoolStatsSampler::~PoolStatsSampler() {
  // GF_SAFE_DELETE(m_adminRegion);
}

int32_t PoolStatsSampler::svc() {
  DistributedSystemImpl::setThreadName(NC_PSS_Thread);
  auto msSpentWorking = milliseconds::zero();
  auto samplingRate = milliseconds(m_sampleRate);
  // ACE_Guard < ACE_Recursive_Thread_Mutex > _guard( m_lock );
  while (!m_stopRequested) {
    auto sampleStart = high_resolution_clock::now();
    putStatsInAdminRegion();
    nanoseconds spentWorking = high_resolution_clock::now() - sampleStart;
    auto sleepDuration =
        samplingRate - duration_cast<milliseconds>(spentWorking);
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
        StatisticsType* cacheStatType =
            m_statisticsFactory->findType("CachePerfStats");
        if (cacheStatType) {
          Statistics* cachePerfStats =
              m_statisticsFactory->findFirstStatisticsByType(cacheStatType);
          if (cachePerfStats) {
            puts = cachePerfStats->getInt((char*)"puts");
            gets = cachePerfStats->getInt((char*)"gets");
            misses = cachePerfStats->getInt((char*)"misses");
            creates = cachePerfStats->getInt((char*)"creates");
            numListeners =
                cachePerfStats->getInt((char*)"cacheListenerCallsCompleted");
            puts += creates;
          }
        }
        numThreads = HostStatHelper::getNumThreads();
        cpuTime = HostStatHelper::getCpuTime();
      }
      static int numCPU = ACE_OS::num_processors();
      ClientHealthStatsPtr obj = ClientHealthStats::create(
          gets, puts, misses, numListeners, numThreads, cpuTime, numCPU);
      ClientProxyMembershipID* memId = m_distMan->getMembershipId();
      clientId = memId->getDSMemberIdForThinClientUse();
      CacheableKeyPtr keyPtr = CacheableString::create(clientId.c_str());
      m_adminRegion->put(keyPtr, obj);
    }
  } catch (const AllConnectionsInUseException&) {
    LOGDEBUG("All connection are in use, trying again.");
  } catch (const NotConnectedException& ex) {
    if (std::dynamic_pointer_cast<NoAvailableLocatorsException>(
            ex.getCause())) {
      LOGDEBUG("No locators available, trying again.");
    } else {
      LOGDEBUG("Not connected to geode, trying again.");
    }
  } catch (...) {
    LOGDEBUG("Exception occurred, trying again.");
  }
}
}  // namespace statistics
}  // namespace geode
}  // namespace apache
