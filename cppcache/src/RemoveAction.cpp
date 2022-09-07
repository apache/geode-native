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

#include "RemoveAction.hpp"

#include <geode/internal/DataSerializablePrimitive.hpp>

#include "CacheImpl.hpp"
#include "LocalRegion.hpp"
#include "SerializableHelper.hpp"
#include "TSSTXStateWrapper.hpp"
#include "Utils.hpp"
#include "VersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializablePrimitive;

RemoveAction::RemoveAction(LocalRegion& region, bool allowNull)
    : region_(region),
      response_(GF_ENOENT),
      txState_{TSSTXStateWrapper::get().getTXState()},
      allowNull_{allowNull} {}

void RemoveAction::getCallbackOldValue(
    bool cachingEnabled, const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<MapEntryImpl>& entry,
    std::shared_ptr<Cacheable>& oldValue) const {
  if (cachingEnabled) {
    region_.getEntryMap()->getEntry(key, entry, oldValue);
  }
}

GfErrType RemoveAction::remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                                     const std::shared_ptr<Cacheable>& value,
                                     const std::shared_ptr<Cacheable>& cbArg,
                                     std::shared_ptr<Cacheable>&,
                                     std::shared_ptr<VersionTag>& versionTag) {
  // propagate the remove to remote server, if any
  std::shared_ptr<Cacheable> oldValue;
  GfErrType err = GF_NOERR;
  if (!allowNull_ && region_.getAttributes().getCachingEnabled()) {
    region_.getEntry(key, oldValue);
    if (oldValue && value) {
      if (!serializedEqualTo(oldValue, value)) {
        err = GF_ENOENT;
        return err;
      }
    } else if (!oldValue || CacheableToken::isInvalid(oldValue)) {
      response_ = region_.remoteRemove(key, value, cbArg, versionTag);
      return response_;
    } else if (oldValue && !value) {
      return GF_ENOENT;
    }
  }

  if (allowNull_) {
    response_ = region_.remoteRemoveEx(key, cbArg, versionTag);
  } else {
    response_ = region_.remoteRemove(key, value, cbArg, versionTag);
  }

  LOGDEBUG("RemoveAction::remoteUpdate server response: %d", response_);
  return response_;
}

GfErrType RemoveAction::localUpdate(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Cacheable>& value,
                                    std::shared_ptr<Cacheable>& oldValue,
                                    bool cachingEnabled,
                                    const CacheEventFlags eventFlags,
                                    int updateCount,
                                    std::shared_ptr<VersionTag> versionTag,
                                    DataInput*, std::shared_ptr<EventId>,
                                    bool afterRemote) {
  std::shared_ptr<Cacheable> fetchedValue;
  GfErrType err = GF_NOERR;
  if (!allowNull_ && cachingEnabled) {
    region_.getEntry(key, fetchedValue);
    if (fetchedValue && value) {
      if (!serializedEqualTo(fetchedValue, value)) {
        return GF_ENOENT;
      }
    } else if (!value &&
               (!CacheableToken::isInvalid(fetchedValue) || !fetchedValue)) {
      err = (response_ == GF_NOERR && !fetchedValue) ? GF_NOERR : GF_ENOENT;
      if (updateCount >= 0 &&
          !region_.getAttributes().getConcurrencyChecksEnabled()) {
        // This means server has deleted an entry & same entry has been
        // destroyed locally
        // So call removeTrackerForEntry to remove key that
        // was added in the  map during addTrackerForEntry call.
        region_.getEntryMap()->removeTrackerForEntry(key);
      }

      return err;
    } else if (!fetchedValue && value && response_ != GF_NOERR) {
      err = GF_ENOENT;
      return err;
    }
  }

  auto& cachePerfStats = region_.getCacheImpl()->getCachePerfStats();
  if (cachingEnabled) {
    //  for notification invoke the listener even if the key does
    // not exist locally
    LOGDEBUG("Region::remove: region [%s] removing key [%s]",
             region_.getFullPath().c_str(),
             Utils::nullSafeToString(key).c_str());

    std::shared_ptr<MapEntryImpl> entry;
    if ((err = region_.getEntryMap()->remove(key, oldValue, entry, updateCount,
                                             versionTag, afterRemote)) !=
        GF_NOERR) {
      if (eventFlags.isNotification()) {
        LOGDEBUG(
            "Region::remove: region [%s] remove key [%s] for "
            "notification having value [%s] failed with %d",
            region_.getFullPath().c_str(), Utils::nullSafeToString(key).c_str(),
            Utils::nullSafeToString(oldValue).c_str(), err);
        err = GF_NOERR;
      }

      return err;
    }

    if (oldValue) {
      LOGDEBUG(
          "Region::remove: region [%s] removed key [%s] having "
          "value [%s]",
          region_.getFullPath().c_str(), Utils::nullSafeToString(key).c_str(),
          Utils::nullSafeToString(oldValue).c_str());

      // any cleanup required for the entry (e.g. removing from LRU list)
      if (entry) {
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

bool RemoveAction::serializedEqualTo(const std::shared_ptr<Cacheable>& lhs,
                                     const std::shared_ptr<Cacheable>& rhs) {
  auto&& cache = *region_.getCacheImpl();

  if (const auto primitive =
          std::dynamic_pointer_cast<DataSerializablePrimitive>(lhs)) {
    return SerializableHelper<DataSerializablePrimitive>{}.equalTo(
        cache, primitive,
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
    throw UnsupportedOperationException("Serialization type not implemented.");
  }
}

GfErrType RemoveAction::checkArgs(const std::shared_ptr<CacheableKey>& key,
                                  const std::shared_ptr<Cacheable>&,
                                  DataInput*) {
  if (!key) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }

  return GF_NOERR;
}

void RemoveAction::logCacheWriterFailure(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Cacheable>&) {
  LOGFINER("Cache writer vetoed remove for key %s",
           Utils::nullSafeToString(key).c_str());
}

RemoveActionEx::RemoveActionEx(LocalRegion& region)
    : RemoveAction(region, true) {}

}  // namespace client
}  // namespace geode
}  // namespace apache
