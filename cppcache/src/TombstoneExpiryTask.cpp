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

#include "TombstoneExpiryTask.hpp"

#include "MapSegment.hpp"
#include "TombstoneEntry.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

TombstoneExpiryTask::TombstoneExpiryTask(
    ExpiryTaskManager& manager, MapSegment& segment,
    std::shared_ptr<TombstoneEntry> tombstone)
    : ExpiryTask(manager),
      segment_(segment),
      tombstone_(std::move(tombstone)) {}

bool TombstoneExpiryTask::on_expire() {
  std::shared_ptr<CacheableKey> key;
  tombstone_->entry()->getKeyI(key);

  LOG_DEBUG(
      "TombstoneExpiryTask::on_expire LOCAL_DESTROY "
      "for region entry with key %s",
      Utils::nullSafeToString(key).c_str());
  segment_.remove_tomb_entry(tombstone_);
  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
