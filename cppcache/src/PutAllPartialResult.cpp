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
#include "PutAllPartialResult.hpp"

#include <boost/thread/lock_types.hpp>

namespace apache {
namespace geode {
namespace client {

PutAllPartialResult::PutAllPartialResult(int totalMapSize,
                                         std::recursive_mutex& responseLock) {
  m_succeededKeys = std::make_shared<VersionedCacheableObjectPartList>(
      std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>(),
      responseLock);
  m_totalMapSize = totalMapSize;
}

// Add all succeededKeys and firstfailedKey.
// Before calling this, we must read PutAllPartialResultServerException and
// formulate obj of type PutAllPartialResult.
void PutAllPartialResult::consolidate(
    std::shared_ptr<PutAllPartialResult> other) {
  {
    boost::unique_lock<decltype(mutex_)> guard{mutex_};
    m_succeededKeys->addAll(other->getSucceededKeysAndVersions());
  }
  saveFailedKey(other->m_firstFailedKey, other->m_firstCauseOfFailure);
}

void PutAllPartialResult::addKeysAndVersions(
    std::shared_ptr<VersionedCacheableObjectPartList> keysAndVersion) {
  this->m_succeededKeys->addAll(keysAndVersion);
}

void PutAllPartialResult::addKeys(
    std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_keys) {
  {
    boost::unique_lock<decltype(mutex_)> guard{mutex_};
    if (m_succeededKeys->getVersionedTagsize() > 0) {
      throw IllegalStateException(
          "attempt to store versionless keys when there are already versioned "
          "results");
    }
    this->m_succeededKeys->addAllKeys(m_keys);
  }
}
std::shared_ptr<VersionedCacheableObjectPartList>
PutAllPartialResult::getSucceededKeysAndVersions() {
  return m_succeededKeys;
}

bool PutAllPartialResult::hasSucceededKeys() {
  return this->m_succeededKeys->size() > 0;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
