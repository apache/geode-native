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

#include "EntryExpiryHandler.hpp"

#include <ace/Reactor.h>
#include <ace/Timer_Heap.h>
#include <ace/Timer_Heap_T.h>
#include <ace/Timer_Queue.h>
#include <ace/Timer_Queue_Adapters.h>
#include <ace/svc_export.h>

#include "CacheImpl.hpp"
#include "ExpiryTaskManager.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

EntryExpiryHandler::EntryExpiryHandler(std::shared_ptr<RegionInternal>& rptr,
                                       std::shared_ptr<MapEntryImpl>& entryPtr,
                                       ExpirationAction action,
                                       std::chrono::seconds duration)
    : m_regionPtr(rptr),
      m_entryPtr(entryPtr),
      m_action(action),
      m_duration(duration) {}

int EntryExpiryHandler::handle_timeout(const ACE_Time_Value& current_time,
                                       const void*) {
  std::shared_ptr<CacheableKey> key;
  m_entryPtr->getKeyI(key);
  ExpEntryProperties& expProps = m_entryPtr->getExpProperties();
  try {
    auto curr_time = std::chrono::system_clock::from_time_t(current_time.sec());

    auto lastTimeForExp = expProps.getLastAccessTime();
    if (m_regionPtr->getAttributes().getEntryTimeToLive() >
        std::chrono::seconds::zero()) {
      lastTimeForExp = expProps.getLastModifiedTime();
    }

    auto elapsed = curr_time - lastTimeForExp;

    LOGDEBUG("Entered entry expiry task handler for key [%s] of region [%s]",
             Utils::nullSafeToString(key).c_str(),
             m_regionPtr->getFullPath().c_str());
    if (elapsed >= m_duration) {
      DoTheExpirationAction(key);
    } else {
      // reset the task after
      // (lastAccessTime + entryExpiryDuration - curr_time) in seconds
      auto remaining = m_duration - elapsed;
      LOGDEBUG("Resetting expiry task for key [%s] of region [%s]",
               Utils::nullSafeToString(key).c_str(),
               m_regionPtr->getFullPath().c_str());
      m_regionPtr->getCacheImpl()->getExpiryTaskManager().resetTask(
          expProps.getExpiryTaskId(), remaining);
      return 0;
    }
  } catch (...) {
    // Ignore whatever exception comes
  }
  LOGDEBUG("Removing expiry task for key [%s] of region [%s]",
           Utils::nullSafeToString(key).c_str(),
           m_regionPtr->getFullPath().c_str());
  m_regionPtr->getCacheImpl()->getExpiryTaskManager().resetTask(
      expProps.getExpiryTaskId(), std::chrono::seconds::zero());
  //  we now delete the handler in GF_Timer_Heap_ImmediateReset_T
  // and always return success.

  // set the invalid taskid as we have removed the expiry task
  expProps.setExpiryTaskId(-1);
  return 0;
}

int EntryExpiryHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask) {
  //  we now delete the handler in GF_Timer_Heap_ImmediateReset_T
  return 0;
}

inline void EntryExpiryHandler::DoTheExpirationAction(
    const std::shared_ptr<CacheableKey>& key) {
  // Pass a blank version tag.
  std::shared_ptr<VersionTag> versionTag;
  switch (m_action) {
    case ExpirationAction::INVALIDATE: {
      LOGDEBUG(
          "EntryExpiryHandler::DoTheExpirationAction INVALIDATE "
          "for region %s entry with key %s",
          m_regionPtr->getFullPath().c_str(),
          Utils::nullSafeToString(key).c_str());
      m_regionPtr->invalidateNoThrow(key, nullptr, -1,
                                     CacheEventFlags::EXPIRATION, versionTag);
      break;
    }
    case ExpirationAction::LOCAL_INVALIDATE: {
      LOGDEBUG(
          "EntryExpiryHandler::DoTheExpirationAction LOCAL_INVALIDATE "
          "for region %s entry with key %s",
          m_regionPtr->getFullPath().c_str(),
          Utils::nullSafeToString(key).c_str());
      m_regionPtr->invalidateNoThrow(
          key, nullptr, -1,
          CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL, versionTag);
      break;
    }
    case ExpirationAction::DESTROY: {
      LOGDEBUG(
          "EntryExpiryHandler::DoTheExpirationAction DESTROY "
          "for region %s entry with key %s",
          m_regionPtr->getFullPath().c_str(),
          Utils::nullSafeToString(key).c_str());
      m_regionPtr->destroyNoThrow(key, nullptr, -1, CacheEventFlags::EXPIRATION,
                                  versionTag);
      break;
    }
    case ExpirationAction::LOCAL_DESTROY: {
      LOGDEBUG(
          "EntryExpiryHandler::DoTheExpirationAction LOCAL_DESTROY "
          "for region %s entry with key %s",
          m_regionPtr->getFullPath().c_str(),
          Utils::nullSafeToString(key).c_str());
      m_regionPtr->destroyNoThrow(
          key, nullptr, -1,
          CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL, versionTag);
      break;
    }
    default: {
      LOGERROR(
          "Unknown expiration action "
          "%d for region %s for key %s",
          m_action, m_regionPtr->getFullPath().c_str(),
          Utils::nullSafeToString(key).c_str());
      break;
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
