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
#include "LRUAction.hpp"

#include <geode/Cache.hpp>

#include "CacheImpl.hpp"
#include "LRUEntriesMap.hpp"
#include "LRULocalDestroyAction.hpp"

namespace apache {
namespace geode {
namespace client {

LRUAction* LRUAction::newLRUAction(const LRUAction::Action& actionType,
                                   RegionInternal* regionPtr,
                                   LRUEntriesMap* entriesMapPtr) {
  LRUAction* result = nullptr;

  switch (actionType) {
    case LRUAction::INVALIDATE:
      result = new LRULocalInvalidateAction(regionPtr);
      break;
    case LRUAction::LOCAL_DESTROY:
      result = new LRULocalDestroyAction(regionPtr, entriesMapPtr);
      break;
    case LRUAction::OVERFLOW_TO_DISK:
      result = new LRUOverFlowToDiskAction(regionPtr, entriesMapPtr);
      break;
    case LRUAction::DESTROY:
      result = new LRULocalDestroyAction(regionPtr, entriesMapPtr);
      break;
    case LRUAction::LOCAL_INVALIDATE:
    case LRUAction::INVALID_ACTION:
      /** @TODO throw IllegalArgumentException; */
      break;
  }
  return result;
}

bool LRUOverFlowToDiskAction::evict(
    const std::shared_ptr<MapEntryImpl>& mePtr) {
  if (m_regionPtr->isDestroyed()) {
    LOG_ERROR(
        "[internal error] :: OverflowAction: region is being destroyed, so not "
        "evicting entries");
    return false;
  }
  std::shared_ptr<CacheableKey> keyPtr;
  std::shared_ptr<Cacheable> valuePtr;
  mePtr->getKeyI(keyPtr);
  mePtr->getValueI(valuePtr);
  if (valuePtr == nullptr) {
    LOG_ERROR(
        "[internal error]:: OverflowAction: destroyed entry added to "
        "LRU list");
    throw FatalInternalException(
        "OverflowAction: destroyed entry added to "
        "LRU list");
  }
  auto&& lruProps = mePtr->getLRUProperties();
  auto persistenceInfo = lruProps.persistence_info();
  bool setInfo = false;
  if (persistenceInfo == nullptr) {
    setInfo = true;
  }
  auto pmPtr = m_regionPtr->getPersistenceManager();
  try {
    pmPtr->write(keyPtr, valuePtr, persistenceInfo);
  } catch (DiskFailureException& ex) {
    LOG_ERROR("DiskFailureException - %s", ex.what());
    return false;
  } catch (Exception& ex) {
    LOG_ERROR("write to persistence layer failed - %s", ex.what());
    return false;
  }
  if (setInfo == true) {
    lruProps.persistence_info(persistenceInfo);
  }
  (m_regionPtr->getRegionStats())->incOverflows();
  (m_regionPtr->getCacheImpl())->getCachePerfStats().incOverflows();
  // set value after write on disk to indicate that it is on disk.
  mePtr->setValueI(CacheableToken::overflowed());

  if (m_entriesMapPtr != nullptr) {
    int64_t newSize =
        CacheableToken::overflowed()->objectSize() - valuePtr->objectSize();
    m_entriesMapPtr->updateMapSize(newSize);
  }
  return true;
}

bool LRULocalInvalidateAction::evict(
    const std::shared_ptr<MapEntryImpl>& mePtr) {
  std::shared_ptr<VersionTag> versionTag;
  std::shared_ptr<CacheableKey> keyPtr;
  mePtr->getKeyI(keyPtr);
  //  we should invoke the invalidateNoThrow with appropriate
  // flags to correctly invoke listeners
  LOG_DEBUG("LRULocalInvalidate: evicting entry with key [%s]",
            Utils::nullSafeToString(keyPtr).c_str());
  GfErrType err = GF_NOERR;
  if (!m_regionPtr->isDestroyed()) {
    err = m_regionPtr->invalidateNoThrow(
        keyPtr, nullptr, -1, CacheEventFlags::EVICTION | CacheEventFlags::LOCAL,
        versionTag);
  }
  return (err == GF_NOERR);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
