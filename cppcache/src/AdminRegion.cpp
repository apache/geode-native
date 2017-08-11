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
#include "AdminRegion.hpp"
#include "CacheImpl.hpp"
#include <geode/SystemProperties.hpp>
#include "ThinClientRegion.hpp"
#include <statistics/StatisticsManager.hpp>
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

AdminRegionPtr AdminRegion::create(CacheImpl* cache,
                                   ThinClientBaseDM* distMan) {
  auto adminRegion = std::make_shared<AdminRegion>();

  auto& props = cache->getDistributedSystem().getSystemProperties();
  if (props.statisticsEnabled()) {
    // no need to create a region .. just create a cacheDistribution Manager
    adminRegion->m_connectionMgr = &(cache->tcrConnectionManager());
    if (!distMan) {
      adminRegion->m_distMngr =
          new ThinClientCacheDistributionManager(*adminRegion->m_connectionMgr);
      auto mngr = cache->getDistributedSystem().getStatisticsManager();
      if (mngr) {
        // Register it with StatisticsManager
        mngr->RegisterAdminRegion(adminRegion);
      }
    } else {
      adminRegion->m_distMngr = distMan;
    }
  }

  return adminRegion;
}

void AdminRegion::init() {
  /*TryWriteGuard _guard(m_rwLock, m_destroyPending);
  if (m_destroyPending) {
    return;
  }
  */
  // Init distribution manager if it is not a pool
  ThinClientPoolDM* pool = dynamic_cast<ThinClientPoolDM*>(m_distMngr);
  if (pool == nullptr) {
    m_distMngr->init();
  }
}

TcrConnectionManager* AdminRegion::getConnectionManager() {
  return m_connectionMgr;
}

void AdminRegion::put(const CacheableKeyPtr& keyPtr,
                      const CacheablePtr& valuePtr) {
  GfErrType err = putNoThrow(keyPtr, valuePtr);
  GfErrTypeToException("AdminRegion::put", err);
}

GfErrType AdminRegion::putNoThrow(const CacheableKeyPtr& keyPtr,
                                  const CacheablePtr& valuePtr) {
  // put obj to region
  GfErrType err = GF_NOERR;

  TcrMessagePut request(
      m_connectionMgr->getCacheImpl()->getCache()->createDataOutput(), nullptr,
      keyPtr, valuePtr, nullptr, false, m_distMngr, true, false,
      m_fullPath.c_str());
  request.setMetaRegion(true);
  TcrMessageReply reply(true, m_distMngr);
  reply.setMetaRegion(true);
  err = m_distMngr->sendSyncRequest(request, reply, true, true);
  if (err != GF_NOERR) {
    return err;
  }

  // put the object into local region
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      LOGDEBUG(
          "AdminRegion::put: entry is written into remote server "
          "at region %s",
          m_fullPath.c_str());
      break;
    }
    case TcrMessage::EXCEPTION: {
      const char* exceptionMsg = reply.getException();
      err = ThinClientRegion::handleServerException("AdminRegion::put",
                                                    exceptionMsg);
      break;
    }
    case TcrMessage::PUT_DATA_ERROR: {
      LOGERROR("A write error occurred on the endpoint %s",
               m_distMngr->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type in put reply %d", reply.getMessageType());
      err = GF_MSG;
    }
  }
  return err;
}

void AdminRegion::close() {
  TryWriteGuard _guard(m_rwLock, m_destroyPending);
  if (m_destroyPending) {
    return;
  }
  m_destroyPending = true;

  // Close distribution manager if it is not a pool
  ThinClientPoolDM* pool = dynamic_cast<ThinClientPoolDM*>(m_distMngr);
  if (pool == nullptr) {
    m_distMngr->destroy();
    GF_SAFE_DELETE(m_distMngr);
  }
}

AdminRegion::~AdminRegion() {
  // destructor should be single threaded in any case, so no need of guard
  if (m_distMngr != nullptr) {
    close();
  }
}

const bool& AdminRegion::isDestroyed() { return m_destroyPending; }
ACE_RW_Thread_Mutex& AdminRegion::getRWLock() { return m_rwLock; }

}  // namespace client
}  // namespace geode
}  // namespace apache
