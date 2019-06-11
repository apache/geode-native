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

#ifndef GEODE_VERSIONEDCACHEABLEOBJECTPARTLIST_H_
#define GEODE_VERSIONEDCACHEABLEOBJECTPARTLIST_H_

#include <mutex>
#include <vector>

#include "CacheableObjectPartList.hpp"
#include "VersionTag.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientRegion;

/**
 * Implement an immutable list of object parts that encapsulates an object,
 * a raw byte array or a java exception object. Optionally can also store
 * the keys corresponding to those objects. This is used for reading the
 * results of a GET_ALL/PUTALL request or initial values of register interest.
 *
 * Also see GetAll.ObjectPartList on java side.
 *
 *
 */
class VersionedCacheableObjectPartList : public CacheableObjectPartList {
 private:
  bool m_regionIsVersioned;
  bool m_serializeValues;
  bool m_hasTags;
  bool m_hasKeys;
  std::vector<std::shared_ptr<VersionTag>> m_versionTags;
  std::vector<uint8_t> m_byteArray;
  uint16_t m_endpointMemId;
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_tempKeys;
  std::recursive_mutex& m_responseLock;

  static const uint8_t FLAG_NULL_TAG;
  static const uint8_t FLAG_FULL_TAG;
  static const uint8_t FLAG_TAG_WITH_NEW_ID;
  static const uint8_t FLAG_TAG_WITH_NUMBER_ID;

  void readObjectPart(int32_t index, DataInput& input,
                      std::shared_ptr<CacheableKey> keyPtr);
  // never implemented.
  VersionedCacheableObjectPartList& operator=(
      const VersionedCacheableObjectPartList& other);
  VersionedCacheableObjectPartList(
      const VersionedCacheableObjectPartList& other);

 public:
  VersionedCacheableObjectPartList(
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      uint32_t* keysOffset, const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
          resultKeys,
      ThinClientRegion* region, MapOfUpdateCounters* trackerMap,
      int32_t destroyTracker, bool addToLocalCache, uint16_t m_dsmemId,
      std::recursive_mutex& responseLock);

  VersionedCacheableObjectPartList(
      std::vector<std::shared_ptr<CacheableKey>>* keys, int32_t totalMapSize,
      std::recursive_mutex& responseLock);

  VersionedCacheableObjectPartList(ThinClientRegion* region, uint16_t dsmemId,
                                   std::recursive_mutex& responseLock);

  VersionedCacheableObjectPartList(
      std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> keys,
      std::recursive_mutex& responseLock);

  VersionedCacheableObjectPartList(
      ThinClientRegion* region,
      std::vector<std::shared_ptr<CacheableKey>>* keys,
      std::recursive_mutex& responseLock);

  VersionedCacheableObjectPartList(ThinClientRegion* region,
                                   std::recursive_mutex& responseLock);

  inline uint16_t getEndpointMemId();

  std::vector<std::shared_ptr<VersionTag>>& getVersionedTagptr();

  void setVersionedTagptr(
      std::vector<std::shared_ptr<VersionTag>>& versionTags);

  int getVersionedTagsize() const;

  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>
  getSucceededKeys();

  VersionedCacheableObjectPartList(uint16_t endpointMemId,
                                   std::recursive_mutex& responseLock);

  void addAll(std::shared_ptr<VersionedCacheableObjectPartList> other);

  int size();

  void addAllKeys(
      std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> keySet);

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  DSFid getDSFID() const override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_VERSIONEDCACHEABLEOBJECTPARTLIST_H_
