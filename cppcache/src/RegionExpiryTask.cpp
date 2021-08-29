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

#include "RegionExpiryTask.hpp"

#include "CacheImpl.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

RegionExpiryTask::RegionExpiryTask(ExpiryTaskManager& manager,
                                   std::shared_ptr<RegionInternal> region,
                                   ExpirationAction action,
                                   const duration_t& duration)
    : ExpiryTask(manager),
      region_(region),
      duration_(duration),
      action_(action) {}

ExpiryTask::time_point_t RegionExpiryTask::expire_at() const {
  auto statistics = region_->getStatistics();
  auto last_time = statistics->getLastAccessedSteadyTime();
  if (region_->getAttributes().getRegionTimeToLive() >
      std::chrono::seconds::zero()) {
    last_time = statistics->getLastModifiedSteadyTime();
  }

  return last_time + duration_;
}

bool RegionExpiryTask::on_expire() {
  auto tp = expire_at();
  if (tp > ExpiryTask::clock_t::now()) {
    // Region expiration needs to be re-scheduled as it was accessed/modified
    // since the last time the expiration task was (re-)scheduled.
    // This is the best approach, rather than re-scheduling the task each time
    // the region is accessed/modified, as access/modify is a more frequent
    // event than expiration.
    reset(tp);
    return false;
  }

  const auto full_path = region_->getFullPath().c_str();
  switch (action_) {
    case ExpirationAction::INVALIDATE: {
      LOGDEBUG("RegionExpiryTask INVALIDATE region [%s]", full_path);
      region_->invalidateRegionNoThrow(nullptr, CacheEventFlags::EXPIRATION);
      break;
    }
    case ExpirationAction::LOCAL_INVALIDATE: {
      LOGDEBUG("RegionExpiryTask LOCAL_INVALIDATE region [%s]", full_path);
      region_->invalidateRegionNoThrow(
          nullptr, CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL);
      break;
    }
    case ExpirationAction::DESTROY: {
      LOGDEBUG("RegionExpiryTask DESTROY region [%s]", full_path);
      region_->destroyRegionNoThrow(nullptr, true, CacheEventFlags::EXPIRATION);
      break;
    }
    case ExpirationAction::LOCAL_DESTROY: {
      LOGDEBUG("RegionExpiryTask LOCAL_DESTROY region [%s]", full_path);
      region_->destroyRegionNoThrow(
          nullptr, true, CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL);
      break;
    }
    case ExpirationAction::INVALID_ACTION: {
      LOGERROR("Unknown expiration action %d for region [%s]",
               static_cast<int32_t>(action_), full_path);
      break;
    }
  }

  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
