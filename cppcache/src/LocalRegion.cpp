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

#include "LocalRegion.hpp"

#include <regex>
#include <vector>

#include <geode/PoolManager.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/internal/DataSerializablePrimitive.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheableToken.hpp"
#include "EntriesMapFactory.hpp"
#include "EntryExpiryTask.hpp"
#include "ExpiryTaskManager.hpp"
#include "InterestResultPolicy.hpp"
#include "LRUEntriesMap.hpp"
#include "RegionExpiryTask.hpp"
#include "RegionGlobalLocks.hpp"
#include "SerializableHelper.hpp"
#include "TXState.hpp"
#include "TcrConnectionManager.hpp"
#include "Utils.hpp"
#include "VersionTag.hpp"
#include "VersionedCacheableObjectPartList.hpp"
#include "util/Log.hpp"
#include "util/bounds.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializablePrimitive;

LocalRegion::LocalRegion(const std::string& name, CacheImpl* cacheImpl,
                         const std::shared_ptr<RegionInternal>& rPtr,
                         RegionAttributes attributes,
                         const std::shared_ptr<CacheStatistics>& stats,
                         bool enableTimeStatistics)
    : RegionInternal(cacheImpl, attributes),
      m_name(name),
      m_parentRegion(rPtr),
      m_destroyPending(false),
      expiry_task_id_(ExpiryTask::invalid()),
      m_listener(nullptr),
      m_writer(nullptr),
      m_loader(nullptr),
      m_released(false),
      m_entries(nullptr),
      m_cacheStatistics(stats),
      m_transactionEnabled(false),
      m_isPRSingleHopEnabled(false),
      m_attachedPool(nullptr),
      m_enableTimeStatistics(enableTimeStatistics),
      m_persistenceManager(nullptr) {
  if (m_parentRegion != nullptr) {
    ((m_fullPath = m_parentRegion->getFullPath()) += "/") += m_name;
  } else {
    (m_fullPath = "/") += m_name;
  }
  // create entries map based on RegionAttributes...
  if (attributes.getCachingEnabled()) {
    m_entries = EntriesMapFactory::createMap(this, m_regionAttributes);
  }

  // Initialize callbacks
  std::shared_ptr<CacheListener> clptr;
  std::shared_ptr<CacheWriter> cwptr;
  clptr = m_regionAttributes.getCacheListener();
  m_listener = clptr;
  cwptr = m_regionAttributes.getCacheWriter();
  m_writer = cwptr;
  std::shared_ptr<CacheLoader> cldptr;
  cldptr = m_regionAttributes.getCacheLoader();
  m_loader = cldptr;

  if (m_parentRegion != nullptr) {
    ((m_fullPath = m_parentRegion->getFullPath()) += "/") += m_name;
  } else {
    (m_fullPath = "/") += m_name;
  }

  m_regionStats = new RegionStats(
      cacheImpl->getStatisticsManager().getStatisticsFactory(), m_fullPath);
  auto p = cacheImpl->getPoolManager().find(m_regionAttributes.getPoolName());
  setPool(p);
}

const std::string& LocalRegion::getName() const { return m_name; }

const std::string& LocalRegion::getFullPath() const { return m_fullPath; }

std::shared_ptr<Region> LocalRegion::getParentRegion() const {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::getParentRegion);
  return m_parentRegion;
}

void LocalRegion::updateAccessAndModifiedTime(bool modified) {
  // locking not required since setters use atomic operations
  if (!regionExpiryEnabled()) {
    return;
  }

  auto now = std::chrono::steady_clock::now();
  auto timeStr = to_string(now.time_since_epoch());
  LOG_DEBUG("Setting last accessed time for region %s to %s",
            getFullPath().c_str(), timeStr.c_str());
  m_cacheStatistics->setLastAccessedTime(now);
  if (modified) {
    LOG_DEBUG("Setting last modified time for region %s to %s",
              getFullPath().c_str(), timeStr.c_str());
    m_cacheStatistics->setLastModifiedTime(now);
  }
  // TODO:  should we really touch the parent region??
  RegionInternal* ri = dynamic_cast<RegionInternal*>(m_parentRegion.get());
  if (ri != nullptr) {
    ri->updateAccessAndModifiedTime(modified);
  }
}

std::shared_ptr<CacheStatistics> LocalRegion::getStatistics() const {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::getStatistics);

  if (!m_cacheImpl->getDistributedSystem()
           .getSystemProperties()
           .statisticsEnabled()) {
    throw StatisticsDisabledException(
        "LocalRegion::getStatistics statistics disabled for this region");
  }

  return m_cacheStatistics;
}

void LocalRegion::invalidateRegion(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err =
      invalidateRegionNoThrow(aCallbackArgument, CacheEventFlags::NORMAL);
  throwExceptionIfError("Region::invalidateRegion", err);
}

void LocalRegion::localInvalidateRegion(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err =
      invalidateRegionNoThrow(aCallbackArgument, CacheEventFlags::LOCAL);
  throwExceptionIfError("Region::localInvalidateRegion", err);
}

void LocalRegion::destroyRegion(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err =
      destroyRegionNoThrow(aCallbackArgument, true, CacheEventFlags::NORMAL);
  throwExceptionIfError("Region::destroyRegion", err);
}

void LocalRegion::localDestroyRegion(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err =
      destroyRegionNoThrow(aCallbackArgument, true, CacheEventFlags::LOCAL);
  throwExceptionIfError("Region::localDestroyRegion", err);
}

void LocalRegion::tombstoneOperationNoThrow(
    const std::shared_ptr<CacheableHashMap>& tombstoneVersions,
    const std::shared_ptr<CacheableHashSet>& tombstoneKeys) {
  bool cachingEnabled = m_regionAttributes.getCachingEnabled();

  if (!cachingEnabled) return;

  if (tombstoneVersions) {
    std::map<uint16_t, int64_t> gcVersions;
    for (const auto& itr : *tombstoneVersions) {
      if (auto member =
              std::dynamic_pointer_cast<DSMemberForVersionStamp>(itr.first)) {
        uint16_t memberId =
            getCacheImpl()->getMemberListForVersionStamp()->add(member);
        int64_t version =
            (std::dynamic_pointer_cast<CacheableInt64>(itr.second))->value();
        gcVersions[memberId] = version;
      } else {
        LOG_ERROR(
            "tombstone_operation contains incorrect gc versions in the "
            "message. Region " +
            getFullPath());
        continue;
      }
    }
    m_entries->reapTombstones(gcVersions);
  } else {
    m_entries->reapTombstones(tombstoneKeys);
  }
}

std::shared_ptr<Region> LocalRegion::findSubRegion(const std::string& name) {
  auto&& lock = m_subRegions.make_lock<std::lock_guard>();
  const auto& find = m_subRegions.find(name);
  if (find != m_subRegions.end()) {
    return find->second;
  }
  return nullptr;
}

std::shared_ptr<Region> LocalRegion::getSubregion(const std::string& path) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::getSubregion);

  static const std::string slash("/");
  if (path == slash || path.empty()) {
    LOG_ERROR("Get subregion path [" + path + "] is not valid.");
    throw IllegalArgumentException("Get subegion path is empty or a /");
  }

  auto fullname = path;
  if (fullname.substr(0, 1) == slash) {
    fullname = path.substr(1);
  }

  // find second separator
  auto idx = fullname.find('/');
  auto stepname = fullname.substr(0, idx);

  auto region = findSubRegion(stepname);
  if (region) {
    if (stepname == fullname) {
      // done...
      return region;
    } else {
      std::string remainder = fullname.substr(stepname.length() + 1);
      return region->getSubregion(remainder);
    }
  }

  return nullptr;
}

std::shared_ptr<Region> LocalRegion::createSubregion(
    const std::string& subregionName, RegionAttributes regionAttributes) {
  CHECK_DESTROY_PENDING(TryWriteGuard, LocalRegion::createSubregion);
  {
    std::string namestr = subregionName;
    if (namestr.find('/') != std::string::npos) {
      throw IllegalArgumentException(
          "Malformed name string, contains region path seperator '/'");
    }
  }

  auto&& lock = m_subRegions.make_lock();
  std::shared_ptr<Region> region_ptr;
  if (m_subRegions.find(subregionName) != m_subRegions.end()) {
    throw RegionExistsException(
        "LocalRegion::createSubregion: named region exists in the region");
  }

  auto csptr = std::make_shared<CacheStatistics>();
  auto rPtr = m_cacheImpl->createRegion_internal(
      subregionName,
      std::static_pointer_cast<RegionInternal>(shared_from_this()),
      regionAttributes, csptr, false);
  region_ptr = rPtr;
  if (!rPtr) {
    throw OutOfMemoryException("createSubregion: failed to create region");
  }

  // Instantiate a PersistenceManager object if DiskPolicy is overflow
  if (regionAttributes.getDiskPolicy() == DiskPolicyType::OVERFLOWS) {
    auto pmPtr = regionAttributes.getPersistenceManager();
    if (pmPtr == nullptr) {
      throw NullPointerException(
          "PersistenceManager could not be instantiated");
    }
    auto props = regionAttributes.getPersistenceProperties();
    pmPtr->init(std::shared_ptr<Region>(rPtr), props);
    rPtr->setPersistenceManager(pmPtr);
  }

  rPtr->acquireReadLock();
  m_subRegions.emplace(rPtr->getName(), rPtr);

  // schedule the sub region expiry if regionExpiry enabled.
  rPtr->setRegionExpiryTask();
  rPtr->releaseReadLock();
  return region_ptr;
}

std::vector<std::shared_ptr<Region>> LocalRegion::subregions(
    const bool recursive) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::subregions);
  if (m_subRegions.empty()) {
    return std::vector<std::shared_ptr<Region>>();
  }

  return subregions_internal(recursive);
}
std::shared_ptr<RegionEntry> LocalRegion::getEntry(
    const std::shared_ptr<CacheableKey>& key) {
  if (getTXState() != nullptr) {
    GfErrTypeThrowException("GetEntry is not supported in transaction",
                            GF_NOTSUP);
  }
  std::shared_ptr<RegionEntry> rptr;
  std::shared_ptr<Cacheable> valuePtr;
  getEntry(key, valuePtr);
  if (valuePtr != nullptr) {
    rptr = createRegionEntry(key, valuePtr);
  }
  return rptr;
}

void LocalRegion::getEntry(const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<Cacheable>& valuePtr) {
  if (key == nullptr) {
    throw IllegalArgumentException("LocalRegion::getEntry: null key");
  }

  std::shared_ptr<MapEntryImpl> mePtr;
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::getEntry);
  if (m_regionAttributes.getCachingEnabled()) {
    m_entries->getEntry(key, mePtr, valuePtr);
  }
}
std::shared_ptr<Cacheable> LocalRegion::get(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<Cacheable> rptr;
  int64_t sampleStartNanos = startStatOpTime();
  GfErrType err = getNoThrow(key, rptr, aCallbackArgument);
  updateStatOpTime(m_regionStats->getStat(), m_regionStats->getGetTimeId(),
                   sampleStartNanos);

  // rptr = handleReplay(err, rptr);

  throwExceptionIfError("Region::get", err);

  return rptr;
}

void LocalRegion::put(const std::shared_ptr<CacheableKey>& key,
                      const std::shared_ptr<Cacheable>& value,
                      const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<Cacheable> oldValue;
  int64_t sampleStartNanos = startStatOpTime();
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = putNoThrow(key, value, aCallbackArgument, oldValue, -1,
                             CacheEventFlags::NORMAL, versionTag);
  updateStatOpTime(m_regionStats->getStat(), m_regionStats->getPutTimeId(),
                   sampleStartNanos);
  //  handleReplay(err, nullptr);
  throwExceptionIfError("Region::put", err);
}

void LocalRegion::localPut(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<Cacheable> oldValue;
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = putNoThrow(key, value, aCallbackArgument, oldValue, -1,
                             CacheEventFlags::LOCAL, versionTag);
  throwExceptionIfError("Region::localPut", err);
}

void LocalRegion::putAll(
    const HashMapOfCacheable& map, std::chrono::milliseconds timeout,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  util::PROTOCOL_OPERATION_TIMEOUT_BOUNDS(timeout);

  auto sampleStartNanos = startStatOpTime();
  auto err = putAllNoThrow(map, timeout, aCallbackArgument);
  updateStatOpTime(m_regionStats->getStat(), m_regionStats->getPutAllTimeId(),
                   sampleStartNanos);
  // handleReplay(err, nullptr);
  throwExceptionIfError("Region::putAll", err);
}

void LocalRegion::removeAll(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  if (keys.size() == 0) {
    throw IllegalArgumentException("Region::removeAll: zero keys provided");
  }
  int64_t sampleStartNanos = startStatOpTime();
  GfErrType err = removeAllNoThrow(keys, aCallbackArgument);
  updateStatOpTime(m_regionStats->getStat(),
                   m_regionStats->getRemoveAllTimeId(), sampleStartNanos);
  throwExceptionIfError("Region::removeAll", err);
}

void LocalRegion::create(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = createNoThrow(key, value, aCallbackArgument, -1,
                                CacheEventFlags::NORMAL, versionTag);
  // handleReplay(err, nullptr);
  throwExceptionIfError("Region::create", err);
}

void LocalRegion::localCreate(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = createNoThrow(key, value, aCallbackArgument, -1,
                                CacheEventFlags::LOCAL, versionTag);
  throwExceptionIfError("Region::localCreate", err);
}

void LocalRegion::invalidate(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = invalidateNoThrow(key, aCallbackArgument, -1,
                                    CacheEventFlags::NORMAL, versionTag);
  //  handleReplay(err, nullptr);
  throwExceptionIfError("Region::invalidate", err);
}

void LocalRegion::localInvalidate(
    const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = invalidateNoThrow(keyPtr, aCallbackArgument, -1,
                                    CacheEventFlags::LOCAL, versionTag);
  throwExceptionIfError("Region::localInvalidate", err);
}

void LocalRegion::destroy(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;

  GfErrType err = destroyNoThrow(key, aCallbackArgument, -1,
                                 CacheEventFlags::NORMAL, versionTag);
  // handleReplay(err, nullptr);
  throwExceptionIfError("Region::destroy", err);
}

GfErrType LocalRegion::localDestroyNoCallbacks(
    const std::shared_ptr<CacheableKey>& key) {
  return destroyNoThrow(key, nullptr, -1,
                        CacheEventFlags::LOCAL | CacheEventFlags::NOCALLBACKS,
                        nullptr);
}

void LocalRegion::localDestroy(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = destroyNoThrow(key, aCallbackArgument, -1,
                                 CacheEventFlags::LOCAL, versionTag);
  throwExceptionIfError("Region::localDestroy", err);
}

bool LocalRegion::remove(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = removeNoThrow(key, value, aCallbackArgument, -1,
                                CacheEventFlags::NORMAL, versionTag);

  bool result = false;

  if (err == GF_NOERR) {
    result = true;
  } else if (err != GF_ENOENT && err != GF_CACHE_ENTRY_NOT_FOUND) {
    throwExceptionIfError("Region::remove", err);
  }

  return result;
}

bool LocalRegion::removeEx(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = removeNoThrowEx(key, aCallbackArgument, -1,
                                  CacheEventFlags::NORMAL, versionTag);
  bool result = false;

  if (err == GF_NOERR) {
    result = true;
  } else if (err != GF_ENOENT && err != GF_CACHE_ENTRY_NOT_FOUND) {
    throwExceptionIfError("Region::removeEx", err);
  }

  return result;
}

bool LocalRegion::localRemove(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = removeNoThrow(key, value, aCallbackArgument, -1,
                                CacheEventFlags::LOCAL, versionTag);

  bool result = false;

  if (err == GF_NOERR) {
    result = true;
  } else if (err != GF_ENOENT && err != GF_CACHE_ENTRY_NOT_FOUND) {
    throwExceptionIfError("Region::localRemove", err);
  }

  return result;
}

bool LocalRegion::localRemoveEx(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  std::shared_ptr<VersionTag> versionTag;
  GfErrType err = removeNoThrowEx(key, aCallbackArgument, -1,
                                  CacheEventFlags::LOCAL, versionTag);

  bool result = false;

  if (err == GF_NOERR) {
    result = true;
  } else if (err != GF_ENOENT && err != GF_CACHE_ENTRY_NOT_FOUND) {
    throwExceptionIfError("Region::localRemoveEx", err);
  }

  return result;
}

std::vector<std::shared_ptr<CacheableKey>> LocalRegion::keys() {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::keys);
  return keys_internal();
}

std::vector<std::shared_ptr<CacheableKey>> LocalRegion::serverKeys() {
  throw UnsupportedOperationException(
      "serverKeys is not supported for local regions.");
}

std::vector<std::shared_ptr<Cacheable>> LocalRegion::values() {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::values);

  std::vector<std::shared_ptr<Cacheable>> values;

  if (m_regionAttributes.getCachingEnabled()) {
    // invalidToken should not be added by the MapSegments.
    m_entries->getValues(values);
  }

  return values;
}

std::vector<std::shared_ptr<RegionEntry>> LocalRegion::entries(bool recursive) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::entries);

  std::vector<std::shared_ptr<RegionEntry>> entries;

  if (m_regionAttributes.getCachingEnabled()) {
    entries_internal(entries, recursive);
  }

  return entries;
}

HashMapOfCacheable LocalRegion::getAll(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  return getAll_internal(keys, aCallbackArgument, true);
}

HashMapOfCacheable LocalRegion::getAll_internal(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    bool addToLocalCache) {
  if (keys.empty()) {
    throw IllegalArgumentException("Region::getAll: zero keys provided");
  }

  int64_t sampleStartNanos = startStatOpTime();

  auto values = std::make_shared<HashMapOfCacheable>();
  auto exceptions = std::make_shared<HashMapOfException>();
  GfErrType err = getAllNoThrow(keys, values, exceptions, addToLocalCache,
                                aCallbackArgument);

  updateStatOpTime(m_regionStats->getStat(), m_regionStats->getGetAllTimeId(),
                   sampleStartNanos);

  throwExceptionIfError("Region::getAll", err);

  return *values;
}

uint32_t LocalRegion::size_remote() {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::size);
  if (m_regionAttributes.getCachingEnabled()) {
    return m_entries->size();
  }
  return 0;
}

uint32_t LocalRegion::size() {
  TXState* txState = getTXState();
  if (txState != nullptr) {
    if (isLocalOp()) {
      return GF_NOTSUP;
    }
    return size_remote();
  }

  return LocalRegion::size_remote();
}
RegionService& LocalRegion::getRegionService() const {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::getRegionService);
  return *m_cacheImpl->getCache();
}

CacheImpl* LocalRegion::getCacheImpl() const {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::getCache);
  return m_cacheImpl;
}

bool LocalRegion::containsValueForKey_remote(
    const std::shared_ptr<CacheableKey>& keyPtr) const {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::containsValueForKey);
  if (!m_regionAttributes.getCachingEnabled()) {
    return false;
  }
  std::shared_ptr<Cacheable> valuePtr;
  std::shared_ptr<MapEntryImpl> mePtr;
  m_entries->getEntry(keyPtr, mePtr, valuePtr);
  if (mePtr == nullptr) {
    return false;
  }
  return (valuePtr != nullptr && !CacheableToken::isInvalid(valuePtr));
}

bool LocalRegion::containsValueForKey(
    const std::shared_ptr<CacheableKey>& keyPtr) const {
  if (keyPtr == nullptr) {
    throw IllegalArgumentException(
        "LocalRegion::containsValueForKey: "
        "key is null");
  }

  TXState* txState = getTXState();
  if (txState == nullptr) {
    return LocalRegion::containsValueForKey_remote(keyPtr);
  }

  return containsValueForKey_remote(keyPtr);
}

bool LocalRegion::containsKeyOnServer(
    const std::shared_ptr<CacheableKey>&) const {
  throw UnsupportedOperationException(
      "LocalRegion::containsKeyOnServer: is not supported.");
}
std::vector<std::shared_ptr<CacheableKey>> LocalRegion::getInterestList()
    const {
  throw UnsupportedOperationException(
      "LocalRegion::getInterestList: is not supported.");
}
std::vector<std::shared_ptr<CacheableString>>
LocalRegion::getInterestListRegex() const {
  throw UnsupportedOperationException(
      "LocalRegion::getInterestListRegex: is not supported.");
}

bool LocalRegion::containsKey(
    const std::shared_ptr<CacheableKey>& keyPtr) const {
  if (keyPtr == nullptr) {
    throw IllegalArgumentException(
        "LocalRegion::containsKey: "
        "key is null");
  }
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::containsKey);
  return containsKey_internal(keyPtr);
}

void LocalRegion::setPersistenceManager(
    std::shared_ptr<PersistenceManager>& pmPtr) {
  m_persistenceManager = pmPtr;
  // set the memberVariable of LRUEntriesMap too.
  LRUEntriesMap* lruMap = dynamic_cast<LRUEntriesMap*>(m_entries);
  if (lruMap != nullptr) {
    lruMap->setPersistenceManager(pmPtr);
  }
}

void LocalRegion::setRegionExpiryTask() {
  if (!regionExpiryEnabled()) {
    return;
  }

  auto& manager = getCacheImpl()->getExpiryTaskManager();
  auto rptr = std::static_pointer_cast<RegionInternal>(shared_from_this());
  const auto& duration = getRegionExpiryDuration();
  auto&& task = std::make_shared<RegionExpiryTask>(
      manager, rptr, getRegionExpiryAction(), duration);
  expiry_task_id_ = manager.schedule(task, duration);
  LOG_FINE(
      "expiry for region [%s], expiry task id = %zu, duration = %s, "
      "action = %d",
      m_fullPath.c_str(), expiry_task_id_, to_string(duration).c_str(),
      static_cast<int32_t>(getRegionExpiryAction()));
}

void LocalRegion::registerEntryExpiryTask(
    std::shared_ptr<MapEntryImpl>& entry) {
  // locking is not required here since only the thread that creates
  // the entry will register the expiry task for that entry

  ExpEntryProperties& expProps = entry->getExpProperties();
  const auto& duration = getEntryExpiryDuration();
  auto& manager = getCacheImpl()->getExpiryTaskManager();
  auto region = std::static_pointer_cast<RegionInternal>(shared_from_this());
  auto task = std::make_shared<EntryExpiryTask>(
      manager, region, entry, getEntryExpirationAction(), duration);
  auto id = manager.schedule(std::move(task), duration);
  expProps.task_id(id);

  if (Log::enabled(LogLevel::Finest)) {
    std::shared_ptr<CacheableKey> key;
    entry->getKeyI(key);
    LOG_FINEST(
        "entry expiry in region [%s], key [%s], task id = %zu, "
        "duration = %s, action = %d",
        m_fullPath.c_str(), Utils::nullSafeToString(key).c_str(), id,
        to_string(duration).c_str(),
        static_cast<int32_t>(getEntryExpirationAction()));
  }
}

LocalRegion::~LocalRegion() noexcept {
  TryWriteGuard guard(m_rwLock, m_destroyPending);
  if (!m_destroyPending) {
    // TODO suspect
    // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
    release(false);
  }
  m_listener = nullptr;
  m_writer = nullptr;
  m_loader = nullptr;

  _GEODE_SAFE_DELETE(m_entries);
  _GEODE_SAFE_DELETE(m_regionStats);
}

/**
 * Release the region resources if not released already.
 */
void LocalRegion::release(bool invokeCallbacks) {
  if (m_released) {
    return;
  }
  LOG_FINE("LocalRegion::release entered for region %s", m_fullPath.c_str());
  m_released = true;

  if (m_regionStats != nullptr) {
    m_regionStats->close();
  }
  if (invokeCallbacks) {
    try {
      if (m_loader != nullptr) {
        m_loader->close(*this);
      }
      if (m_writer != nullptr) {
        m_writer->close(*this);
      }
      // TODO:  shouldn't listener also be here instead of
      // during CacheImpl.close()
    } catch (...) {
      LOG_WARN(
          "Region close caught unknown exception in loader/writer "
          "close; continuing");
    }
  }

  if (m_persistenceManager != nullptr) {
    m_persistenceManager->close();
    m_persistenceManager = nullptr;
  }
  if (m_entries != nullptr && m_regionAttributes.getCachingEnabled()) {
    m_entries->close();
  }
  LOG_FINE("LocalRegion::release done for region %s", m_fullPath.c_str());
}

/** Returns whether the specified key currently exists in this region.
 * This method is equivalent to <code>getEntry(key) != null</code>.
 *
 * @param keyPtr the key to check for an existing entry, type is
 *CacheableString
 *&
 * @return true if there is an entry in this region for the specified key
 *@throw RegionDestroyedException,  if region is destroyed.
 *@throw IllegalArgumentException, if the key is 'null'.
 *@throw NotConnectedException, if not connected to geode system.
 */
bool LocalRegion::containsKey_internal(
    const std::shared_ptr<CacheableKey>& keyPtr) const {
  if (keyPtr == nullptr) {
    throw IllegalArgumentException("Region::containsKey: key is null");
  }
  if (!m_regionAttributes.getCachingEnabled()) {
    return false;
  }
  return m_entries->containsKey(keyPtr);
}

std::vector<std::shared_ptr<Region>> LocalRegion::subregions_internal(
    const bool recursive) {
  auto&& lock = m_subRegions.make_lock();

  std::vector<std::shared_ptr<Region>> regions;
  regions.reserve(m_subRegions.size());

  for (const auto& kv : m_subRegions) {
    const auto& subRegion = kv.second;
    regions.push_back(subRegion);

    if (recursive) {
      if (auto localRegion =
              std::dynamic_pointer_cast<LocalRegion>(subRegion)) {
        auto subRegions = localRegion->subregions_internal(true);
        regions.insert(regions.end(), subRegions.begin(), subRegions.end());
      }
    }
  }

  return regions;
}

GfErrType LocalRegion::getNoThrow(
    const std::shared_ptr<CacheableKey>& keyPtr,
    std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;

  if (keyPtr == nullptr) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }
  TXState* txState = getTXState();
  if (txState != nullptr) {
    if (isLocalOp()) {
      return GF_NOTSUP;
    }
    std::shared_ptr<VersionTag> versionTag;
    err = getNoThrow_remote(keyPtr, value, aCallbackArgument, versionTag);
    if (err == GF_NOERR) {
      txState->setDirty();
    }
    if (CacheableToken::isInvalid(value) ||
        CacheableToken::isTombstone(value)) {
      value = nullptr;
    }
    return err;
  }

  m_regionStats->incGets();
  auto& cachePerfStats = m_cacheImpl->getCachePerfStats();
  cachePerfStats.incGets();

  // TODO:  CacheableToken::isInvalid should be completely hidden
  // inside MapSegment; this should be done both for the value obtained
  // from local cache as well as oldValue in every instance
  std::shared_ptr<MapEntryImpl> me;
  int updateCount = -1;
  bool isLoaderInvoked = false;
  bool isLocal = false;
  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  std::shared_ptr<Cacheable> localValue = nullptr;
  if (cachingEnabled) {
    isLocal = m_entries->get(keyPtr, value, me);
    if (isLocal && (value != nullptr && !CacheableToken::isInvalid(value))) {
      m_regionStats->incHits();
      cachePerfStats.incHits();
      updateAccessAndModifiedTimeForEntry(me, false);
      updateAccessAndModifiedTime(false);
      return err;  // found it in local cache...
    }
    localValue = value;
    value = nullptr;
    // start tracking the entry
    if (!m_regionAttributes.getConcurrencyChecksEnabled()) {
      updateCount =
          m_entries->addTrackerForEntry(keyPtr, value, true, false, false);
      LOG_DEBUG(
          "Region::get: added tracking with update counter [%d] for key "
          "[%s] with value [%s]",
          updateCount, Utils::nullSafeToString(keyPtr).c_str(),
          Utils::nullSafeToString(value).c_str());
    }
  }

  // remove tracking for the entry before exiting the function
  struct RemoveTracking {
   private:
    const std::shared_ptr<CacheableKey>& m_key;
    const int& m_updateCount;
    LocalRegion& m_region;

   public:
    RemoveTracking(const std::shared_ptr<CacheableKey>& key,
                   const int& updateCount, LocalRegion& region)
        : m_key(key), m_updateCount(updateCount), m_region(region) {}
    ~RemoveTracking() {
      if (m_updateCount >= 0 &&
          !m_region.getAttributes().getConcurrencyChecksEnabled()) {
        m_region.m_entries->removeTrackerForEntry(m_key);
      }
    }
  } _removeTracking(keyPtr, updateCount, *this);

  // The control will come here only when caching is disabled or/and
  // the entry was not found. In this case atleast update the region
  // access times.
  updateAccessAndModifiedTime(false);
  m_regionStats->incMisses();

  cachePerfStats.incMisses();
  std::shared_ptr<VersionTag> versionTag;
  // Get from some remote source (e.g. external java server) if required.
  err = getNoThrow_remote(keyPtr, value, aCallbackArgument, versionTag);

  // Its a cache missor it is invalid token then Check if we have a local
  // loader.
  if ((value == nullptr || CacheableToken::isInvalid(value) ||
       CacheableToken::isTombstone(value)) &&
      m_loader != nullptr) {
    try {
      isLoaderInvoked = true;
      /*Update the statistics*/
      int64_t sampleStartNanos = startStatOpTime();
      value = m_loader->load(*this, keyPtr, aCallbackArgument);
      updateStatOpTime(m_regionStats->getStat(),
                       m_regionStats->getLoaderCallTimeId(), sampleStartNanos);
      m_regionStats->incLoaderCallsCompleted();
    } catch (const Exception& ex) {
      LOG_ERROR("Error in CacheLoader::load: %s: %s", ex.getName().c_str(),
                ex.what());
      err = GF_CACHE_LOADER_EXCEPTION;
    } catch (...) {
      LOG_ERROR("Error in CacheLoader::load, unknown");
      err = GF_CACHE_LOADER_EXCEPTION;
    }
    if (err != GF_NOERR) {
      return err;
    }
  }

  std::shared_ptr<Cacheable> oldValue;
  // Found it somehow, so store it.
  if (value != nullptr /*&& value != CacheableToken::invalid( )*/ &&
      cachingEnabled &&
      !(CacheableToken::isTombstone(value) &&
        (localValue == nullptr || CacheableToken::isInvalid(localValue)))) {
    //  try to create the entry and if that returns an existing value
    // (e.g. from another thread or notification) then return that
    LOG_DEBUG(
        "Region::get: creating entry with tracking update counter [%d] for "
        "key "
        "[%s]",
        updateCount, Utils::nullSafeToString(keyPtr).c_str());
    if ((err = putLocal("Region::get", false, keyPtr, value, oldValue,
                        cachingEnabled, updateCount, 0, versionTag)) !=
        GF_NOERR) {
      if (err == GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION) {
        LOG_DEBUG(
            "Region::get: putLocal for key [%s] failed because the cache already contains \
          an entry with higher version.",
            Utils::nullSafeToString(keyPtr).c_str());
        if (CacheableToken::isInvalid(value) ||
            CacheableToken::isTombstone(value)) {
          value = nullptr;
        }
        // don't do anything and  exit
        return GF_NOERR;
      }

      LOG_DEBUG("Region::get: putLocal for key [%s] failed with error %d",
                Utils::nullSafeToString(keyPtr).c_str(), err);
      err = GF_NOERR;
      if (oldValue != nullptr && !CacheableToken::isInvalid(oldValue)) {
        LOG_DEBUG("Region::get: returning updated value [%s] for key [%s]",
                  Utils::nullSafeToString(oldValue).c_str(),
                  Utils::nullSafeToString(keyPtr).c_str());
        value = oldValue;
      }
    }
  }

  if (CacheableToken::isInvalid(value) || CacheableToken::isTombstone(value)) {
    value = nullptr;
  }

  // invokeCacheListenerForEntryEvent method has the check that if oldValue
  // is a CacheableToken then it sets it to nullptr; also determines if it
  // should be AFTER_UPDATE or AFTER_CREATE depending on oldValue, so don't
  // check here.
  if (isLoaderInvoked == false && err == GF_NOERR && value != nullptr) {
    err = invokeCacheListenerForEntryEvent(
        keyPtr, oldValue, value, aCallbackArgument, CacheEventFlags::NORMAL,
        AFTER_UPDATE, isLocal);
  }

  return err;
}

GfErrType LocalRegion::getAllNoThrow(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<HashMapOfCacheable>& values,
    const std::shared_ptr<HashMapOfException>& exceptions,
    const bool addToLocalCache,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;
  std::shared_ptr<Cacheable> value;

  TXState* txState = getTXState();
  if (txState != nullptr) {
    if (isLocalOp()) {
      return GF_NOTSUP;
    }
    err = getAllNoThrow_remote(&keys, values, exceptions, nullptr, false,
                               aCallbackArgument);
    if (err == GF_NOERR) {
      txState->setDirty();
    }

    return err;
  }
  // keys not in cache with their tracking numbers to be gotten using
  // a remote call
  std::vector<std::shared_ptr<CacheableKey>> serverKeys;
  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  bool regionAccessed = false;
  auto& cachePerfStats = m_cacheImpl->getCachePerfStats();

  for (const auto& key : keys) {
    std::shared_ptr<MapEntryImpl> me;
    value = nullptr;
    m_regionStats->incGets();
    cachePerfStats.incGets();
    if (values && cachingEnabled) {
      if (m_entries->get(key, value, me) && value &&
          !CacheableToken::isInvalid(value)) {
        m_regionStats->incHits();
        cachePerfStats.incHits();
        updateAccessAndModifiedTimeForEntry(me, false);
        regionAccessed = true;
        values->emplace(key, value);
      } else {
        value = nullptr;
      }
    }
    if (value == nullptr) {
      // Add to missed keys list.
      serverKeys.push_back(key);

      m_regionStats->incMisses();
      cachePerfStats.incMisses();
    }
    // TODO: No support for loaders in getAll for now.
  }
  if (regionAccessed) {
    updateAccessAndModifiedTime(false);
  }
  if (serverKeys.size() > 0) {
    err = getAllNoThrow_remote(&serverKeys, values, exceptions, nullptr,
                               addToLocalCache, aCallbackArgument);
  }
  m_regionStats->incGetAll();
  return err;
}

// encapsulates actions that need to be taken for a put() operation
class PutActions {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_UPDATE;
  static const EntryEventType s_afterEventType = AFTER_UPDATE;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = false;
  TXState* m_txState;

  inline explicit PutActions(LocalRegion& region) : m_region(region) {
    m_txState = TSSTXStateWrapper::get().getTXState();
  }

  inline static const char* name() { return "Region::put"; }

  inline static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& value,
                                    DataInput* delta = nullptr) {
    if (key == nullptr || (value == nullptr && delta == nullptr)) {
      return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return GF_NOERR;
  }

  inline void getCallbackOldValue(bool cachingEnabled,
                                  const std::shared_ptr<CacheableKey>& key,
                                  std::shared_ptr<MapEntryImpl>& entry,
                                  std::shared_ptr<Cacheable>& oldValue) const {
    if (cachingEnabled) {
      m_region.m_entries->getEntry(key, entry, oldValue);
    }
  }

  inline static void logCacheWriterFailure(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& oldValue) {
    bool isUpdate = (oldValue != nullptr);
    LOG_FINER("Cache writer vetoed %s for key %s",
              (isUpdate ? "update" : "create"),
              Utils::nullSafeToString(key).c_str());
  }

  inline GfErrType remoteUpdate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) {
    // propagate the put to remote server, if any
    return m_region.putNoThrow_remote(key, value, aCallbackArgument,
                                      versionTag);
  }

  inline GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Cacheable>& value,
                               std::shared_ptr<Cacheable>& oldValue,
                               bool cachingEnabled,
                               const CacheEventFlags /*eventFlags*/,
                               int updateCount,
                               std::shared_ptr<VersionTag> versionTag,
                               DataInput* delta = nullptr,
                               std::shared_ptr<EventId> eventId = nullptr,
                               bool /*afterRemote*/ = false) {
    return m_region.putLocal(name(), false, key, value, oldValue,
                             cachingEnabled, updateCount, 0, versionTag, delta,
                             eventId);
  }

 private:
  LocalRegion& m_region;
};

// encapsulates actions that need to be taken for a put() operation. This
// implementation allows
// null values in Put during transaction. See defect #743
class PutActionsTx : public PutActions {
 public:
  inline explicit PutActionsTx(LocalRegion& region) : PutActions(region) {}
  inline static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& /*value*/,
                                    DataInput* /*delta*/ = nullptr) {
    if (key == nullptr) {
      return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return GF_NOERR;
  }
};

// encapsulates actions that need to be taken for a create() operation
class CreateActions {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_CREATE;
  static const EntryEventType s_afterEventType = AFTER_CREATE;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = true;
  TXState* m_txState;

  inline explicit CreateActions(LocalRegion& region) : m_region(region) {
    m_txState = TSSTXStateWrapper::get().getTXState();
  }

  inline static const char* name() { return "Region::create"; }

  inline static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& /*value*/,
                                    DataInput* /*delta*/) {
    if (key == nullptr) {
      return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return GF_NOERR;
  }

  inline void getCallbackOldValue(
      bool /*cachingEnabled*/, const std::shared_ptr<CacheableKey>& /*key*/,
      std::shared_ptr<MapEntryImpl>& /*entry*/,
      std::shared_ptr<Cacheable>& /*oldValue*/) const {}

  inline static void logCacheWriterFailure(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& /*oldValue*/) {
    LOG_FINER("Cache writer vetoed create for key %s",
              Utils::nullSafeToString(key).c_str());
  }

  inline GfErrType remoteUpdate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) {
    return m_region.createNoThrow_remote(key, value, aCallbackArgument,
                                         versionTag);
  }

  inline GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Cacheable>& value,
                               std::shared_ptr<Cacheable>& oldValue,
                               bool cachingEnabled,
                               const CacheEventFlags /*eventFlags*/,
                               int updateCount,
                               std::shared_ptr<VersionTag> versionTag,
                               DataInput* /*delta*/ = nullptr,
                               std::shared_ptr<EventId> /*eventId*/ = nullptr,
                               bool /*afterRemote*/ = false) {
    return m_region.putLocal(name(), true, key, value, oldValue, cachingEnabled,
                             updateCount, 0, versionTag);
  }

 private:
  LocalRegion& m_region;
};

// encapsulates actions that need to be taken for a destroy() operation
class DestroyActions {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_DESTROY;
  static const EntryEventType s_afterEventType = AFTER_DESTROY;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = false;
  TXState* m_txState;

  inline explicit DestroyActions(LocalRegion& region) : m_region(region) {
    m_txState = TSSTXStateWrapper::get().getTXState();
  }

  inline static const char* name() { return "Region::destroy"; }

  inline static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& /*value*/,
                                    DataInput* /*delta*/) {
    if (key == nullptr) {
      return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return GF_NOERR;
  }

  inline void getCallbackOldValue(bool cachingEnabled,
                                  const std::shared_ptr<CacheableKey>& key,
                                  std::shared_ptr<MapEntryImpl>& entry,
                                  std::shared_ptr<Cacheable>& oldValue) const {
    if (cachingEnabled) {
      m_region.m_entries->getEntry(key, entry, oldValue);
    }
  }

  inline static void logCacheWriterFailure(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& /*oldValue*/) {
    LOG_FINER("Cache writer vetoed destroy for key %s",
              Utils::nullSafeToString(key).c_str());
  }

  inline GfErrType remoteUpdate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& /*value*/,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) {
    return m_region.destroyNoThrow_remote(key, aCallbackArgument, versionTag);
  }

  inline GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Cacheable>& /*value*/,
                               std::shared_ptr<Cacheable>& oldValue,
                               bool cachingEnabled,
                               const CacheEventFlags eventFlags,
                               int updateCount,
                               std::shared_ptr<VersionTag> versionTag,
                               DataInput* /*delta*/ = nullptr,
                               std::shared_ptr<EventId> /*eventId*/ = nullptr,
                               bool afterRemote = false) {
    auto& cachePerfStats = m_region.m_cacheImpl->getCachePerfStats();

    if (cachingEnabled) {
      std::shared_ptr<MapEntryImpl> entry;
      //  for notification invoke the listener even if the key does
      // not exist locally
      GfErrType err;
      LOG_DEBUG("Region::destroy: region [%s] destroying key [%s]",
                m_region.getFullPath().c_str(),
                Utils::nullSafeToString(key).c_str());
      if ((err = m_region.m_entries->remove(key, oldValue, entry, updateCount,
                                            versionTag, afterRemote)) !=
          GF_NOERR) {
        if (eventFlags.isNotification()) {
          LOG_DEBUG(
              "Region::destroy: region [%s] destroy key [%s] for "
              "notification having value [%s] failed with %d",
              m_region.getFullPath().c_str(),
              Utils::nullSafeToString(key).c_str(),
              Utils::nullSafeToString(oldValue).c_str(), err);
          err = GF_NOERR;
        }
        return err;
      }

      if (oldValue != nullptr) {
        LOG_DEBUG(
            "Region::destroy: region [%s] destroyed key [%s] having "
            "value [%s]",
            m_region.getFullPath().c_str(),
            Utils::nullSafeToString(key).c_str(),
            Utils::nullSafeToString(oldValue).c_str());
        // any cleanup required for the entry (e.g. removing from LRU list)
        if (entry != nullptr) {
          entry->cleanup(eventFlags);
        }
        // entry/region expiration
        if (!eventFlags.isEvictOrExpire()) {
          m_region.updateAccessAndModifiedTime(true);
        }
        // update the stats
        m_region.m_regionStats->setEntries(m_region.m_entries->size());
        cachePerfStats.incEntries(-1);
      }
    }
    // update the stats
    m_region.m_regionStats->incDestroys();
    cachePerfStats.incDestroys();
    return GF_NOERR;
  }

 private:
  LocalRegion& m_region;
};

// encapsulates actions that need to be taken for a remove() operation
class RemoveActions {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_DESTROY;
  static const EntryEventType s_afterEventType = AFTER_DESTROY;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = false;
  TXState* m_txState;
  bool allowNULLValue;

  inline explicit RemoveActions(LocalRegion& region)
      : m_region(region), m_ServerResponse(GF_ENOENT) {
    m_txState = TSSTXStateWrapper::get().getTXState();
    allowNULLValue = false;
  }

  inline static const char* name() { return "Region::remove"; }

  inline static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& /*value*/,
                                    DataInput* /*delta*/) {
    if (key == nullptr) {
      return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return GF_NOERR;
  }

  inline void getCallbackOldValue(bool cachingEnabled,
                                  const std::shared_ptr<CacheableKey>& key,
                                  std::shared_ptr<MapEntryImpl>& entry,
                                  std::shared_ptr<Cacheable>& oldValue) const {
    if (cachingEnabled) {
      m_region.m_entries->getEntry(key, entry, oldValue);
    }
  }

  inline static void logCacheWriterFailure(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& /*oldValue*/) {
    LOG_FINER("Cache writer vetoed remove for key %s",
              Utils::nullSafeToString(key).c_str());
  }

  bool serializedEqualTo(const std::shared_ptr<Cacheable>& lhs,
                         const std::shared_ptr<Cacheable>& rhs) {
    auto&& cache = *(m_region.getCacheImpl());

    if (const auto dataSerializablePrimitive =
            std::dynamic_pointer_cast<DataSerializablePrimitive>(lhs)) {
      return SerializableHelper<DataSerializablePrimitive>{}.equalTo(
          cache, dataSerializablePrimitive,
          std::dynamic_pointer_cast<DataSerializablePrimitive>(rhs));
    } else if (const auto dataSerializable =
                   std::dynamic_pointer_cast<DataSerializable>(lhs)) {
      return SerializableHelper<DataSerializable>{}.equalTo(
          cache, dataSerializable,
          std::dynamic_pointer_cast<DataSerializable>(rhs));
    } else if (const auto pdxSerializable =
                   std::dynamic_pointer_cast<PdxSerializable>(lhs)) {
      return SerializableHelper<PdxSerializable>{}.equalTo(
          cache, pdxSerializable,
          std::dynamic_pointer_cast<PdxSerializable>(rhs));
    } else if (const auto dataSerializableInternal =
                   std::dynamic_pointer_cast<DataSerializableInternal>(lhs)) {
      return SerializableHelper<DataSerializableInternal>{}.equalTo(
          cache, dataSerializableInternal,
          std::dynamic_pointer_cast<DataSerializableInternal>(rhs));
    } else {
      throw UnsupportedOperationException(
          "Serialization type not implemented.");
    }
  }

  inline GfErrType remoteUpdate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& newValue,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) {
    // propagate the remove to remote server, if any
    std::shared_ptr<Cacheable> oldValue;
    GfErrType err = GF_NOERR;
    if (!allowNULLValue && m_region.getAttributes().getCachingEnabled()) {
      m_region.getEntry(key, oldValue);
      if (oldValue != nullptr && newValue != nullptr) {
        if (!serializedEqualTo(oldValue, newValue)) {
          err = GF_ENOENT;
          return err;
        }
      } else if ((oldValue == nullptr || CacheableToken::isInvalid(oldValue))) {
        m_ServerResponse = m_region.removeNoThrow_remote(
            key, newValue, aCallbackArgument, versionTag);

        return m_ServerResponse;
      } else if (oldValue != nullptr && newValue == nullptr) {
        err = GF_ENOENT;
        return err;
      }
    }
    if (allowNULLValue) {
      m_ServerResponse =
          m_region.removeNoThrowEX_remote(key, aCallbackArgument, versionTag);
    } else {
      m_ServerResponse = m_region.removeNoThrow_remote(
          key, newValue, aCallbackArgument, versionTag);
    }
    LOG_DEBUG("serverResponse::%d", m_ServerResponse);
    return m_ServerResponse;
  }

  inline GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Cacheable>& value,
                               std::shared_ptr<Cacheable>& oldValue,
                               bool cachingEnabled,
                               const CacheEventFlags eventFlags,
                               int updateCount,
                               std::shared_ptr<VersionTag> versionTag,
                               DataInput* /*delta*/ = nullptr,
                               std::shared_ptr<EventId> /*eventId*/ = nullptr,
                               bool afterRemote = false) {
    std::shared_ptr<Cacheable> valuePtr;
    GfErrType err = GF_NOERR;
    if (!allowNULLValue && cachingEnabled) {
      m_region.getEntry(key, valuePtr);
      if (valuePtr != nullptr && value != nullptr) {
        if (!serializedEqualTo(valuePtr, value)) {
          err = GF_ENOENT;
          return err;
        }
      } else if (value == nullptr && (!CacheableToken::isInvalid(valuePtr) ||
                                      valuePtr == nullptr)) {
        err = (m_ServerResponse == 0 && valuePtr == nullptr) ? GF_NOERR
                                                             : GF_ENOENT;
        if (updateCount >= 0 &&
            !m_region.getAttributes().getConcurrencyChecksEnabled()) {
          // This means server has deleted an entry & same entry has been
          // destroyed locally
          // So call removeTrackerForEntry to remove key that
          // was added in the  map during addTrackerForEntry call.
          m_region.m_entries->removeTrackerForEntry(key);
        }
        return err;
      } else if (valuePtr == nullptr && value != nullptr &&
                 m_ServerResponse != 0) {
        err = GF_ENOENT;
        return err;
      }
    }
    auto& cachePerfStats = m_region.m_cacheImpl->getCachePerfStats();

    if (cachingEnabled) {
      std::shared_ptr<MapEntryImpl> entry;
      //  for notification invoke the listener even if the key does
      // not exist locally
      LOG_DEBUG("Region::remove: region [%s] removing key [%s]",
                m_region.getFullPath().c_str(),
                Utils::nullSafeToString(key).c_str());
      if ((err = m_region.m_entries->remove(key, oldValue, entry, updateCount,
                                            versionTag, afterRemote)) !=
          GF_NOERR) {
        if (eventFlags.isNotification()) {
          LOG_DEBUG(
              "Region::remove: region [%s] remove key [%s] for "
              "notification having value [%s] failed with %d",
              m_region.getFullPath().c_str(),
              Utils::nullSafeToString(key).c_str(),
              Utils::nullSafeToString(oldValue).c_str(), err);
          err = GF_NOERR;
        }
        return err;
      }
      if (oldValue != nullptr) {
        LOG_DEBUG(
            "Region::remove: region [%s] removed key [%s] having "
            "value [%s]",
            m_region.getFullPath().c_str(),
            Utils::nullSafeToString(key).c_str(),
            Utils::nullSafeToString(oldValue).c_str());
        // any cleanup required for the entry (e.g. removing from LRU list)
        if (entry != nullptr) {
          entry->cleanup(eventFlags);
        }
        // entry/region expiration
        if (!eventFlags.isEvictOrExpire()) {
          m_region.updateAccessAndModifiedTime(true);
        }
        // update the stats
        m_region.m_regionStats->setEntries(m_region.m_entries->size());
        cachePerfStats.incEntries(-1);
      }
    }
    // update the stats
    m_region.m_regionStats->incDestroys();
    cachePerfStats.incDestroys();
    return GF_NOERR;
  }

 private:
  LocalRegion& m_region;
  GfErrType m_ServerResponse;
};

class RemoveActionsEx : public RemoveActions {
 public:
  inline explicit RemoveActionsEx(LocalRegion& region) : RemoveActions(region) {
    allowNULLValue = true;
  }
};

// encapsulates actions that need to be taken for a invalidate() operation
class InvalidateActions {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_INVALIDATE;
  static const EntryEventType s_afterEventType = AFTER_INVALIDATE;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = false;
  TXState* m_txState;

  inline explicit InvalidateActions(LocalRegion& region) : m_region(region) {
    m_txState = TSSTXStateWrapper::get().getTXState();
  }

  inline static const char* name() { return "Region::invalidate"; }

  inline static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& /*value*/,
                                    DataInput* /*delta*/ = nullptr) {
    if (key == nullptr) {
      return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return GF_NOERR;
  }

  inline void getCallbackOldValue(bool cachingEnabled,
                                  const std::shared_ptr<CacheableKey>& key,
                                  std::shared_ptr<MapEntryImpl>& entry,
                                  std::shared_ptr<Cacheable>& oldValue) const {
    if (cachingEnabled) {
      m_region.m_entries->getEntry(key, entry, oldValue);
    }
  }

  inline static void logCacheWriterFailure(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& oldValue) {
    bool isUpdate = (oldValue != nullptr);
    LOG_FINER("Cache writer vetoed %s for key %s",
              (isUpdate ? "update" : "invalidate"),
              Utils::nullSafeToString(key).c_str());
  }

  inline GfErrType remoteUpdate(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& /*value*/,
      const std::shared_ptr<Serializable>& aCallbackArgument,
      std::shared_ptr<VersionTag>& versionTag) {
    // propagate the invalidate to remote server, if any
    return m_region.invalidateNoThrow_remote(key, aCallbackArgument,
                                             versionTag);
  }

  inline GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Cacheable>& value,
                               std::shared_ptr<Cacheable>& /*oldValue*/,
                               bool /*cachingEnabled*/,
                               const CacheEventFlags eventFlags,
                               int /*updateCount*/,
                               std::shared_ptr<VersionTag> versionTag,
                               DataInput* /*delta*/ = nullptr,
                               std::shared_ptr<EventId> /*eventId*/ = nullptr,
                               bool /*afterRemote*/ = false) {
    return m_region.invalidateLocal(name(), key, value, eventFlags, versionTag);
  }

 private:
  LocalRegion& m_region;
};

template <typename TAction>
GfErrType LocalRegion::updateNoThrow(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<Cacheable>& oldValue, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
    DataInput* delta, std::shared_ptr<EventId> eventId) {
  GfErrType err = GF_NOERR;
  if ((err = TAction::checkArgs(key, value, delta)) != GF_NOERR) {
    return err;
  }

  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);

  TAction action(*this);
  TXState* txState = action.m_txState;
  if (txState != nullptr) {
    if (isLocalOp(&eventFlags)) {
      return GF_NOTSUP;
    }
    /* adongre - Coverity II
     * CID 29194 (6): Parse warning (PW.PARAMETER_HIDDEN)
     */
    // std::shared_ptr<VersionTag> versionTag;
    err = action.remoteUpdate(key, value, aCallbackArgument, versionTag);
    if (err == GF_NOERR) {
      txState->setDirty();
    }

    return err;
  }

  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  std::shared_ptr<MapEntryImpl> entry;

  //  do not invoke the writer in case of notification/eviction
  // or expiration
  if (m_writer != nullptr && eventFlags.invokeCacheWriter()) {
    action.getCallbackOldValue(cachingEnabled, key, entry, oldValue);
    // invokeCacheWriterForEntryEvent method has the check that if oldValue
    // is a CacheableToken then it sets it to nullptr; also determines if it
    // should be BEFORE_UPDATE or BEFORE_CREATE depending on oldValue
    if (!invokeCacheWriterForEntryEvent(key, oldValue, value, aCallbackArgument,
                                        eventFlags,
                                        TAction::s_beforeEventType)) {
      TAction::logCacheWriterFailure(key, oldValue);
      return GF_CACHEWRITER_ERROR;
    }
  }
  bool remoteOpDone = false;
  //  try the remote update; but if this fails (e.g. due to security
  // exception) do not do the local update
  // uses the technique of adding a tracking to the entry before proceeding
  // for put; if the update counter changes when the remote update completes
  // then it means that the local entry was overwritten in the meantime
  // by a notification or another thread, so we do not do the local update
  if (!eventFlags.isLocal() && !eventFlags.isNotification()) {
    if (cachingEnabled && updateCount < 0 &&
        !m_regionAttributes.getConcurrencyChecksEnabled()) {
      // add a tracking for the entry
      if ((updateCount = m_entries->addTrackerForEntry(
               key, oldValue, TAction::s_addIfAbsent, TAction::s_failIfPresent,
               true)) < 0) {
        if (oldValue != nullptr) {
          // fail for "create" when entry exists
          return GF_CACHE_ENTRY_EXISTS;
        }
      }
    }
    // propagate the update to remote server, if any
    err = action.remoteUpdate(key, value, aCallbackArgument, versionTag);
    if (err != GF_NOERR) {
      if (updateCount >= 0 &&
          !m_regionAttributes.getConcurrencyChecksEnabled()) {
        m_entries->removeTrackerForEntry(key);
      }
      return err;
    }
    remoteOpDone = true;
  }
  if (!eventFlags.isNotification() || getProcessedMarker()) {
    if ((err = action.localUpdate(key, value, oldValue, cachingEnabled,
                                  eventFlags, updateCount, versionTag, delta,
                                  eventId, remoteOpDone)) ==
        GF_CACHE_ENTRY_UPDATED) {
      LOG_FINEST(
          "%s: did not change local value for key [%s] since it has "
          "been updated by another thread while operation was in progress",
          TAction::name(), Utils::nullSafeToString(key).c_str());
      err = GF_NOERR;
    } else if (err == GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION) {
      LOG_DEBUG(
          "Region::localUpdate: updateNoThrow<%s> for key [%s] failed because the cache already contains \
        an entry with higher version. The cache listener will not be invoked.",
          TAction::name(), Utils::nullSafeToString(key).c_str());
      // Cache listener won't be called in this case
      return GF_NOERR;
    } else if (err == GF_INVALID_DELTA) {
      LOG_DEBUG(
          "Region::localUpdate: updateNoThrow<%s> for key [%s] failed "
          "because "
          "of invalid delta.",
          TAction::name(), Utils::nullSafeToString(key).c_str());
      m_cacheImpl->getCachePerfStats().incFailureOnDeltaReceived();
      // Get full object from server.
      std::shared_ptr<Cacheable>& newValue1 =
          const_cast<std::shared_ptr<Cacheable>&>(value);
      std::shared_ptr<VersionTag> versionTag1;
      err = getNoThrow_FullObject(eventId, newValue1, versionTag1);
      if (err == GF_NOERR && newValue1 != nullptr) {
        err = m_entries->put(key, newValue1, entry, oldValue, updateCount, 0,
                             versionTag1 != nullptr ? versionTag1 : versionTag);
        if (err == GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION) {
          LOG_DEBUG(
              "Region::localUpdate: updateNoThrow<%s> for key [%s] failed because the cache already contains \
            an entry with higher version. The cache listener will not be invoked.",
              TAction::name(), Utils::nullSafeToString(key).c_str());
          // Cache listener won't be called in this case
          return GF_NOERR;
        } else if (err != GF_NOERR) {
          return err;
        }
      }
    } else if (err != GF_NOERR) {
      return err;
    }
  } else {  // if (getProcessedMarker())
    action.getCallbackOldValue(cachingEnabled, key, entry, oldValue);
    if (updateCount >= 0 && !m_regionAttributes.getConcurrencyChecksEnabled()) {
      m_entries->removeTrackerForEntry(key);
    }
  }

  if (!eventFlags.isNoCallbacks()) {
    // invokeCacheListenerForEntryEvent method has the check that if oldValue
    // is a CacheableToken then it sets it to nullptr; also determines if it
    // should be AFTER_UPDATE or AFTER_CREATE depending on oldValue
    err = invokeCacheListenerForEntryEvent(key, oldValue, value,
                                           aCallbackArgument, eventFlags,
                                           TAction::s_afterEventType);
  }

  return err;
}

template <typename TAction>
GfErrType LocalRegion::updateNoThrowTX(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<Cacheable>& oldValue, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
    DataInput* delta, std::shared_ptr<EventId> eventId) {
  GfErrType err = GF_NOERR;
  if ((err = TAction::checkArgs(key, value, delta)) != GF_NOERR) {
    return err;
  }

  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  TAction action(*this);

  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  std::shared_ptr<MapEntryImpl> entry;

  if (!eventFlags.isNotification() || getProcessedMarker()) {
    if ((err = action.localUpdate(key, value, oldValue, cachingEnabled,
                                  eventFlags, updateCount, versionTag, delta,
                                  eventId)) == GF_CACHE_ENTRY_UPDATED) {
      LOG_FINEST(
          "%s: did not change local value for key [%s] since it has "
          "been updated by another thread while operation was in progress",
          TAction::name(), Utils::nullSafeToString(key).c_str());
      err = GF_NOERR;
    } else if (err == GF_CACHE_ENTRY_NOT_FOUND) {
      // Entry not found. Possibly because the entry was added and removed in
      // the
      // same transaction. Ignoring this error #739
      LOG_FINE(
          "%s: No entry found. Possibly because the entry was added and "
          "removed in the same transaction. "
          "Ignoring this error. ",
          TAction::name(), Utils::nullSafeToString(key).c_str());
      err = GF_NOERR;
    } else if (err != GF_NOERR) {
      return err;
    }
  } else {  // if (getProcessedMarker())
    action.getCallbackOldValue(cachingEnabled, key, entry, oldValue);
    if (updateCount >= 0 && !m_regionAttributes.getConcurrencyChecksEnabled()) {
      m_entries->removeTrackerForEntry(key);
    }
  }
  // invokeCacheListenerForEntryEvent method has the check that if oldValue
  // is a CacheableToken then it sets it to nullptr; also determines if it
  // should be AFTER_UPDATE or AFTER_CREATE depending on oldValue
  err =
      invokeCacheListenerForEntryEvent(key, oldValue, value, aCallbackArgument,
                                       eventFlags, TAction::s_afterEventType);
  return err;
}

GfErrType LocalRegion::putNoThrow(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<Cacheable>& oldValue, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
    DataInput* delta, std::shared_ptr<EventId> eventId) {
  return updateNoThrow<PutActions>(key, value, aCallbackArgument, oldValue,
                                   updateCount, eventFlags, versionTag, delta,
                                   eventId);
}

GfErrType LocalRegion::putNoThrowTX(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    std::shared_ptr<Cacheable>& oldValue, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag,
    DataInput* delta, std::shared_ptr<EventId> eventId) {
  return updateNoThrowTX<PutActionsTx>(key, value, aCallbackArgument, oldValue,
                                       updateCount, eventFlags, versionTag,
                                       delta, eventId);
}

GfErrType LocalRegion::createNoThrow(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrow<CreateActions>(key, value, aCallbackArgument, oldValue,
                                      updateCount, eventFlags, versionTag);
}

GfErrType LocalRegion::destroyNoThrow(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrow<DestroyActions>(key, nullptr, aCallbackArgument,
                                       oldValue, updateCount, eventFlags,
                                       versionTag);
}

GfErrType LocalRegion::destroyNoThrowTX(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrowTX<DestroyActions>(key, nullptr, aCallbackArgument,
                                         oldValue, updateCount, eventFlags,
                                         versionTag);
}

GfErrType LocalRegion::removeNoThrow(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>& value,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrow<RemoveActions>(key, value, aCallbackArgument, oldValue,
                                      updateCount, eventFlags, versionTag);
}

GfErrType LocalRegion::removeNoThrowEx(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrow<RemoveActionsEx>(key, nullptr, aCallbackArgument,
                                        oldValue, updateCount, eventFlags,
                                        versionTag);
}

GfErrType LocalRegion::invalidateNoThrow(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrow<InvalidateActions>(key, nullptr, aCallbackArgument,
                                          oldValue, updateCount, eventFlags,
                                          versionTag);
}

GfErrType LocalRegion::invalidateNoThrowTX(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& aCallbackArgument, int updateCount,
    const CacheEventFlags eventFlags, std::shared_ptr<VersionTag> versionTag) {
  std::shared_ptr<Cacheable> oldValue;
  return updateNoThrowTX<InvalidateActions>(key, nullptr, aCallbackArgument,
                                            oldValue, updateCount, eventFlags,
                                            versionTag);
}

GfErrType LocalRegion::putAllNoThrow(
    const HashMapOfCacheable& map, std::chrono::milliseconds timeout,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;
  // std::shared_ptr<VersionTag> versionTag;
  std::shared_ptr<VersionedCacheableObjectPartList>
      versionedObjPartListPtr;  //= new VersionedCacheableObjectPartList();
  TXState* txState = getTXState();
  if (txState != nullptr) {
    if (isLocalOp()) {
      return GF_NOTSUP;
    }

    err = putAllNoThrow_remote(map, /*versionTag*/ versionedObjPartListPtr,
                               timeout, aCallbackArgument);
    if (err == GF_NOERR) {
      txState->setDirty();
    }

    return err;
  }

  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  MapOfOldValue oldValueMap;

  // remove tracking for the entries befor exiting the function
  struct RemoveTracking {
   private:
    const MapOfOldValue& m_oldValueMap;
    LocalRegion& m_region;

   public:
    RemoveTracking(const MapOfOldValue& oldValueMap, LocalRegion& region)
        : m_oldValueMap(oldValueMap), m_region(region) {}
    ~RemoveTracking() {
      if (!m_region.getAttributes().getConcurrencyChecksEnabled()) {
        // need to remove the tracking added to the entries at the end
        for (MapOfOldValue::const_iterator iter = m_oldValueMap.begin();
             iter != m_oldValueMap.end(); ++iter) {
          if (iter->second.second >= 0) {
            m_region.m_entries->removeTrackerForEntry(iter->first);
          }
        }
      }
    }
  } _removeTracking(oldValueMap, *this);

  if (cachingEnabled || m_writer != nullptr) {
    std::shared_ptr<Cacheable> oldValue;
    for (const auto& iter : map) {
      const auto& key = iter.first;
      if (cachingEnabled && !m_regionAttributes.getConcurrencyChecksEnabled()) {
        int updateCount =
            m_entries->addTrackerForEntry(key, oldValue, true, false, true);
        oldValueMap.insert(
            std::make_pair(key, std::make_pair(oldValue, updateCount)));
      }
      if (m_writer != nullptr) {
        // invokeCacheWriterForEntryEvent method has the check that if
        // oldValue is a CacheableToken then it sets it to nullptr; also
        // determines if it should be BEFORE_UPDATE or BEFORE_CREATE depending
        // on oldValue
        if (!invokeCacheWriterForEntryEvent(
                key, oldValue, iter.second, aCallbackArgument,
                CacheEventFlags::LOCAL, BEFORE_UPDATE)) {
          PutActions::logCacheWriterFailure(key, oldValue);
          return GF_CACHEWRITER_ERROR;
        }
      }
    }
  }
  // try remote putAll, if any
  if ((err = putAllNoThrow_remote(map, versionedObjPartListPtr, timeout,
                                  aCallbackArgument)) != GF_NOERR) {
    return err;
  }
  // next the local puts
  GfErrType localErr;
  std::shared_ptr<VersionTag> versionTag;

  if (cachingEnabled) {
    if (m_isPRSingleHopEnabled) { /*New PRSingleHop Case:: PR Singlehop
                                     condition*/
      for (size_t keyIndex = 0;
           keyIndex < versionedObjPartListPtr->getSucceededKeys()->size();
           keyIndex++) {
        const auto valPtr =
            versionedObjPartListPtr->getSucceededKeys()->at(keyIndex);
        const auto& mapIter = map.find(valPtr);
        std::shared_ptr<CacheableKey> key = nullptr;
        std::shared_ptr<Cacheable> value = nullptr;

        if (mapIter != map.end()) {
          key = mapIter->first;
          value = mapIter->second;
        } else {
          // ThrowERROR
          LOG_ERROR(
              "ERROR :: LocalRegion::putAllNoThrow() Key must be found in "
              "the "
              "usermap");
        }

        if (versionedObjPartListPtr) {
          LOG_DEBUG(
              "versionedObjPartListPtr->getVersionedTagptr().size() = %zu",
              versionedObjPartListPtr->getVersionedTagptr().size());
          if (versionedObjPartListPtr->getVersionedTagptr().size() > 0) {
            versionTag =
                versionedObjPartListPtr->getVersionedTagptr()[keyIndex];
          }
        }
        std::pair<std::shared_ptr<Cacheable>, int>& p = oldValueMap[key];
        if ((localErr = LocalRegion::putNoThrow(
                 key, value, aCallbackArgument, p.first, p.second,
                 CacheEventFlags::LOCAL | CacheEventFlags::NOCACHEWRITER,
                 versionTag)) == GF_CACHE_ENTRY_UPDATED) {
          LOG_FINEST(
              "Region::putAll: did not change local value for key [%s] "
              "since it has been updated by another thread while operation "
              "was "
              "in progress",
              Utils::nullSafeToString(key).c_str());
        } else if (localErr == GF_CACHE_LISTENER_EXCEPTION) {
          LOG_FINER("Region::putAll: invoke listener error [%d] for key [%s]",
                    localErr, Utils::nullSafeToString(key).c_str());
          err = localErr;
        } else if (localErr != GF_NOERR) {
          return localErr;
        }
      }      // End of for loop
    } else { /*Non SingleHop case :: PUTALL has taken multiple hops*/
      LOG_DEBUG(
          "NILKANTH LocalRegion::putAllNoThrow m_isPRSingleHopEnabled = %d "
          "expected false",
          m_isPRSingleHopEnabled);
      int index = 0;
      for (const auto& iter : map) {
        const auto& key = iter.first;
        const auto& value = iter.second;
        auto& p = oldValueMap[key];

        if (versionedObjPartListPtr) {
          LOG_DEBUG(
              "versionedObjPartListPtr->getVersionedTagptr().size() = %zu ",
              versionedObjPartListPtr->getVersionedTagptr().size());
          if (versionedObjPartListPtr->getVersionedTagptr().size() > 0) {
            versionTag = versionedObjPartListPtr->getVersionedTagptr()[index++];
          }
        }
        if ((localErr = LocalRegion::putNoThrow(
                 key, value, aCallbackArgument, p.first, p.second,
                 CacheEventFlags::LOCAL | CacheEventFlags::NOCACHEWRITER,
                 versionTag)) == GF_CACHE_ENTRY_UPDATED) {
          LOG_FINEST(
              "Region::putAll: did not change local value for key [%s] "
              "since it has been updated by another thread while operation "
              "was "
              "in progress",
              Utils::nullSafeToString(key).c_str());
        } else if (localErr == GF_CACHE_LISTENER_EXCEPTION) {
          LOG_FINER("Region::putAll: invoke listener error [%d] for key [%s]",
                    localErr, Utils::nullSafeToString(key).c_str());
          err = localErr;
        } else if (localErr != GF_NOERR) {
          return localErr;
        }
      }
    }
  }

  m_regionStats->incPutAll();
  return err;
}

GfErrType LocalRegion::removeAllNoThrow(
    const std::vector<std::shared_ptr<CacheableKey>>& keys,
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  // 1. check destroy pending
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;
  std::shared_ptr<VersionedCacheableObjectPartList> versionedObjPartListPtr;

  // 2.check transaction state and do remote op
  TXState* txState = getTXState();
  if (txState != nullptr) {
    if (isLocalOp()) return GF_NOTSUP;
    err = removeAllNoThrow_remote(keys, versionedObjPartListPtr,
                                  aCallbackArgument);
    if (err == GF_NOERR) txState->setDirty();
    return err;
  }

  // 3.add tracking
  bool cachingEnabled = m_regionAttributes.getCachingEnabled();

  // 4. do remote removeAll
  err =
      removeAllNoThrow_remote(keys, versionedObjPartListPtr, aCallbackArgument);
  if (err != GF_NOERR) {
    return err;
  }

  // 5. update local cache
  GfErrType localErr;
  std::shared_ptr<VersionTag> versionTag;
  if (cachingEnabled) {
    std::vector<std::shared_ptr<CacheableKey>>* keysPtr;
    if (m_isPRSingleHopEnabled) {
      keysPtr = versionedObjPartListPtr->getSucceededKeys().get();
    } else {
      keysPtr = const_cast<std::vector<std::shared_ptr<CacheableKey>>*>(&keys);
    }

    for (size_t keyIndex = 0; keyIndex < keysPtr->size(); keyIndex++) {
      auto key = keysPtr->at(keyIndex);
      if (versionedObjPartListPtr) {
        LOG_DEBUG("versionedObjPartListPtr->getVersionedTagptr().size() = %zu ",
                  versionedObjPartListPtr->getVersionedTagptr().size());
        if (versionedObjPartListPtr->getVersionedTagptr().size() > 0) {
          versionTag = versionedObjPartListPtr->getVersionedTagptr()[keyIndex];
        }
        if (versionTag == nullptr) {
          LOG_DEBUG(
              "RemoveAll hits EntryNotFoundException at server side for key "
              "[%s], not to destroy it from local cache.",
              Utils::nullSafeToString(key).c_str());
          continue;
        }
      }

      if ((localErr = LocalRegion::destroyNoThrow(
               key, aCallbackArgument, -1,
               CacheEventFlags::LOCAL | CacheEventFlags::NOCACHEWRITER,
               versionTag)) == GF_CACHE_ENTRY_UPDATED) {
        LOG_FINEST(
            "Region::removeAll: did not remove local value for key [%s] "
            "since it has been updated by another thread while operation was "
            "in progress",
            Utils::nullSafeToString(key).c_str());
      } else if (localErr == GF_CACHE_LISTENER_EXCEPTION) {
        LOG_FINER("Region::removeAll: invoke listener error [%d] for key [%s]",
                  localErr, Utils::nullSafeToString(key).c_str());
        err = localErr;
      } else if (localErr == GF_CACHE_ENTRY_NOT_FOUND) {
        LOG_FINER("Region::removeAll: error [%d] for key [%s]", localErr,
                  Utils::nullSafeToString(key).c_str());
      } else if (localErr != GF_NOERR) {
        return localErr;
      }
    }  // End of for loop
  }

  // 6.update stats
  m_regionStats->incRemoveAll();
  return err;
}

void LocalRegion::clear(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  /*update the stats */
  int64_t sampleStartNanos = startStatOpTime();
  localClear(aCallbackArgument);
  updateStatOpTime(m_regionStats->getStat(), m_regionStats->getClearsId(),
                   sampleStartNanos);
}
void LocalRegion::localClear(
    const std::shared_ptr<Serializable>& aCallbackArgument) {
  GfErrType err = localClearNoThrow(aCallbackArgument, CacheEventFlags::LOCAL);
  if (err != GF_NOERR) throwExceptionIfError("LocalRegion::localClear", err);
}
GfErrType LocalRegion::localClearNoThrow(
    const std::shared_ptr<Serializable>& aCallbackArgument,
    const CacheEventFlags eventFlags) {
  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  /*Update the stats for clear*/
  m_regionStats->incClears();
  GfErrType err = GF_NOERR;
  TryReadGuard guard(m_rwLock, m_destroyPending);
  if (m_released || m_destroyPending) return err;
  if (!invokeCacheWriterForRegionEvent(aCallbackArgument, eventFlags,
                                       BEFORE_REGION_CLEAR)) {
    LOG_FINE("Cache writer prevented region clear");
    return GF_CACHEWRITER_ERROR;
  }
  if (cachingEnabled == true) m_entries->clear();
  if (!eventFlags.isNormal()) {
    err = invokeCacheListenerForRegionEvent(aCallbackArgument, eventFlags,
                                            AFTER_REGION_CLEAR);
  }
  return err;
}

GfErrType LocalRegion::invalidateLocal(
    const std::string& name, const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<Cacheable>& value, const CacheEventFlags eventFlags,
    std::shared_ptr<VersionTag> versionTag) {
  if (keyPtr == nullptr) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);

  GfErrType err = GF_NOERR;

  bool cachingEnabled = m_regionAttributes.getCachingEnabled();
  std::shared_ptr<Cacheable> oldValue;
  std::shared_ptr<MapEntryImpl> me;

  if (!eventFlags.isNotification() || getProcessedMarker()) {
    if (cachingEnabled) {
      LOG_DEBUG("%s: region [%s] invalidating key [%s], value [%s]",
                name.c_str(), getFullPath().c_str(),
                Utils::nullSafeToString(keyPtr).c_str(),
                Utils::nullSafeToString(value).c_str());
      /* adongre - Coverity II
       * CID 29193: Parse warning (PW.PARAMETER_HIDDEN)
       */
      // std::shared_ptr<VersionTag> versionTag;
      if ((err = m_entries->invalidate(keyPtr, me, oldValue, versionTag)) !=
          GF_NOERR) {
        if (eventFlags.isNotification()) {
          LOG_DEBUG(
              "Region::invalidate: region [%s] invalidate key [%s] "
              "failed with error %d",
              getFullPath().c_str(), Utils::nullSafeToString(keyPtr).c_str(),
              err);
        }
        if (err == GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION) {
          LOG_DEBUG(
              "Region::invalidateLocal: invalidate for key [%s] failed because the cache already contains \
            an entry with higher version. The cache listener will not be invoked.",
              Utils::nullSafeToString(keyPtr).c_str());
          // Cache listener won't be called in this case
          return GF_NOERR;
        }
        //  for notification invoke the listener even if the key does
        // not exist locally
        if (!eventFlags.isNotification() || err != GF_CACHE_ENTRY_NOT_FOUND) {
          return err;
        } else {
          err = GF_NOERR;
        }
      } else {
        LOG_DEBUG("Region::invalidate: region [%s] invalidated key [%s]",
                  getFullPath().c_str(),
                  Utils::nullSafeToString(keyPtr).c_str());
      }
      // entry/region expiration
      if (!eventFlags.isEvictOrExpire()) {
        updateAccessAndModifiedTime(true);
      }
    }
  } else {  // if (getProcessedMarker())
    if (cachingEnabled) {
      m_entries->getEntry(keyPtr, me, oldValue);
    }
  }
  return err;
}

GfErrType LocalRegion::invalidateRegionNoThrowOnSubRegions(
    const std::shared_ptr<Serializable>& aCallbackArgument,
    const CacheEventFlags eventFlags) {
  auto&& lock = m_subRegions.make_lock();
  for (const auto& kv : m_subRegions) {
    if (auto subRegion = std::dynamic_pointer_cast<RegionInternal>(kv.second)) {
      auto err =
          subRegion->invalidateRegionNoThrow(aCallbackArgument, eventFlags);
      if (err != GF_NOERR) {
        return err;
      }
    }
  }

  return GF_NOERR;
}

GfErrType LocalRegion::invalidateRegionNoThrow(
    const std::shared_ptr<Serializable>& aCallbackArgument,
    const CacheEventFlags eventFlags) {
  CHECK_DESTROY_PENDING_NOTHROW(TryReadGuard);
  GfErrType err = GF_NOERR;

  if (m_regionAttributes.getCachingEnabled()) {
    std::vector<std::shared_ptr<CacheableKey>> v = keys_internal();
    auto size = v.size();
    std::shared_ptr<MapEntryImpl> me;
    for (decltype(size) i = 0; i < size; i++) {
      {
        std::shared_ptr<Cacheable> oldValue;
        // invalidate all the entries with a nullptr versionTag
        std::shared_ptr<VersionTag> versionTag;
        m_entries->invalidate(v.at(i), me, oldValue, versionTag);
        if (!eventFlags.isEvictOrExpire()) {
          updateAccessAndModifiedTimeForEntry(me, true);
        }
      }
    }
    if (!eventFlags.isEvictOrExpire()) {
      updateAccessAndModifiedTime(true);
    }
  }

  // try remote region invalidate, if any
  if (!eventFlags.isLocal()) {
    err = invalidateRegionNoThrow_remote(aCallbackArgument);
    if (err != GF_NOERR) return err;
  }

  err = invalidateRegionNoThrowOnSubRegions(aCallbackArgument, eventFlags);
  if (err != GF_NOERR) {
    return err;
  }

  err = invokeCacheListenerForRegionEvent(aCallbackArgument, eventFlags,
                                          AFTER_REGION_INVALIDATE);

  return err;
}

GfErrType LocalRegion::destroyRegionNoThrow(
    const std::shared_ptr<Serializable>& aCallbackArgument,
    bool removeFromParent, const CacheEventFlags eventFlags) {
  // Get global locks to synchronize with failover thread.
  // TODO:  This should go into RegionGlobalLocks
  // The distMngrsLock is required before RegionGlobalLocks since failover
  // thread acquires distMngrsLock and then tries to acquire endpoints lock
  // which is already taken by RegionGlobalLocks here.
  DistManagersLockGuard _guard(m_cacheImpl->tcrConnectionManager());
  RegionGlobalLocks acquireLocks(this);

  // Fix for BUG:849, i.e Remove subscription on region before destroying the
  // region
  if (eventFlags == CacheEventFlags::LOCAL) {
    if (unregisterKeysBeforeDestroyRegion() != GF_NOERR) {
      LOG_DEBUG(
          "DEBUG :: LocalRegion::destroyRegionNoThrow UnregisteredKeys "
          "Failed");
    }
  }

  TryWriteGuard guard(m_rwLock, m_destroyPending);
  if (m_destroyPending) {
    if (eventFlags.isCacheClose()) {
      return GF_NOERR;
    } else {
      return GF_CACHE_REGION_DESTROYED_EXCEPTION;
    }
  }

  m_destroyPending = true;
  LOG_DEBUG("LocalRegion::destroyRegionNoThrow( ): set flag destroy-pending.");

  GfErrType err = GF_NOERR;

  //  do not invoke the writer for expiry or notification
  if (!eventFlags.isNotification() && !eventFlags.isEvictOrExpire()) {
    if (!invokeCacheWriterForRegionEvent(aCallbackArgument, eventFlags,
                                         BEFORE_REGION_DESTROY)) {
      //  do not let CacheWriter veto when this is Cache::close()
      if (!eventFlags.isCacheClose()) {
        LOG_FINE("Cache writer prevented region destroy");
        m_destroyPending = false;
        return GF_CACHEWRITER_ERROR;
      }
    }
    //  for the expiry case try the local destroy first and remote
    // destroy only if local destroy succeeds
    if (!eventFlags.isLocal()) {
      err = destroyRegionNoThrow_remote(aCallbackArgument);
      if (err != GF_NOERR) {
        m_destroyPending = false;
        return err;
      }
    }
  }

  LOG_FINE("Region %s is being destroyed", m_fullPath.c_str());
  {
    auto&& lock = m_subRegions.make_lock();

    for (const auto& kv : m_subRegions) {
      // TODO: remove unnecessary dynamic_cast by having m_subRegions hold
      // RegionInternal and invoke the destroy method in that
      if (auto subRegion =
              std::dynamic_pointer_cast<RegionInternal>(kv.second)) {
        // for subregions never remove from parent since that will cause
        // the region to be destroyed and SEGV; unbind_all takes care of that
        // Also don't send remote destroy message for sub-regions
        err = subRegion->destroyRegionNoThrow(
            aCallbackArgument, false, eventFlags | CacheEventFlags::LOCAL);
        //  for Cache::close() keep going as far as possible
        if (err != GF_NOERR && !eventFlags.isCacheClose()) {
          m_destroyPending = false;
          return err;
        }
      }
    }
  }
  m_subRegions.clear();

  //  for the expiry case try the local destroy first and remote
  // destroy only if local destroy succeeds
  if (eventFlags.isEvictOrExpire() && !eventFlags.isLocal()) {
    err = destroyRegionNoThrow_remote(aCallbackArgument);
    if (err != GF_NOERR) {
      m_destroyPending = false;
      return err;
    }
  }
  //  if we are not removing from parent then this is a proper
  // region close so invoke listener->close() also
  err = invokeCacheListenerForRegionEvent(aCallbackArgument, eventFlags,
                                          AFTER_REGION_DESTROY);

  release(true);
  if (m_regionAttributes.getCachingEnabled()) {
    _GEODE_SAFE_DELETE(m_entries);
  }

  if (removeFromParent) {
    if (m_parentRegion == nullptr) {
      m_cacheImpl->removeRegion(m_name.c_str());
    } else {
      LocalRegion* parent = dynamic_cast<LocalRegion*>(m_parentRegion.get());
      if (parent != nullptr) {
        parent->removeRegion(m_name);
        if (!eventFlags.isEvictOrExpire()) {
          parent->updateAccessAndModifiedTime(true);
        }
      }
    }
  }
  return err;
}

GfErrType LocalRegion::putLocal(const std::string& name, bool isCreate,
                                const std::shared_ptr<CacheableKey>& key,
                                const std::shared_ptr<Cacheable>& value,
                                std::shared_ptr<Cacheable>& oldValue,
                                bool cachingEnabled, int updateCount,
                                int destroyTracker,
                                std::shared_ptr<VersionTag> versionTag,
                                DataInput* delta,
                                std::shared_ptr<EventId> eventId) {
  GfErrType err = GF_NOERR;
  bool isUpdate = !isCreate;
  auto& cachePerfStats = m_cacheImpl->getCachePerfStats();

  if (cachingEnabled) {
    std::shared_ptr<MapEntryImpl> entry;
    LOG_DEBUG("%s: region [%s] putting key [%s], value [%s]", name.c_str(),
              getFullPath().c_str(), Utils::nullSafeToString(key).c_str(),
              Utils::nullSafeToString(value).c_str());
    if (isCreate) {
      err = m_entries->create(key, value, entry, oldValue, updateCount,
                              destroyTracker, versionTag);
    } else {
      err = m_entries->put(key, value, entry, oldValue, updateCount,
                           destroyTracker, versionTag, isUpdate, delta);
      if (err == GF_INVALID_DELTA) {
        cachePerfStats.incFailureOnDeltaReceived();
        // PXR: Get full object from server.
        std::shared_ptr<Cacheable>& newValue1 =
            const_cast<std::shared_ptr<Cacheable>&>(value);
        std::shared_ptr<VersionTag> versionTag1;
        err = getNoThrow_FullObject(eventId, newValue1, versionTag1);
        if (err == GF_NOERR && newValue1 != nullptr) {
          err = m_entries->put(
              key, newValue1, entry, oldValue, updateCount, destroyTracker,
              versionTag1 != nullptr ? versionTag1 : versionTag, isUpdate);
        }
      }
      if (delta != nullptr &&
          err == GF_NOERR) {  // Means that delta is on and there is no failure.
        cachePerfStats.incDeltaReceived();
      }
    }
    if (err != GF_NOERR) {
      return err;
    }
    LOG_DEBUG("%s: region [%s] %s key [%s], value [%s]", name.c_str(),
              getFullPath().c_str(), isUpdate ? "updated" : "created",
              Utils::nullSafeToString(key).c_str(),
              Utils::nullSafeToString(value).c_str());
    // entry/region expiration
    if (entryExpiryEnabled()) {
      if (isUpdate && entry->getExpProperties().task_scheduled()) {
        updateAccessAndModifiedTimeForEntry(entry, true);
      } else {
        registerEntryExpiryTask(entry);
      }
    }
    updateAccessAndModifiedTime(true);
  }
  // update the stats
  if (isUpdate) {
    m_regionStats->incPuts();
    cachePerfStats.incPuts();
  } else {
    if (cachingEnabled) {
      m_regionStats->setEntries(m_entries->size());
      cachePerfStats.incEntries(1);
    }
    m_regionStats->incCreates();
    cachePerfStats.incCreates();
  }
  return err;
}

std::vector<std::shared_ptr<CacheableKey>> LocalRegion::keys_internal() {
  std::vector<std::shared_ptr<CacheableKey>> keys;

  if (m_regionAttributes.getCachingEnabled()) {
    m_entries->getKeys(keys);
  }

  return keys;
}

void LocalRegion::entries_internal(
    std::vector<std::shared_ptr<RegionEntry>>& me, const bool recursive) {
  m_entries->getEntries(me);

  if (recursive == true) {
    auto&& lock = m_subRegions.make_lock();
    for (const auto& kv : m_subRegions) {
      if (auto subRegion = std::dynamic_pointer_cast<LocalRegion>(kv.second)) {
        subRegion->entries_internal(me, true);
      }
    }
  }
}

void LocalRegion::removeRegion(const std::string& name) {
  m_subRegions.erase(name);
}

bool LocalRegion::invokeCacheWriterForEntryEvent(
    const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<Cacheable>& oldValue,
    const std::shared_ptr<Cacheable>& newValue,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    CacheEventFlags eventFlags, EntryEventType type) {
  // Check if we have a local cache writer. If so, invoke and return.
  bool bCacheWriterReturn = true;
  if (m_writer != nullptr) {
    if (oldValue != nullptr && CacheableToken::isInvalid(oldValue)) {
      oldValue = nullptr;
    }
    EntryEvent event(shared_from_this(), key, oldValue, newValue,
                     aCallbackArgument, eventFlags.isNotification());
    const char* eventStr = "unknown";
    try {
      bool updateStats = true;
      /*Update the CacheWriter Stats*/
      int64_t sampleStartNanos = startStatOpTime();
      switch (type) {
        case BEFORE_UPDATE: {
          if (oldValue != nullptr) {
            eventStr = "beforeUpdate";
            bCacheWriterReturn = m_writer->beforeUpdate(event);
            break;
          }
          // if oldValue is nullptr then fall to BEFORE_CREATE case
          eventStr = "beforeCreate";
          bCacheWriterReturn = m_writer->beforeCreate(event);
          break;
        }
        case BEFORE_CREATE: {
          eventStr = "beforeCreate";
          bCacheWriterReturn = m_writer->beforeCreate(event);
          break;
        }
        case BEFORE_DESTROY: {
          eventStr = "beforeDestroy";
          bCacheWriterReturn = m_writer->beforeDestroy(event);
          break;
        }
        case BEFORE_INVALIDATE:
        case AFTER_CREATE:
        case AFTER_UPDATE:
        case AFTER_INVALIDATE:
        case AFTER_DESTROY: {
          updateStats = false;
          break;
        }
      }

      if (updateStats) {
        updateStatOpTime(m_regionStats->getStat(),
                         m_regionStats->getWriterCallTimeId(),
                         sampleStartNanos);
        m_regionStats->incWriterCallsCompleted();
      }

    } catch (const Exception& ex) {
      LOG_ERROR(std::string("Exception in CacheWriter::") + eventStr + ": " +
                ex.getName() + ": " + ex.getMessage());
      bCacheWriterReturn = false;
    } catch (...) {
      LOG_ERROR("Unknown exception in CacheWriter::%s", eventStr);
      bCacheWriterReturn = false;
    }
  }
  return bCacheWriterReturn;
}

bool LocalRegion::invokeCacheWriterForRegionEvent(
    const std::shared_ptr<Serializable>& aCallbackArgument,
    CacheEventFlags eventFlags, RegionEventType type) {
  // Check if we have a local cache writer. If so, invoke and return.
  bool bCacheWriterReturn = true;
  if (m_writer != nullptr) {
    RegionEvent event(shared_from_this(), aCallbackArgument,
                      eventFlags.isNotification());
    const char* eventStr = "unknown";
    try {
      bool updateStats = true;
      /*Update the CacheWriter Stats*/
      int64_t sampleStartNanos = startStatOpTime();
      switch (type) {
        case BEFORE_REGION_DESTROY: {
          eventStr = "beforeRegionDestroy";
          bCacheWriterReturn = m_writer->beforeRegionDestroy(event);
          break;
        }
        case BEFORE_REGION_CLEAR: {
          eventStr = "beforeRegionClear";
          bCacheWriterReturn = m_writer->beforeRegionClear(event);
          break;
        }
        case BEFORE_REGION_INVALIDATE:
        case AFTER_REGION_INVALIDATE:
        case AFTER_REGION_DESTROY:
        case AFTER_REGION_CLEAR: {
          updateStats = false;
          break;
        }
      }
      if (updateStats) {
        updateStatOpTime(m_regionStats->getStat(),
                         m_regionStats->getWriterCallTimeId(),
                         sampleStartNanos);
        m_regionStats->incWriterCallsCompleted();
      }
    } catch (const Exception& ex) {
      LOG_ERROR(std::string("Exception in CacheWriter::") + eventStr + ": " +
                ex.getName() + ": " + ex.getMessage());
      bCacheWriterReturn = false;
    } catch (...) {
      LOG_ERROR("Unknown exception in CacheWriter::%s", eventStr);
      bCacheWriterReturn = false;
    }
  }
  return bCacheWriterReturn;
}

GfErrType LocalRegion::invokeCacheListenerForEntryEvent(
    const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<Cacheable>& oldValue,
    const std::shared_ptr<Cacheable>& newValue,
    const std::shared_ptr<Serializable>& aCallbackArgument,
    CacheEventFlags eventFlags, EntryEventType type, bool isLocal) {
  GfErrType err = GF_NOERR;

  // Check if we have a local cache listener. If so, invoke and return.
  if (m_listener != nullptr) {
    if (oldValue != nullptr && CacheableToken::isInvalid(oldValue)) {
      oldValue = nullptr;
    }
    EntryEvent event(shared_from_this(), key, oldValue, newValue,
                     aCallbackArgument, eventFlags.isNotification());
    const char* eventStr = "unknown";
    try {
      bool updateStats = true;
      /*Update the CacheWriter Stats*/
      int64_t sampleStartNanos = startStatOpTime();
      switch (type) {
        case AFTER_UPDATE: {
          //  when CREATE is received from server for notification
          // then force an afterUpdate even if key is not present in cache.
          if (oldValue != nullptr || eventFlags.isNotificationUpdate() ||
              isLocal) {
            eventStr = "afterUpdate";
            m_listener->afterUpdate(event);
            break;
          }
          // if oldValue is nullptr then fall to AFTER_CREATE case
          eventStr = "afterCreate";
          m_listener->afterCreate(event);
          break;
        }
        case AFTER_CREATE: {
          eventStr = "afterCreate";
          m_listener->afterCreate(event);
          break;
        }
        case AFTER_DESTROY: {
          eventStr = "afterDestroy";
          m_listener->afterDestroy(event);
          break;
        }
        case AFTER_INVALIDATE: {
          eventStr = "afterInvalidate";
          m_listener->afterInvalidate(event);
          break;
        }
        case BEFORE_CREATE:
        case BEFORE_UPDATE:
        case BEFORE_INVALIDATE:
        case BEFORE_DESTROY: {
          updateStats = false;
          break;
        }
      }
      if (updateStats) {
        m_cacheImpl->getCachePerfStats().incListenerCalls();
        updateStatOpTime(m_regionStats->getStat(),
                         m_regionStats->getListenerCallTimeId(),
                         sampleStartNanos);
        m_regionStats->incListenerCallsCompleted();
      }
    } catch (const Exception& ex) {
      LOG_ERROR("Exception in CacheListener for key[%s]::%s: %s: %s",
                Utils::nullSafeToString(key).c_str(), eventStr,
                ex.getName().c_str(), ex.what());
      err = GF_CACHE_LISTENER_EXCEPTION;
    } catch (...) {
      LOG_ERROR("Unknown exception in CacheListener for key[%s]::%s",
                Utils::nullSafeToString(key).c_str(), eventStr);
      err = GF_CACHE_LISTENER_EXCEPTION;
    }
  }
  return err;
}

GfErrType LocalRegion::invokeCacheListenerForRegionEvent(
    const std::shared_ptr<Serializable>& aCallbackArgument,
    CacheEventFlags eventFlags, RegionEventType type) {
  GfErrType err = GF_NOERR;

  // Check if we have a local cache listener. If so, invoke and return.
  if (m_listener != nullptr) {
    RegionEvent event(shared_from_this(), aCallbackArgument,
                      eventFlags.isNotification());
    const char* eventStr = "unknown";
    try {
      bool updateStats = true;
      /*Update the CacheWriter Stats*/
      int64_t sampleStartNanos = Utils::startStatOpTime();
      switch (type) {
        case AFTER_REGION_DESTROY: {
          eventStr = "afterRegionDestroy";
          m_listener->afterRegionDestroy(event);
          m_cacheImpl->getCachePerfStats().incListenerCalls();
          if (eventFlags.isCacheClose()) {
            eventStr = "close";
            m_listener->close(*this);
            m_cacheImpl->getCachePerfStats().incListenerCalls();
          }
          break;
        }
        case AFTER_REGION_INVALIDATE: {
          eventStr = "afterRegionInvalidate";
          m_listener->afterRegionInvalidate(event);
          m_cacheImpl->getCachePerfStats().incListenerCalls();
          break;
        }
        case AFTER_REGION_CLEAR: {
          eventStr = "afterRegionClear";
          m_listener->afterRegionClear(event);
          break;
        }
        case BEFORE_REGION_INVALIDATE:
        case BEFORE_REGION_DESTROY:
        case BEFORE_REGION_CLEAR: {
          updateStats = false;
          break;
        }
      }
      if (updateStats) {
        updateStatOpTime(m_regionStats->getStat(),
                         m_regionStats->getListenerCallTimeId(),
                         sampleStartNanos);
        m_regionStats->incListenerCallsCompleted();
      }
    } catch (const Exception& ex) {
      LOG_ERROR("Exception in CacheListener::%s: %s: %s", eventStr,
                ex.getName().c_str(), ex.what());
      err = GF_CACHE_LISTENER_EXCEPTION;
    } catch (...) {
      LOG_ERROR("Unknown exception in CacheListener::%s", eventStr);
      err = GF_CACHE_LISTENER_EXCEPTION;
    }
  }
  return err;
}

// TODO:  pass current time instead of evaluating it twice, here
// and in region
void LocalRegion::updateAccessAndModifiedTimeForEntry(
    std::shared_ptr<MapEntryImpl>& ptr, bool modified) {
  // locking is not required since setters use atomic operations
  if (ptr != nullptr && entryExpiryEnabled()) {
    ExpEntryProperties& expProps = ptr->getExpProperties();
    auto now = std::chrono::steady_clock::now();
    std::string keyStr;
    if (Log::enabled(LogLevel::Debug)) {
      std::shared_ptr<CacheableKey> key;
      ptr->getKeyI(key);
      keyStr = Utils::nullSafeToString(key);
    }
    LOG_DEBUG("Setting last accessed time for key [%s] in region %s to %s",
              keyStr.c_str(), getFullPath().c_str(),
              to_string(now.time_since_epoch()).c_str());
    expProps.last_accessed(now);
    if (modified) {
      LOG_DEBUG("Setting last modified time for key [%s] in region %s to %s",
                keyStr.c_str(), getFullPath().c_str(),
                to_string(now.time_since_epoch()).c_str());
      expProps.last_modified(now);
    }
  }
}

uint32_t LocalRegion::adjustLruEntriesLimit(uint32_t limit) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::adjustLruEntriesLimit);

  auto attrs = m_regionAttributes;
  if (!attrs.getCachingEnabled()) return 0;
  bool hadlru = (attrs.getLruEntriesLimit() != 0);
  bool needslru = (limit != 0);
  if (hadlru != needslru) {
    throw IllegalStateException(
        "Cannot disable or enable LRU, can only adjust limit.");
  }
  uint32_t oldValue = attrs.getLruEntriesLimit();
  setLruEntriesLimit(limit);
  if (needslru) {
    // checked in AttributesMutator already to assert that LRU was enabled..
    LRUEntriesMap* lrumap = static_cast<LRUEntriesMap*>(m_entries);

    lrumap->adjustLimit(limit);
  }
  return oldValue;
}

ExpirationAction LocalRegion::adjustRegionExpiryAction(
    ExpirationAction action) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::adjustRegionExpiryAction);

  auto attrs = m_regionAttributes;
  bool hadExpiry = (getRegionExpiryDuration() > std::chrono::seconds::zero());
  if (!hadExpiry) {
    throw IllegalStateException(
        "Cannot change region ExpirationAction for region created without "
        "region expiry.");
  }
  ExpirationAction oldValue = getRegionExpiryAction();

  setRegionTimeToLiveExpirationAction(action);
  setRegionIdleTimeoutExpirationAction(action);
  // m_regionExpirationAction = action;

  return oldValue;
}

ExpirationAction LocalRegion::adjustEntryExpiryAction(ExpirationAction action) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::adjustEntryExpiryAction);

  auto attrs = m_regionAttributes;
  bool hadExpiry = (getEntryExpiryDuration() > std::chrono::seconds::zero());
  if (!hadExpiry) {
    throw IllegalStateException(
        "Cannot change entry ExpirationAction for region created without "
        "entry "
        "expiry.");
  }
  ExpirationAction oldValue = getEntryExpirationAction();

  setEntryTimeToLiveExpirationAction(action);
  setEntryIdleTimeoutExpirationAction(action);

  return oldValue;
}

std::chrono::seconds LocalRegion::adjustRegionExpiryDuration(
    const std::chrono::seconds& duration) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::adjustRegionExpiryDuration);

  bool hadExpiry = (getEntryExpiryDuration() > std::chrono::seconds::zero());
  if (!hadExpiry) {
    throw IllegalStateException(
        "Cannot change region  expiration duration for region created "
        "without "
        "region expiry.");
  }
  const auto& oldValue = getRegionExpiryDuration();

  setRegionTimeToLive(duration);
  setRegionIdleTimeout(duration);

  return oldValue;
}

std::chrono::seconds LocalRegion::adjustEntryExpiryDuration(
    const std::chrono::seconds& duration) {
  CHECK_DESTROY_PENDING(TryReadGuard, LocalRegion::adjustEntryExpiryDuration);

  bool hadExpiry = (getEntryExpiryDuration() > std::chrono::seconds::zero());
  if (!hadExpiry) {
    throw IllegalStateException(
        "Cannot change entry expiration duration for region created without "
        "entry expiry.");
  }
  auto oldValue = getEntryExpiryDuration();

  setEntryTimeToLive(duration);
  setEntryIdleTimeout(duration);

  return oldValue;
}

/** they used to public methods in hpp file */
bool LocalRegion::isStatisticsEnabled() {
  if (m_cacheImpl == nullptr) {
    return false;
  }
  return m_cacheImpl->getDistributedSystem()
      .getSystemProperties()
      .statisticsEnabled();
}

bool LocalRegion::useModifiedTimeForRegionExpiry() {
  const auto& region_ttl = m_regionAttributes.getRegionTimeToLive();
  if (region_ttl > std::chrono::seconds::zero()) {
    return true;
  } else {
    return false;
  }
}

bool LocalRegion::useModifiedTimeForEntryExpiry() {
  if (m_regionAttributes.getEntryTimeToLive() > std::chrono::seconds::zero()) {
    return true;
  } else {
    return false;
  }
}

bool LocalRegion::isEntryIdletimeEnabled() {
  if (m_regionAttributes.getCachingEnabled() &&
      m_regionAttributes.getEntryIdleTimeout() > std::chrono::seconds::zero()) {
    return true;
  } else {
    return false;
  }
}

ExpirationAction LocalRegion::getEntryExpirationAction() const {
  if (m_regionAttributes.getEntryTimeToLive() > std::chrono::seconds::zero()) {
    return m_regionAttributes.getEntryTimeToLiveAction();
  } else {
    return m_regionAttributes.getEntryIdleTimeoutAction();
  }
}

ExpirationAction LocalRegion::getRegionExpiryAction() const {
  const auto& region_ttl = m_regionAttributes.getRegionTimeToLive();
  if (region_ttl > std::chrono::seconds::zero()) {
    return m_regionAttributes.getRegionTimeToLiveAction();
  } else {
    return m_regionAttributes.getRegionIdleTimeoutAction();
  }
}

std::chrono::seconds LocalRegion::getRegionExpiryDuration() const {
  const auto& region_ttl = m_regionAttributes.getRegionTimeToLive();
  const auto& region_idle = m_regionAttributes.getRegionIdleTimeout();
  if (region_ttl > std::chrono::seconds::zero()) {
    return region_ttl;
  } else {
    return region_idle;
  }
}

std::chrono::seconds LocalRegion::getEntryExpiryDuration() const {
  const auto& entry_ttl = m_regionAttributes.getEntryTimeToLive();
  const auto& entry_idle = m_regionAttributes.getEntryIdleTimeout();

  if (entry_ttl > std::chrono::seconds::zero()) {
    return entry_ttl;
  } else {
    return entry_idle;
  }
}

/** methods to be overridden by derived classes*/
GfErrType LocalRegion::unregisterKeysBeforeDestroyRegion() { return GF_NOERR; }

GfErrType LocalRegion::getNoThrow_remote(const std::shared_ptr<CacheableKey>&,
                                         std::shared_ptr<Cacheable>&,
                                         const std::shared_ptr<Serializable>&,
                                         std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::putNoThrow_remote(const std::shared_ptr<CacheableKey>&,
                                         const std::shared_ptr<Cacheable>&,
                                         const std::shared_ptr<Serializable>&,
                                         std::shared_ptr<VersionTag>&, bool) {
  return GF_NOERR;
}

GfErrType LocalRegion::putAllNoThrow_remote(
    const HashMapOfCacheable&,
    std::shared_ptr<VersionedCacheableObjectPartList>&,
    std::chrono::milliseconds, const std::shared_ptr<Serializable>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::removeAllNoThrow_remote(
    const std::vector<std::shared_ptr<CacheableKey>>&,
    std::shared_ptr<VersionedCacheableObjectPartList>&,
    const std::shared_ptr<Serializable>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::createNoThrow_remote(
    const std::shared_ptr<CacheableKey>&, const std::shared_ptr<Cacheable>&,
    const std::shared_ptr<Serializable>&, std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::destroyNoThrow_remote(
    const std::shared_ptr<CacheableKey>&, const std::shared_ptr<Serializable>&,
    std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::removeNoThrow_remote(
    const std::shared_ptr<CacheableKey>&, const std::shared_ptr<Cacheable>&,
    const std::shared_ptr<Serializable>&, std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::removeNoThrowEX_remote(
    const std::shared_ptr<CacheableKey>&, const std::shared_ptr<Serializable>&,
    std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::invalidateNoThrow_remote(
    const std::shared_ptr<CacheableKey>&, const std::shared_ptr<Serializable>&,
    std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::getAllNoThrow_remote(
    const std::vector<std::shared_ptr<CacheableKey>>*,
    const std::shared_ptr<HashMapOfCacheable>&,
    const std::shared_ptr<HashMapOfException>&,
    const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&, bool,
    const std::shared_ptr<Serializable>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::invalidateRegionNoThrow_remote(
    const std::shared_ptr<Serializable>&) {
  return GF_NOERR;
}

GfErrType LocalRegion::destroyRegionNoThrow_remote(
    const std::shared_ptr<Serializable>&) {
  return GF_NOERR;
}

void LocalRegion::adjustCacheListener(
    const std::shared_ptr<CacheListener>& aListener) {
  WriteGuard guard(m_rwLock);
  setCacheListener(aListener);
  m_listener = aListener;
}

void LocalRegion::adjustCacheListener(const std::string& lib,
                                      const std::string& func) {
  WriteGuard guard(m_rwLock);
  setCacheListener(lib, func);
  m_listener = m_regionAttributes.getCacheListener();
}

void LocalRegion::adjustCacheLoader(
    const std::shared_ptr<CacheLoader>& aLoader) {
  WriteGuard guard(m_rwLock);
  setCacheLoader(aLoader);
  m_loader = aLoader;
}

void LocalRegion::adjustCacheLoader(const std::string& lib,
                                    const std::string& func) {
  WriteGuard guard(m_rwLock);
  setCacheLoader(lib, func);
  m_loader = m_regionAttributes.getCacheLoader();
}

void LocalRegion::adjustCacheWriter(
    const std::shared_ptr<CacheWriter>& aWriter) {
  WriteGuard guard(m_rwLock);
  setCacheWriter(aWriter);
  m_writer = aWriter;
}

void LocalRegion::adjustCacheWriter(const std::string& lib,
                                    const std::string& func) {
  WriteGuard guard(m_rwLock);
  setCacheWriter(lib, func);
  m_writer = m_regionAttributes.getCacheWriter();
}

void LocalRegion::evict(float percentage) {
  TryReadGuard guard(m_rwLock, m_destroyPending);
  if (m_released || m_destroyPending) return;
  if (m_entries != nullptr) {
    int32_t size = m_entries->size();
    int32_t entriesToEvict = static_cast<int32_t>(percentage * size);
    // only invoked from EvictionController so static_cast is always safe
    LRUEntriesMap* lruMap = static_cast<LRUEntriesMap*>(m_entries);
    LOG_INFO("Evicting %d entries. Current entry count is %d", entriesToEvict,
             size);
    lruMap->processLRU(entriesToEvict);
  }
}
void LocalRegion::invokeAfterAllEndPointDisconnected() {
  if (m_listener != nullptr) {
    int64_t sampleStartNanos = startStatOpTime();
    try {
      m_listener->afterRegionDisconnected(*this);
    } catch (const Exception& ex) {
      LOG_ERROR("Exception in CacheListener::afterRegionDisconnected: %s: %s",
                ex.getName().c_str(), ex.what());
    } catch (...) {
      LOG_ERROR("Unknown exception in CacheListener::afterRegionDisconnected");
    }
    updateStatOpTime(m_regionStats->getStat(),
                     m_regionStats->getListenerCallTimeId(), sampleStartNanos);
    m_regionStats->incListenerCallsCompleted();
  }
}

GfErrType LocalRegion::getNoThrow_FullObject(std::shared_ptr<EventId>,
                                             std::shared_ptr<Cacheable>&,
                                             std::shared_ptr<VersionTag>&) {
  return GF_NOERR;
}

std::shared_ptr<Cacheable> LocalRegion::handleReplay(
    GfErrType& err, std::shared_ptr<Cacheable> value) const {
  if (err == GF_TRANSACTION_DATA_REBALANCED_EXCEPTION ||
      err == GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION) {
    bool isRollBack = (err == GF_TRANSACTION_DATA_REBALANCED_EXCEPTION);
    auto txState = getTXState();
    if (!txState) {
      GfErrTypeThrowException("TXState is nullptr",
                              GF_CACHE_ILLEGAL_STATE_EXCEPTION);
      throw "";  // never reached
    }

    auto ret = txState->replay(isRollBack);
    err = GF_NOERR;
    return ret;
  }

  return value;
}
std::shared_ptr<TombstoneList> LocalRegion::getTombstoneList() {
  return m_tombstoneList;
}

int64_t LocalRegion::startStatOpTime() {
  return m_enableTimeStatistics ? Utils::startStatOpTime() : 0;
}
void LocalRegion::updateStatOpTime(Statistics* statistics, int32_t statId,
                                   int64_t start) {
  if (m_enableTimeStatistics) {
    Utils::updateStatOpTime(statistics, statId, start);
  }
}

void LocalRegion::acquireGlobals(bool) {}

void LocalRegion::releaseGlobals(bool) {}

void LocalRegion::clearKeysOfInterest(
    const std::unordered_map<std::shared_ptr<CacheableKey>,
                             InterestResultPolicy>& interest_list) {
  if (m_entries->empty()) {
    return;
  }

  for (const auto& kv : interest_list) {
    auto err = localDestroyNoCallbacks(kv.first);
    // It could happen that interest was registered for a key for which
    // there is not an entry right now
    if (err != GF_CACHE_ENTRY_NOT_FOUND) {
      throwExceptionIfError("LocalRegion::clearKeysOfInterest", err);
    }
  }
}

void LocalRegion::clearKeysOfInterestRegex(const std::string& pattern) {
  if (m_entries->empty()) {
    return;
  }

  std::regex expression{pattern};
  for (const auto& key : keys()) {
    if (std::regex_search(key->toString(), expression)) {
      auto err = localDestroyNoCallbacks(key);
      if (err != GF_CACHE_ENTRY_NOT_FOUND) {
        throwExceptionIfError("LocalRegion::clearKeysOfInterest", err);
      }
    }
  }
}

void LocalRegion::clearKeysOfInterestRegex(
    const std::unordered_map<std::string, InterestResultPolicy>&
        interest_list) {
  if (m_entries->empty()) {
    return;
  }

  static const std::string ALL_KEYS_REGEX = ".*";
  for (const auto& kv : interest_list) {
    const auto& regex = kv.first;
    if (regex == ALL_KEYS_REGEX) {
      localClear();
      break;
    } else {
      clearKeysOfInterestRegex(kv.first);
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
