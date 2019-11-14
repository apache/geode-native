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

#include "RegionExpiryHandler.hpp"

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

RegionExpiryHandler::RegionExpiryHandler(std::shared_ptr<RegionInternal>& rptr,
                                         ExpirationAction action,
                                         std::chrono::seconds duration)
    : m_regionPtr(rptr),
      m_action(action),
      m_duration(duration),
      /* adongre
       * CID 28941: Uninitialized scalar field (UNINIT_CTOR)
       */
      m_expiryTaskId(0) {}

int RegionExpiryHandler::handle_timeout(const ACE_Time_Value& current_time,
                                        const void*) {
  auto curr_time = std::chrono::system_clock::from_time_t(current_time.sec());
  try {
    auto statistics = m_regionPtr->getStatistics();
    auto lastTimeForExp = statistics->getLastAccessedTime();
    if (m_regionPtr->getAttributes().getRegionTimeToLive() >
        std::chrono::seconds::zero()) {
      lastTimeForExp = statistics->getLastModifiedTime();
    }

    auto elapsed = curr_time - lastTimeForExp;
    LOGDEBUG("Entered region expiry task handler for region [%s]: %s,%s,%s,%s",
             m_regionPtr->getFullPath().c_str(),
             to_string(curr_time.time_since_epoch()).c_str(),
             to_string(lastTimeForExp.time_since_epoch()).c_str(),
             to_string(m_duration).c_str(), to_string(elapsed).c_str());
    if (elapsed >= m_duration) {
      DoTheExpirationAction();
    } else {
      auto remaining = m_duration - elapsed;
      // reset the task after
      // (lastAccessTime + entryExpiryDuration - curr_time) in seconds
      LOGDEBUG("Resetting expiry task for region [%s] after %s sec",
               m_regionPtr->getFullPath().c_str(),
               to_string(remaining).c_str());
      m_regionPtr->getCacheImpl()->getExpiryTaskManager().resetTask(
          m_expiryTaskId, remaining);
      return 0;
    }
    LOGDEBUG("Removing expiry task for region [%s]",
             m_regionPtr->getFullPath().c_str());
    m_regionPtr->getCacheImpl()->getExpiryTaskManager().resetTask(
        m_expiryTaskId, std::chrono::seconds::zero());
  } catch (...) {
    // Ignore whatever exception comes
  }
  //  we now delete the handler in GF_Timer_Heap_ImmediateReset_T
  // and always return success.
  return 0;
}

int RegionExpiryHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask) {
  //  we now delete the handler in GF_Timer_Heap_ImmediateReset_T
  // delete this;
  return 0;
}

void RegionExpiryHandler::DoTheExpirationAction() {
  switch (m_action) {
    case ExpirationAction::INVALIDATE: {
      LOGDEBUG(
          "RegionExpiryHandler::DoTheExpirationAction INVALIDATE "
          "region [%s]",
          m_regionPtr->getFullPath().c_str());
      m_regionPtr->invalidateRegionNoThrow(nullptr,
                                           CacheEventFlags::EXPIRATION);
      break;
    }
    case ExpirationAction::LOCAL_INVALIDATE: {
      LOGDEBUG(
          "RegionExpiryHandler::DoTheExpirationAction LOCAL_INVALIDATE "
          "region [%s]",
          m_regionPtr->getFullPath().c_str());
      m_regionPtr->invalidateRegionNoThrow(
          nullptr, CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL);
      break;
    }
    case ExpirationAction::DESTROY: {
      LOGDEBUG(
          "RegionExpiryHandler::DoTheExpirationAction DESTROY "
          "region [%s]",
          m_regionPtr->getFullPath().c_str());
      m_regionPtr->destroyRegionNoThrow(nullptr, true,
                                        CacheEventFlags::EXPIRATION);
      break;
    }
    case ExpirationAction::LOCAL_DESTROY: {
      LOGDEBUG(
          "RegionExpiryHandler::DoTheExpirationAction LOCAL_DESTROY "
          "region [%s]",
          m_regionPtr->getFullPath().c_str());
      m_regionPtr->destroyRegionNoThrow(
          nullptr, true, CacheEventFlags::EXPIRATION | CacheEventFlags::LOCAL);
      break;
    }
    default: {
      LOGERROR(
          "Unknown expiration action "
          "%d for region [%s]",
          m_action, m_regionPtr->getFullPath().c_str());
      break;
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
