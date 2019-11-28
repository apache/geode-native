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

#include "TombstoneExpiryHandler.hpp"

#include <ace/Reactor.h>
#include <ace/Timer_Heap.h>
#include <ace/Timer_Heap_T.h>
#include <ace/Timer_Queue.h>
#include <ace/Timer_Queue_Adapters.h>
#include <ace/svc_export.h>

#include "CacheImpl.hpp"
#include "ExpiryTaskManager.hpp"
#include "MapEntry.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

TombstoneExpiryHandler::TombstoneExpiryHandler(
    std::shared_ptr<TombstoneEntry> entryPtr, TombstoneList* tombstoneList,
    std::chrono::milliseconds duration, CacheImpl* cacheImpl)
    : m_entryPtr(entryPtr),
      m_duration(duration),
      m_cacheImpl(cacheImpl),
      m_tombstoneList(tombstoneList) {}

int TombstoneExpiryHandler::handle_timeout(const ACE_Time_Value&, const void*) {
  std::shared_ptr<CacheableKey> key;
  m_entryPtr->getEntry()->getKeyI(key);
  auto creationTime = m_entryPtr->getTombstoneCreationTime();
  auto curr_time = TombstoneEntry::clock::now();
  auto expiryTaskId = m_entryPtr->getExpiryTaskId();
  auto sec = curr_time - creationTime - m_duration;
  try {
    using apache::geode::internal::chrono::duration::to_string;
    LOGDEBUG(
        "Entered entry expiry task handler for tombstone of key [%s]: "
        "%s,%s,%s,%s",
        Utils::nullSafeToString(key).c_str(),
        to_string(curr_time.time_since_epoch()).c_str(),
        to_string(creationTime.time_since_epoch()).c_str(),
        to_string(m_duration).c_str(), to_string(sec).c_str());
    if (sec >= std::chrono::seconds::zero()) {
      DoTheExpirationAction(key);
    } else {
      // reset the task after
      // (lastAccessTime + entryExpiryDuration - curr_time) in seconds
      LOGDEBUG(
          "Resetting expiry task %s later for key "
          "[%s]",
          to_string(-sec).c_str(), Utils::nullSafeToString(key).c_str());
      m_cacheImpl->getExpiryTaskManager().resetTask(
          m_entryPtr->getExpiryTaskId(), -sec);
      return 0;
    }
  } catch (...) {
    // Ignore whatever exception comes
  }
  LOGDEBUG("Removing expiry task for key [%s]",
           Utils::nullSafeToString(key).c_str());
  // we now delete the handler in GF_Timer_Heap_ImmediateReset_T
  // and always return success.
  m_cacheImpl->getExpiryTaskManager().resetTask(expiryTaskId, 0);
  return 0;
}

int TombstoneExpiryHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask) {
  return 0;
}

inline void TombstoneExpiryHandler::DoTheExpirationAction(
    const std::shared_ptr<CacheableKey>& key) {
  LOGDEBUG(
      "EntryExpiryHandler::DoTheExpirationAction LOCAL_DESTROY "
      "for region entry with key %s",
      Utils::nullSafeToString(key).c_str());
  m_tombstoneList->removeEntryFromMapSegment(key);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
