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

#include "CreateAction.hpp"

#include "LocalRegion.hpp"
#include "TSSTXStateWrapper.hpp"
#include "Utils.hpp"
#include "VersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

CreateAction::CreateAction(LocalRegion& region)
    : region_{region}, txState_{TSSTXStateWrapper::get().getTXState()} {}

void CreateAction::getCallbackOldValue(bool,
                                       const std::shared_ptr<CacheableKey>&,
                                       std::shared_ptr<MapEntryImpl>&,
                                       std::shared_ptr<Serializable>&) const {}

GfErrType CreateAction::remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                                     const std::shared_ptr<Serializable>& value,
                                     const std::shared_ptr<Serializable>& cbArg,
                                     std::shared_ptr<Serializable>&,
                                     std::shared_ptr<VersionTag>& versionTag) {
  return region_.remoteCreate(key, value, cbArg, versionTag);
}

GfErrType CreateAction::localUpdate(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Serializable>& value,
                                    std::shared_ptr<Serializable>& oldValue,
                                    bool cachingEnabled, const CacheEventFlags,
                                    int updateCount,
                                    std::shared_ptr<VersionTag> versionTag,
                                    DataInput*, std::shared_ptr<EventId>,
                                    bool) {
  return region_.putLocal(name(), true, key, value, oldValue, cachingEnabled,
                          updateCount, 0, versionTag);
}

GfErrType CreateAction::checkArgs(const std::shared_ptr<CacheableKey>& key,
                                  const std::shared_ptr<Serializable>&,
                                  DataInput*) {
  if (!key) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }

  return GF_NOERR;
}

void CreateAction::logCacheWriterFailure(
    const std::shared_ptr<CacheableKey>& key,
    const std::shared_ptr<Serializable>&) {
  LOGFINER("Cache writer vetoed create for key %s",
           Utils::nullSafeToString(key).c_str());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
