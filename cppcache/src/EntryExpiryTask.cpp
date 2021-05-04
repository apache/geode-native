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

#include "EntryExpiryTask.hpp"

#include "CacheImpl.hpp"
#include "RegionInternal.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

EntryExpiryTask::EntryExpiryTask(ExpiryTaskManager& manager,
                                 std::shared_ptr<RegionInternal> region,
                                 std::shared_ptr<MapEntryImpl> entry,
                                 ExpirationAction action,
                                 const duration_t& duration)
    : ExpiryTask(manager),
      duration_(duration),
      action_(action),
      entry_(entry),
      region_(region) {}

EntryExpiryTask::time_point_t EntryExpiryTask::expire_at() const {
  auto& properties = entry_->getExpProperties();
  auto last_time = properties.last_accessed();
  if (region_->getAttributes().getEntryTimeToLive() >
      std::chrono::seconds::zero()) {
    last_time = properties.last_modified();
  }

  return last_time + duration_;
}

bool EntryExpiryTask::on_expire() {
  auto tp = expire_at();
  if (tp > ExpiryTask::clock_t::now()) {
    // Entry expiration needs to be re-scheduled as it was accessed/modified
    // since the last time the expiration task was (re-)scheduled.
    // This is the best approach, rather than re-scheduling the task each time
    // the entry is accessed/modified, as access/modify is a more frequent
    // event than expiration.
    reset(tp);
    return false;
  }

  // Pass a blank version tag.
  std::shared_ptr<CacheableKey> key;
  std::shared_ptr<VersionTag> versionTag;

  entry_->getKeyI(key);

  const auto& full_path = region_->getFullPath();
  auto key_str = Utils::nullSafeToString(key);

  switch (action_) {
    case ExpirationAction::INVALIDATE: {
      LOG_DEBUG(
          "EntryExpiryTask::DoTheExpirationAction INVALIDATE "
          "for region " +
          full_path + " entry with key " + key_str);
      region_->invalidateNoThrow(key, nullptr, -1, CacheEventFlags::EXPIRATION,
                                 versionTag);
      break;
    }
    case ExpirationAction::LOCAL_INVALIDATE: {
      LOG_DEBUG(
          "EntryExpiryTask::DoTheExpirationAction LOCAL_INVALIDATE "
          "for region " +
          full_path + " entry with key " + key_str);
      region_->invalidateNoThrow(
          key, nullptr, -1,
          CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL, versionTag);
      break;
    }
    case ExpirationAction::DESTROY: {
      LOG_DEBUG(
          "EntryExpiryTask::DoTheExpirationAction DESTROY "
          "for region " +
          full_path + " entry with key " + key_str);
      region_->destroyNoThrow(key, nullptr, -1, CacheEventFlags::EXPIRATION,
                              versionTag);
      break;
    }
    case ExpirationAction::LOCAL_DESTROY: {
      LOG_DEBUG(
          "EntryExpiryTask::DoTheExpirationAction LOCAL_DESTROY "
          "for region " +
          full_path + " entry with key " + key_str);
      region_->destroyNoThrow(
          key, nullptr, -1,
          CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL, versionTag);
      break;
    }
    case ExpirationAction::INVALID_ACTION: {
      LOG_ERROR(
          "Unknown expiration action "
          "%d for region %s for key %s",
          static_cast<int32_t>(action_), full_path.c_str(), key_str.c_str());
      break;
    }
  }

  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
