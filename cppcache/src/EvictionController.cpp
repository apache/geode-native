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

#include "EvictionController.hpp"

#include <chrono>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "DistributedSystem.hpp"
#include "ReadWriteLock.hpp"
#include "RegionInternal.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

const char* EvictionController::NC_EC_Thread = "NC EC Thread";
EvictionController::EvictionController(size_t maxHeapSize,
                                       int32_t heapSizeDelta, CacheImpl* cache)
    : m_run(false),
      m_maxHeapSize(maxHeapSize * 1024 * 1024),
      m_heapSizeDelta(heapSizeDelta),
      m_cacheImpl(cache),
      m_currentHeapSize(0),
      m_evictionThread(this) {
  LOGINFO("Maximum heap size for Heap LRU set to %ld bytes", m_maxHeapSize);
}

void EvictionController::start() {
  m_evictionThread.start();

  m_run = true;
  m_thread = std::thread(&EvictionController::svc, this);

  LOGFINE("Eviction Controller started");
}

void EvictionController::stop() {
  m_run = false;
  m_queueCondition.notify_one();
  m_thread.join();

  m_evictionThread.stop();

  m_regions.clear();
  m_queue.clear();

  LOGFINE("Eviction controller stopped");
}

void EvictionController::svc() {
  DistributedSystemImpl::setThreadName(NC_EC_Thread);

  int64_t pendingEvictions = 0;

  while (m_run) {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_queueCondition.wait(lock, [this] { return !m_run || !m_queue.empty(); });

    while (!m_queue.empty()) {
      auto readInfo = m_queue.front();
      m_queue.pop_front();
      if (0 != readInfo) {
        processHeapInfo(readInfo, pendingEvictions);
      }
    }
  }
}

void EvictionController::updateRegionHeapInfo(int64_t info) {
  std::unique_lock<std::mutex> lock(m_queueMutex);
  m_queue.push_back(info);
  m_queueCondition.notify_one();

  // We could block here if we wanted to prevent any further memory use
  // until the evictions had been completed.
}

void EvictionController::processHeapInfo(int64_t& readInfo,
                                         int64_t& pendingEvictions) {
  m_currentHeapSize += readInfo;

  // Waiting for evictions to catch up.Negative numbers
  // are attributed to evictions that were triggered by the
  // EvictionController
  int64_t sizeToCompare = 0;
  if (readInfo < 0 && pendingEvictions > 0) {
    pendingEvictions += readInfo;
    if (pendingEvictions < 0) pendingEvictions = 0;
    return;  // as long as you are still evicting, don't do the rest of the work
  } else {
    sizeToCompare = m_currentHeapSize - pendingEvictions;
  }

  if (sizeToCompare > m_maxHeapSize) {
    // Check if overflow is above the delta
    int64_t sizeOverflow = sizeToCompare - m_maxHeapSize;

    // Calculate the percentage that we are over the limit.
    int32_t fractionalOverflow =
        static_cast<int32_t>(((sizeOverflow * 100) % m_maxHeapSize) > 0) ? 1
                                                                         : 0;
    int32_t percentage =
        static_cast<int32_t>((sizeOverflow * 100) / m_maxHeapSize) +
        fractionalOverflow;
    // need to evict
    int32_t evictionPercentage =
        static_cast<int32_t>(percentage + m_heapSizeDelta);
    int32_t bytesToEvict =
        static_cast<int32_t>((sizeToCompare * evictionPercentage) / 100);
    pendingEvictions += bytesToEvict;
    if (evictionPercentage > 100) evictionPercentage = 100;
    orderEvictions(evictionPercentage);
  }
}

void EvictionController::registerRegion(const std::string& name) {
  boost::unique_lock<decltype(m_regionLock)> lock(m_regionLock);
  m_regions.push_back(name);
  LOGFINE("Registered region with Heap LRU eviction controller: name is " +
          name);
}

void EvictionController::deregisterRegion(const std::string& name) {
  // Iterate over regions vector and remove the one that we need to remove
  boost::unique_lock<decltype(m_regionLock)> lock(m_regionLock);

  const auto& removed =
      std::remove_if(m_regions.begin(), m_regions.end(),
                     [&](const std::string& region) { return region == name; });
  if (removed != m_regions.cend()) {
    LOGFINE("Deregistered region with Heap LRU eviction controller: name is " +
            name);
  }
  m_regions.erase(removed, m_regions.cend());
}

void EvictionController::orderEvictions(int32_t percentage) {
  m_evictionThread.putEvictionInfo(percentage);
}

void EvictionController::evict(int32_t percentage) {
  // TODO:  Shouldn't we take the CacheImpl::m_regions
  // lock here? Otherwise we might invoke eviction on a region
  // that has been destroyed or is being destroyed.
  // Its important to not hold this lock for too long
  // because it prevents new regions from getting created or destroyed
  // On the flip side, this requires a copy of the registered region list
  // every time eviction is ordered and that might not be cheap
  //@TODO: Discuss with team

  decltype(m_regions) regionTempVector;
  {
    boost::shared_lock<decltype(m_regionLock)> lock(m_regionLock);
    regionTempVector.reserve(m_regions.size());
    regionTempVector.insert(regionTempVector.end(), m_regions.begin(),
                            m_regions.end());
  }

  for (const auto& regionName : regionTempVector) {
    if (auto region = std::dynamic_pointer_cast<RegionInternal>(
            m_cacheImpl->getRegion(regionName))) {
      region->evict(percentage);
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
