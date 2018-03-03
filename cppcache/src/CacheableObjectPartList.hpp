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

#ifndef GEODE_CACHEABLEOBJECTPARTLIST_H_
#define GEODE_CACHEABLEOBJECTPARTLIST_H_

#include <vector>

#include <geode/internal/geode_globals.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>
#include <geode/Serializable.hpp>

#include "MapWithLock.hpp"
#include "HashMapOfException.hpp"
#include "GeodeTypeIdsImpl.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

class ThinClientRegion;

/**
 * Implement an immutable list of object parts that encapsulates an object,
 * a raw byte array or a java exception object. Optionally can also store
 * the keys corresponding to those objects. This is used for reading the
 * results of a GET_ALL request or initial values of register interest.
 *
 * Also see GetAll.ObjectPartList on java side.
 *
 *
 */
class CacheableObjectPartList : public DataSerializableFixedId {
 protected:
  const std::vector<std::shared_ptr<CacheableKey>> * m_keys;
  uint32_t* m_keysOffset;
  std::shared_ptr<HashMapOfCacheable> m_values;
  std::shared_ptr<HashMapOfException> m_exceptions;
  std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> m_resultKeys;
  ThinClientRegion* m_region;
  MapOfUpdateCounters* m_updateCountMap;
  int32_t m_destroyTracker;
  bool m_addToLocalCache;

  inline CacheableObjectPartList()
      : m_keys(nullptr),
        m_keysOffset(nullptr),
        m_values(nullptr),
        m_exceptions(nullptr),
        m_resultKeys(nullptr),
        m_region(nullptr),
        m_updateCountMap(nullptr),
        m_destroyTracker(0),
        m_addToLocalCache(false) {}

  inline CacheableObjectPartList(ThinClientRegion* region)
      : m_keys(nullptr),
        m_keysOffset(nullptr),
        m_values(nullptr),
        m_exceptions(nullptr),
        m_resultKeys(nullptr),
        m_region(region),
        m_updateCountMap(nullptr),
        m_destroyTracker(0),
        m_addToLocalCache(false) {}

  // never implemented.
  CacheableObjectPartList& operator=(const CacheableObjectPartList& other);
  CacheableObjectPartList(const CacheableObjectPartList& other);

 public:
  /**
   * @brief constructor given the list of keys and a map of values,
   *        map of exceptions and region to populate with the values
   *        obtained in fromData
   */
  CacheableObjectPartList(
      const std::vector<std::shared_ptr<CacheableKey>>* keys,
      uint32_t* keysOffset, const std::shared_ptr<HashMapOfCacheable>& values,
      const std::shared_ptr<HashMapOfException>& exceptions,
      const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
          resultKeys,
      ThinClientRegion* region, MapOfUpdateCounters* trackerMap,
      int32_t destroyTracker, bool addToLocalCache)
      : m_keys(keys),
        m_keysOffset(keysOffset),
        m_values(values),
        m_exceptions(exceptions),
        m_resultKeys(resultKeys),
        m_region(region),
        m_updateCountMap(trackerMap),
        m_destroyTracker(destroyTracker),
        m_addToLocalCache(addToLocalCache) {}

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  int32_t getDSFID() const override {
    return GeodeTypeIdsImpl::CacheableObjectPartList;
  }

  size_t objectSize() const override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEOBJECTPARTLIST_H_
