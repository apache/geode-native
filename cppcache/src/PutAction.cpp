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

#include "PutAction.hpp"

#include "LocalRegion.hpp"
#include "TSSTXStateWrapper.hpp"
#include "Utils.hpp"
#include "VersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

PutAction::PutAction(LocalRegion& region)
    : region_{region}, txState_{TSSTXStateWrapper::get().getTXState()} {}

void PutAction::getCallbackOldValue(
    bool cachingEnabled, const std::shared_ptr<CacheableKey>& key,
    std::shared_ptr<MapEntryImpl>& entry,
    std::shared_ptr<Serializable>& oldValue) const {
  if (cachingEnabled) {
    region_.getEntryMap()->getEntry(key, entry, oldValue);
  }
}

GfErrType PutAction::remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                                  const std::shared_ptr<Serializable>& value,
                                  const std::shared_ptr<Serializable>& cbArg,
                                  std::shared_ptr<Serializable>& retVal,
                                  std::shared_ptr<VersionTag>& versionTag) {
  return region_.remotePut(key, value, cbArg, versionTag, retVal,
                           EventOperation::UPDATE);
}

GfErrType PutAction::localUpdate(const std::shared_ptr<CacheableKey>& key,
                                 const std::shared_ptr<Serializable>& value,
                                 std::shared_ptr<Serializable>& oldValue,
                                 bool cachingEnabled, const CacheEventFlags,
                                 int updateCount,
                                 std::shared_ptr<VersionTag> versionTag,
                                 DataInput* delta,
                                 std::shared_ptr<EventId> eventId, bool) {
  return region_.putLocal(name(), false, key, value, oldValue, cachingEnabled,
                          updateCount, 0, versionTag, delta, eventId);
}

void PutAction::logCacheWriterFailure(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& oldValue) {
  bool isUpdate = (oldValue != nullptr);
  LOGFINER("Cache writer vetoed %s for key %s",
           (isUpdate ? "update" : "create"),
           Utils::nullSafeToString(key).c_str());
}

GfErrType PutAction::checkArgs(const std::shared_ptr<CacheableKey>& key,
                               const std::shared_ptr<Serializable>& value,
                               DataInput* delta) {
  if (key == nullptr || (value == nullptr && delta == nullptr)) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }

  return GF_NOERR;
}

PutActionTx::PutActionTx(LocalRegion& region) : PutAction{region} {}

GfErrType PutActionTx::checkArgs(const std::shared_ptr<CacheableKey>& key,
                                 const std::shared_ptr<Serializable>&,
                                 DataInput*) {
  if (key == nullptr) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }

  return GF_NOERR;
}

PutIfAbsentAction::PutIfAbsentAction(LocalRegion& region)
    : PutAction{region}, absent_{true} {}

GfErrType PutIfAbsentAction::remoteUpdate(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& value,
    const std::shared_ptr<Serializable>& cbArg,
    std::shared_ptr<Serializable>& retVal,
    std::shared_ptr<VersionTag>& versionTag) {
  auto err = region_.remotePut(key, value, cbArg, versionTag, retVal,
                               EventOperation::PUT_IF_ABSENT);
  if (err == GF_NOERR) {
    absent_ = retVal == nullptr;
  }

  return err;
}

GfErrType PutIfAbsentAction::localUpdate(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>& value,
    std::shared_ptr<Serializable>& oldValue, bool cachingEnabled,
    const CacheEventFlags flags, int updateCount,
    std::shared_ptr<VersionTag> versionTag, DataInput* delta,
    std::shared_ptr<EventId> eventId, bool afterRemote) {
  if (!absent_) {
    return GF_NOERR;
  }

  return PutAction::localUpdate(key, value, oldValue, cachingEnabled, flags,
                                updateCount, versionTag, delta, eventId,
                                afterRemote);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
