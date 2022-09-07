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

#include "DestroyAction.hpp"

#include "CacheImpl.hpp"
#include "LocalRegion.hpp"
#include "TSSTXStateWrapper.hpp"
#include "Utils.hpp"
#include "VersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

DestroyAction::DestroyAction(LocalRegion& region)
    : region_{region}, txState_{TSSTXStateWrapper::get().getTXState()} {}

void DestroyAction::getCallbackOldValue(
    bool cachingEnabled, const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<MapEntryImpl>& entry,
    std::shared_ptr<Cacheable>& oldValue) const {
  if (cachingEnabled) {
    region_.getEntryMap()->getEntry(key, entry, oldValue);
  }
}

GfErrType DestroyAction::remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                                      const std::shared_ptr<Cacheable>&,
                                      const std::shared_ptr<Cacheable>& cbArg,
                                      std::shared_ptr<Cacheable>&,
                                      std::shared_ptr<VersionTag>& versionTag) {
  return region_.remoteDestroy(key, cbArg, versionTag);
}

GfErrType DestroyAction::localUpdate(
    const std::shared_ptr<CacheableKey>& key, const std::shared_ptr<Cacheable>&,
    std::shared_ptr<Cacheable>& oldValue, bool cachingEnabled,
    const CacheEventFlags eventFlags, int updateCount,
    std::shared_ptr<VersionTag> versionTag, DataInput*,
    std::shared_ptr<EventId>, bool afterRemote) {
  auto& cachePerfStats = region_.getCacheImpl()->getCachePerfStats();

  if (cachingEnabled) {
    std::shared_ptr<MapEntryImpl> entry;
    //  for notification invoke the listener even if the key does
    // not exist locally
    GfErrType err;
    LOGDEBUG("Region::destroy: region [%s] destroying key [%s]",
             region_.getFullPath().c_str(),
             Utils::nullSafeToString(key).c_str());
    if ((err = region_.getEntryMap()->remove(key, oldValue, entry, updateCount,
                                             versionTag, afterRemote)) !=
        GF_NOERR) {
      if (eventFlags.isNotification()) {
        LOGDEBUG(
            "Region::destroy: region [%s] destroy key [%s] for "
            "notification having value [%s] failed with %d",
            region_.getFullPath().c_str(), Utils::nullSafeToString(key).c_str(),
            Utils::nullSafeToString(oldValue).c_str(), err);
        err = GF_NOERR;
      }

      return err;
    }

    if (oldValue != nullptr) {
      LOGDEBUG(
          "Region::destroy: region [%s] destroyed key [%s] having "
          "value [%s]",
          region_.getFullPath().c_str(), Utils::nullSafeToString(key).c_str(),
          Utils::nullSafeToString(oldValue).c_str());
      // any cleanup required for the entry (e.g. removing from LRU list)
      if (entry != nullptr) {
        entry->cleanup(eventFlags);
      }
      // entry/region expiration
      if (!eventFlags.isEvictOrExpire()) {
        region_.updateAccessAndModifiedTime(true);
      }
      // update the stats
      region_.getRegionStats()->setEntries(region_.getEntryMap()->size());
      cachePerfStats.incEntries(-1);
    }
  }

  // update the stats
  region_.getRegionStats()->incDestroys();
  cachePerfStats.incDestroys();
  return GF_NOERR;
}

GfErrType DestroyAction::checkArgs(const std::shared_ptr<CacheableKey>& key,
                                   const std::shared_ptr<Cacheable>&,
                                   DataInput*) {
  if (!key) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }

  return GF_NOERR;
}

void DestroyAction::logCacheWriterFailure(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>&) {
  LOGFINER("Cache writer vetoed destroy for key %s",
           Utils::nullSafeToString(key).c_str());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
