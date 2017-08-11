#pragma once

#ifndef GEODE_VERSIONEDCACHEABLEOBJECTPARTLIST_H_
#define GEODE_VERSIONEDCACHEABLEOBJECTPARTLIST_H_

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

/*
#include <geode/geode_globals.hpp>
#include <geode/geode_types.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>
#include <geode/Cacheable.hpp>
#include <geode/VectorT.hpp>
#include <geode/HashMapT.hpp>
#include "MapWithLock.hpp"
*/
#include "CacheableObjectPartList.hpp"
#include "VersionTag.hpp"
//#include "DiskVersionTag.hpp"
#include <ace/Task.h>
#include <vector>

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
 * results of a GET_ALL/PUTALL request or initial values of register interest.
 *
 * Also see GetAll.ObjectPartList on java side.
 *
 *
 */

class VersionedCacheableObjectPartList;
typedef std::shared_ptr<VersionedCacheableObjectPartList>
    VersionedCacheableObjectPartListPtr;

class VersionedCacheableObjectPartList : public CacheableObjectPartList {
 private:
  bool m_regionIsVersioned;
  bool m_serializeValues;
  bool m_hasTags;
  bool m_hasKeys;
  std::vector<VersionTagPtr> m_versionTags;
  std::vector<uint8_t> m_byteArray;
  uint16_t m_endpointMemId;
  VectorOfCacheableKeyPtr m_tempKeys;
  ACE_Recursive_Thread_Mutex& m_responseLock;

  static const uint8_t FLAG_NULL_TAG;
  static const uint8_t FLAG_FULL_TAG;
  static const uint8_t FLAG_TAG_WITH_NEW_ID;
  static const uint8_t FLAG_TAG_WITH_NUMBER_ID;

  void readObjectPart(int32_t index, DataInput& input, CacheableKeyPtr keyPtr);
  // never implemented.
  VersionedCacheableObjectPartList& operator=(
      const VersionedCacheableObjectPartList& other);
  VersionedCacheableObjectPartList(
      const VersionedCacheableObjectPartList& other);
  /*inline VersionedCacheableObjectPartList() : m_responseLock()
  {
        m_regionIsVersioned = false;
        m_serializeValues = false;
        m_endpointMemId = 0;
        GF_NEW(m_tempKeys, VectorOfCacheableKey);
  }*/

 public:
  VersionedCacheableObjectPartList(const VectorOfCacheableKey* keys,
                                   uint32_t* keysOffset,
                                   const HashMapOfCacheablePtr& values,
                                   const HashMapOfExceptionPtr& exceptions,
                                   const VectorOfCacheableKeyPtr& resultKeys,
                                   ThinClientRegion* region,
                                   MapOfUpdateCounters* trackerMap,
                                   int32_t destroyTracker, bool addToLocalCache,
                                   uint16_t m_dsmemId,
                                   ACE_Recursive_Thread_Mutex& responseLock)
      : CacheableObjectPartList(keys, keysOffset, values, exceptions,
                                resultKeys, region, trackerMap, destroyTracker,
                                addToLocalCache),
        m_tempKeys(std::make_shared<VectorOfCacheableKey>()),
        m_responseLock(responseLock) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_endpointMemId = m_dsmemId;
    m_hasTags = false;
    m_hasKeys = false;
  }

  VersionedCacheableObjectPartList(VectorOfCacheableKey* keys,
                                   int32_t totalMapSize,
                                   ACE_Recursive_Thread_Mutex& responseLock)
      : m_tempKeys(keys), m_responseLock(responseLock) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_hasTags = false;
    m_endpointMemId = 0;
    m_versionTags.resize(totalMapSize);
    this->m_hasKeys = false;
    ;
  }

  VersionedCacheableObjectPartList(ThinClientRegion* region, uint16_t dsmemId,
                                   ACE_Recursive_Thread_Mutex& responseLock)
      : CacheableObjectPartList(region),
        m_responseLock(responseLock),
        m_endpointMemId(dsmemId) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_hasTags = false;
    this->m_hasKeys = false;
    ;
  }

  VersionedCacheableObjectPartList(VectorOfCacheableKey* keys,
                                   ACE_Recursive_Thread_Mutex& responseLock)
      : m_tempKeys(keys), m_responseLock(responseLock) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_hasTags = false;
    m_endpointMemId = 0;
    this->m_hasKeys = false;
  }

  VersionedCacheableObjectPartList(ThinClientRegion* region,
                                   VectorOfCacheableKey* keys,
                                   ACE_Recursive_Thread_Mutex& responseLock)
      : CacheableObjectPartList(region),
        m_tempKeys(keys),
        m_responseLock(responseLock) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_hasTags = false;
    m_endpointMemId = 0;
    this->m_hasKeys = false;
  }

  VersionedCacheableObjectPartList(ThinClientRegion* region,
                                   ACE_Recursive_Thread_Mutex& responseLock)
      : m_responseLock(responseLock), CacheableObjectPartList(region) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_hasTags = false;
    m_endpointMemId = 0;
    this->m_hasKeys = false;
  }

  inline uint16_t getEndpointMemId() { return m_endpointMemId; }

  std::vector<VersionTagPtr>& getVersionedTagptr() { return m_versionTags; }

  void setVersionedTagptr(std::vector<VersionTagPtr>& versionTags) {
    m_versionTags = versionTags;
    m_hasTags = (m_versionTags.size() > 0);
  }

  int getVersionedTagsize() const {
    return static_cast<int>(m_versionTags.size());
  }

  VectorOfCacheableKeyPtr getSucceededKeys() { return m_tempKeys; }

  inline VersionedCacheableObjectPartList(
      uint16_t endpointMemId, ACE_Recursive_Thread_Mutex& responseLock)
      : m_tempKeys(std::make_shared<VectorOfCacheableKey>()),
        m_responseLock(responseLock) {
    m_regionIsVersioned = false;
    m_serializeValues = false;
    m_endpointMemId = endpointMemId;
    m_hasTags = false;
    m_hasKeys = false;
  }

  void addAll(VersionedCacheableObjectPartListPtr other) {
    // LOGDEBUG("DEBUG:: COPL.addAll called");
    // ACE_Guard< ACE_Recursive_Thread_Mutex > guard( this->m_responseLock );
    if (other->m_tempKeys != nullptr) {
      if (this->m_tempKeys == nullptr) {
        this->m_tempKeys = std::make_shared<VectorOfCacheableKey>();
        this->m_hasKeys = true;
        const auto size = other->m_tempKeys->size();
        for (int i = 0; i < size; i++) {
          this->m_tempKeys->push_back(other->m_tempKeys->at(i));
        }
      } else {
        if (this->m_tempKeys != nullptr) {
          if (!this->m_hasKeys) {
            LOGDEBUG(" VCOPL::addAll m_hasKeys should be true here");
            this->m_hasKeys = true;
          }
          const auto size = other->m_tempKeys->size();
          for (int i = 0; i < size; i++) {
            this->m_tempKeys->push_back(other->m_tempKeys->at(i));
          }
        }
      }
    }

    // set m_regionIsVersioned
    this->m_regionIsVersioned |= other->m_regionIsVersioned;
    size_t size = other->m_versionTags.size();
    LOGDEBUG(" VCOPL::addAll other->m_versionTags.size() = %d ", size);
    // Append m_versionTags
    if (size > 0) {
      for (size_t i = 0; i < size; i++) {
        this->m_versionTags.push_back(other->m_versionTags[i]);
      }
      m_hasTags = true;
    }
  }

  int size() {
    if (this->m_hasKeys) {
      return static_cast<int>(this->m_tempKeys->size());
    } else if (this->m_hasTags) {
      return static_cast<int>(this->m_versionTags.size());
    } else {
      LOGDEBUG(
          "DEBUG:: Should not call VCOPL.size() if hasKeys and hasTags both "
          "are false.!!");
    }
    return -1;
  }

  void addAllKeys(VectorOfCacheableKeyPtr keySet) {
    if (!this->m_hasKeys) {
      this->m_hasKeys = true;
      this->m_tempKeys = std::make_shared<VectorOfCacheableKey>(*keySet);
    } else {
      for (int i = 0; i < keySet->size(); i++) {
        this->m_tempKeys->push_back(keySet->at(i));
      }
    }
  }

  /**
   *@brief serialize this object
   **/
  virtual void toData(DataOutput& output) const;

  /**
   *@brief deserialize this object
   **/
  virtual Serializable* fromData(DataInput& input);

  /**
   * @brief creation function for java Object[]
   */
  /*inline static Serializable* createDeserializable()
  {
    return new VersionedCacheableObjectPartList();
  }*/

  /**
   *@brief Return the classId byte of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and derserialize into.
   */
  virtual int32_t classId() const;

  /**
   *@brief return the typeId byte of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and derserialize into.
   */
  virtual int8_t typeId() const;

  /**
   * Return the data serializable fixed ID size type for internal use.
   * @since GFE 5.7
   */
  virtual int8_t DSFID() const;

  virtual uint32_t objectSize() const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_VERSIONEDCACHEABLEOBJECTPARTLIST_H_
