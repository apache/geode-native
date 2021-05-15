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

#include <boost/thread/lock_types.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "DistributedSystem.hpp"
#include "RegionInternal.hpp"
#include "util/Log.hpp"

namespace {
const char* const NC_EC_Thread = "NC EC Thread";
const std::chrono::seconds EVICTION_TIMEOUT{1};
}  // namespace

namespace apache {
namespace geode {
namespace client {

EvictionController::EvictionController(int64_t max_heap_size,
                                       int64_t heap_size_delta,
                                       CacheImpl* cache)
    : cache_{cache},
      running_{false},
      max_heap_size_{max_heap_size << 20ULL},
      heap_size_delta_{heap_size_delta / 100.0f},
      heap_size_{0} {
  LOG_INFO("Maximum heap size for Heap LRU set to %ld bytes", max_heap_size_);
}

void EvictionController::start() {
  running_ = true;
  thread_ = std::thread(&EvictionController::svc, this);

  LOG_FINE("Eviction Controller started");
}

void EvictionController::stop() {
  running_ = false;
  cv_.notify_one();
  thread_.join();

  regions_.clear();
  LOG_FINE("Eviction controller stopped");
}

void EvictionController::svc() {
  std::mutex mutex;
  DistributedSystemImpl::setThreadName(NC_EC_Thread);

  while (running_) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      cv_.wait(lock,
               [this] { return !running_ || heap_size_ > max_heap_size_; });
    }

    checkHeapSize();
  }
}

void EvictionController::incrementHeapSize(int64_t delta) {
  heap_size_ += delta;
  cv_.notify_one();

  // We could block here if we wanted to prevent any further memory use
  // until the evictions had been completed.
}

void EvictionController::checkHeapSize() {
  int64_t heap_size = heap_size_;
  if (heap_size <= max_heap_size_) {
    return;
  }

  float percentage =
      static_cast<float>(heap_size - max_heap_size_) / max_heap_size_ +
      heap_size_delta_;

  LOG_FINE(
      "EvictionController::process_delta: evicting %.03f%% of the entries. "
      "Heap size is: %lld / %lld",
      percentage * 100.0f, heap_size, max_heap_size_);

  evict(percentage);
}

void EvictionController::registerRegion(const std::string& name) {
  boost::unique_lock<decltype(regions_mutex_)> lock(regions_mutex_);
  if (regions_.insert(name).second) {
    LOG_FINE("Registered region with Heap LRU eviction controller: name is " +
             name);
  }
}

void EvictionController::unregisterRegion(const std::string& name) {
  boost::unique_lock<decltype(regions_mutex_)> lock(regions_mutex_);
  if (regions_.erase(name) > 0) {
    LOG_FINE("Deregistered region with Heap LRU eviction controller: name is " +
             name);
  }
}

void EvictionController::evict(float percentage) {
  // TODO:  Shouldn't we take the CacheImpl::m_regions
  // lock here? Otherwise we might invoke eviction on a region
  // that has been destroyed or is being destroyed.
  // Its important to not hold this lock for too long
  // because it prevents new regions from getting created or destroyed
  // On the flip side, this requires a copy of the registered region list
  // every time eviction is ordered and that might not be cheap
  //@TODO: Discuss with team

  std::vector<std::string> regions;
  {
    boost::shared_lock<decltype(regions_mutex_)> lock(regions_mutex_);
    regions.reserve(regions_.size());
    regions.insert(regions.end(), regions_.begin(), regions_.end());
  }

  for (const auto& regionName : regions) {
    if (auto region = std::dynamic_pointer_cast<RegionInternal>(
            cache_->getRegion(regionName))) {
      region->evict(percentage);
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
