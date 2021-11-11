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

#pragma once

#ifndef GEODE_PUTALLPARTIALRESULT_H_
#define GEODE_PUTALLPARTIALRESULT_H_

#include <mutex>

#include <boost/thread/shared_mutex.hpp>

#include <geode/CacheableString.hpp>
#include <geode/Serializable.hpp>

#include "VersionedCacheableObjectPartList.hpp"

namespace apache {
namespace geode {
namespace client {

class PutAllPartialResult final : public Serializable {
 private:
  std::shared_ptr<VersionedCacheableObjectPartList> m_succeededKeys;
  std::shared_ptr<CacheableKey> m_firstFailedKey;
  std::shared_ptr<Exception> m_firstCauseOfFailure;
  int32_t m_totalMapSize;
  boost::shared_mutex mutex_;

 public:
  PutAllPartialResult(int totalMapSize, std::recursive_mutex& responseLock);
  ~PutAllPartialResult() noexcept final {}

  void setTotalMapSize(int totalMapSize) { m_totalMapSize = totalMapSize; }

  // Add all succeededKeys and firstfailedKey.
  // Before calling this, we must read PutAllPartialResultServerException and
  // formulate obj of type PutAllPartialResult.
  void consolidate(std::shared_ptr<PutAllPartialResult> other);

  std::shared_ptr<Exception> getFailure() { return m_firstCauseOfFailure; }

  void addKeysAndVersions(
      std::shared_ptr<VersionedCacheableObjectPartList> keysAndVersion);

  void addKeys(
      std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_keys);

  void saveFailedKey(std::shared_ptr<CacheableKey> key,
                     std::shared_ptr<Exception> cause) {
    if (key == nullptr) {
      return;
    }
    // TODO:: Do we need to handle server cancelException.
    if (m_firstFailedKey == nullptr /*|| cause instanceof CaccelException */) {
      m_firstFailedKey = key;
      m_firstCauseOfFailure = cause;
    }
  }

  std::shared_ptr<VersionedCacheableObjectPartList>
  getSucceededKeysAndVersions();

  // Returns the first key that failed
  std::shared_ptr<CacheableKey> getFirstFailedKey() { return m_firstFailedKey; }

  // Returns there's failedKeys
  bool hasFailure() { return m_firstFailedKey != nullptr; }

  // Returns there's saved succeed keys
  bool hasSucceededKeys();

  virtual std::string toString() const final {
    auto asString = std::string("PutAllPartialResult: [ Key =") +
                    m_firstFailedKey->toString() + "]";

    if (m_totalMapSize > 0) {
      // TODO:: impl. CacheableObjectPartList.size();
      int failedKeyNum = m_totalMapSize - m_succeededKeys->size();
      if (failedKeyNum > 0) {
        asString += "The putAll operation failed to put " +
                    std::to_string(failedKeyNum) + " out of " +
                    std::to_string(m_totalMapSize) + " entries ";
      } else {
        asString += "The putAll operation sucessfully put " +
                    std::to_string(m_succeededKeys->size()) + " out of " +
                    std::to_string(m_totalMapSize) + " entries ";
      }
    }

    return asString;
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PUTALLPARTIALRESULT_H_
