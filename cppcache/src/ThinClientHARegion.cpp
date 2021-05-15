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

#include "ThinClientHARegion.hpp"

#include <geode/PoolManager.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "ReadWriteLock.hpp"
#include "TcrHADistributionManager.hpp"
#include "ThinClientPoolHADM.hpp"
namespace apache {
namespace geode {
namespace client {

ThinClientHARegion::ThinClientHARegion(
    const std::string& name, CacheImpl* cache,
    const std::shared_ptr<RegionInternal>& rPtr, RegionAttributes attributes,
    const std::shared_ptr<CacheStatistics>& stats, bool shared,
    bool enableNotification)
    : ThinClientRegion(name, cache, rPtr, attributes, stats, shared),
      m_attributes(attributes),
      m_processedMarker(false) {
  setClientNotificationEnabled(enableNotification);
}

void ThinClientHARegion::initTCR() {
  try {
    m_tcrdm = std::dynamic_pointer_cast<ThinClientBaseDM>(
        m_cacheImpl->getCache()->getPoolManager().find(
            m_attributes.getPoolName()));
    if (m_tcrdm) {
      // Pool DM should only be inited once and it
      // is already done in PoolFactory::create();
      // m_tcrdm->init();
      auto poolDM = std::dynamic_pointer_cast<ThinClientPoolHADM>(m_tcrdm);
      poolDM->addRegion(this);
      poolDM->incRegionCount();
    } else {
      throw IllegalStateException("pool not found");
    }
  } catch (const Exception& ex) {
    LOG_ERROR(
        "ThinClientHARegion: failed to create a DistributionManager "
        "object due to: %s: %s",
        ex.getName().c_str(), ex.what());
    throw;
  }
}

void ThinClientHARegion::acquireGlobals(bool isFailover) {
  if (isFailover) {
    ThinClientRegion::acquireGlobals(isFailover);
  } else {
    m_tcrdm->acquireRedundancyLock();
  }
}

void ThinClientHARegion::releaseGlobals(bool isFailover) {
  if (isFailover) {
    ThinClientRegion::releaseGlobals(isFailover);
  } else {
    m_tcrdm->releaseRedundancyLock();
  }
}

void ThinClientHARegion::handleMarker() {
  TryReadGuard guard(m_rwLock, m_destroyPending);
  if (m_destroyPending) {
    return;
  }

  if (m_listener != nullptr && !m_processedMarker) {
    RegionEvent event(shared_from_this(), nullptr, false);
    int64_t sampleStartNanos = startStatOpTime();
    try {
      m_listener->afterRegionLive(event);
    } catch (const Exception& ex) {
      LOG_ERROR("Exception in CacheListener::afterRegionLive: %s: %s",
                ex.getName().c_str(), ex.what());
    } catch (...) {
      LOG_ERROR("Unknown exception in CacheListener::afterRegionLive");
    }
    m_cacheImpl->getCachePerfStats().incListenerCalls();
    updateStatOpTime(m_regionStats->getStat(),
                     m_regionStats->getListenerCallTimeId(), sampleStartNanos);
    m_regionStats->incListenerCallsCompleted();
  }
  m_processedMarker = true;
}

bool ThinClientHARegion::getProcessedMarker() {
  return m_processedMarker || !isDurableClient();
}

void ThinClientHARegion::destroyDM(bool) {
  LOG_DEBUG(
      "ThinClientHARegion::destroyDM( ): removing region from "
      "ThinClientPoolHADM list.");
  auto poolDM = std::dynamic_pointer_cast<ThinClientPoolHADM>(m_tcrdm);
  poolDM->removeRegion(this);
  poolDM->decRegionCount();
}

void ThinClientHARegion::addDisconnectedMessageToQueue() {
  auto poolDM = std::dynamic_pointer_cast<ThinClientPoolHADM>(m_tcrdm);
  poolDM->addDisconnectedMessageToQueue(this);

  if (poolDM->redundancyManager_->m_globalProcessedMarker &&
      !m_processedMarker) {
    receiveNotification(TcrMessageClientMarker(
        new DataOutput(m_cacheImpl->createDataOutput()), true));
  }
}

GfErrType ThinClientHARegion::getNoThrow_FullObject(
    std::shared_ptr<EventId> eventId, std::shared_ptr<Cacheable>& fullObject,
    std::shared_ptr<VersionTag>& versionTag) {
  TcrMessageRequestEventValue fullObjectMsg(
      new DataOutput(m_cacheImpl->createDataOutput()), eventId);
  TcrMessageReply reply(true, nullptr);

  auto poolHADM = std::dynamic_pointer_cast<ThinClientPoolHADM>(m_tcrdm);
  GfErrType err = GF_NOTCON;
  if (poolHADM) {
    err = poolHADM->sendRequestToPrimary(fullObjectMsg, reply);
  } else {
    err = std::static_pointer_cast<TcrHADistributionManager>(m_tcrdm)
              ->sendRequestToPrimary(fullObjectMsg, reply);
  }
  if (err == GF_NOERR) {
    fullObject = reply.getValue();
  }
  versionTag = reply.getVersionTag();
  return err;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
