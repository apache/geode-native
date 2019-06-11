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
#include "VersionedCacheableObjectPartList.hpp"

#include <geode/CacheableString.hpp>
#include <geode/ExceptionTypes.hpp>

#include "CacheableToken.hpp"
#include "DiskStoreId.hpp"
#include "DiskVersionTag.hpp"
#include "ThinClientRegion.hpp"
namespace apache {
namespace geode {
namespace client {

const uint8_t VersionedCacheableObjectPartList::FLAG_NULL_TAG = 0;
const uint8_t VersionedCacheableObjectPartList::FLAG_FULL_TAG = 1;
const uint8_t VersionedCacheableObjectPartList::FLAG_TAG_WITH_NEW_ID = 2;
const uint8_t VersionedCacheableObjectPartList::FLAG_TAG_WITH_NUMBER_ID = 3;

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    const std::vector<std::shared_ptr<CacheableKey>>* keys,
    uint32_t* keysOffset, const std::shared_ptr<HashMapOfCacheable>& values,
    const std::shared_ptr<HashMapOfException>& exceptions,
    const std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>&
        resultKeys,
    ThinClientRegion* region, MapOfUpdateCounters* trackerMap,
    int32_t destroyTracker, bool addToLocalCache, uint16_t m_dsmemId,
    std::recursive_mutex& responseLock)
    : CacheableObjectPartList(keys, keysOffset, values, exceptions, resultKeys,
                              region, trackerMap, destroyTracker,
                              addToLocalCache),
      m_tempKeys(
          std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>()),
      m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_endpointMemId = m_dsmemId;
  m_hasTags = false;
  m_hasKeys = false;
}

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    std::vector<std::shared_ptr<CacheableKey>>* keys, int32_t totalMapSize,
    std::recursive_mutex& responseLock)
    : m_tempKeys(keys), m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_hasTags = false;
  m_endpointMemId = 0;
  m_versionTags.resize(totalMapSize);
  this->m_hasKeys = false;
  ;
}

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    ThinClientRegion* region, uint16_t dsmemId,
    std::recursive_mutex& responseLock)
    : CacheableObjectPartList(region),
      m_endpointMemId(dsmemId),
      m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_hasTags = false;
  this->m_hasKeys = false;
  ;
}

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> keys,
    std::recursive_mutex& responseLock)
    : m_tempKeys(keys), m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_hasTags = false;
  m_endpointMemId = 0;
  this->m_hasKeys = false;
}

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    ThinClientRegion* region, std::vector<std::shared_ptr<CacheableKey>>* keys,
    std::recursive_mutex& responseLock)
    : CacheableObjectPartList(region),
      m_tempKeys(keys),
      m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_hasTags = false;
  m_endpointMemId = 0;
  this->m_hasKeys = false;
}

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    ThinClientRegion* region, std::recursive_mutex& responseLock)
    : CacheableObjectPartList(region), m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_hasTags = false;
  m_endpointMemId = 0;
  this->m_hasKeys = false;
}

uint16_t VersionedCacheableObjectPartList::getEndpointMemId() {
  return m_endpointMemId;
}

std::vector<std::shared_ptr<VersionTag>>&
VersionedCacheableObjectPartList::getVersionedTagptr() {
  return m_versionTags;
}

void VersionedCacheableObjectPartList::setVersionedTagptr(
    std::vector<std::shared_ptr<VersionTag>>& versionTags) {
  m_versionTags = versionTags;
  m_hasTags = (m_versionTags.size() > 0);
}

int VersionedCacheableObjectPartList::getVersionedTagsize() const {
  return static_cast<int>(m_versionTags.size());
}

std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>
VersionedCacheableObjectPartList::getSucceededKeys() {
  return m_tempKeys;
}

VersionedCacheableObjectPartList::VersionedCacheableObjectPartList(
    uint16_t endpointMemId, std::recursive_mutex& responseLock)
    : m_tempKeys(
          std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>()),
      m_responseLock(responseLock) {
  m_regionIsVersioned = false;
  m_serializeValues = false;
  m_endpointMemId = endpointMemId;
  m_hasTags = false;
  m_hasKeys = false;
}

void VersionedCacheableObjectPartList::addAll(
    std::shared_ptr<VersionedCacheableObjectPartList> other) {
  if (other->m_tempKeys != nullptr) {
    if (this->m_tempKeys == nullptr) {
      this->m_tempKeys =
          std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>();
      this->m_hasKeys = true;
      this->m_tempKeys->insert(this->m_tempKeys->cend(),
                               other->m_tempKeys->cbegin(),
                               other->m_tempKeys->cend());
    } else {
      if (this->m_tempKeys != nullptr) {
        if (!this->m_hasKeys) {
          LOGDEBUG(" VCOPL::addAll m_hasKeys should be true here");
          this->m_hasKeys = true;
        }
        this->m_tempKeys->insert(this->m_tempKeys->cend(),
                                 other->m_tempKeys->cbegin(),
                                 other->m_tempKeys->cend());
      }
    }
  }

  // set m_regionIsVersioned
  this->m_regionIsVersioned |= other->m_regionIsVersioned;
  auto size = other->m_versionTags.size();
  LOGDEBUG(" VCOPL::addAll other->m_versionTags.size() = %zd ", size);
  // Append m_versionTags
  if (size > 0) {
    this->m_versionTags.insert(this->m_versionTags.cend(),
                               other->m_versionTags.cbegin(),
                               other->m_versionTags.cend());
    m_hasTags = true;
  }
}

int VersionedCacheableObjectPartList::size() {
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

void VersionedCacheableObjectPartList::addAllKeys(
    std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>> keySet) {
  if (!this->m_hasKeys) {
    this->m_hasKeys = true;
    this->m_tempKeys =
        std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>(*keySet);
  } else {
    this->m_tempKeys->insert(this->m_tempKeys->cend(), keySet->cbegin(),
                             keySet->cend());
  }
}

DSFid VersionedCacheableObjectPartList::getDSFID() const {
  return DSFid::VersionedObjectPartList;
}

void VersionedCacheableObjectPartList::toData(DataOutput&) const {
  throw UnsupportedOperationException(
      "VersionedCacheableObjectPartList::toData not implemented");
}

void VersionedCacheableObjectPartList::readObjectPart(
    int32_t index, DataInput& input, std::shared_ptr<CacheableKey> keyPtr) {
  auto objType = input.read();
  std::shared_ptr<Cacheable> value;
  m_byteArray[index] = objType;
  bool isException = (objType == 2 ? 1 : 0);

  if (isException) {  // Exception case
    // Skip the exception that is in java serialized format, we cant read it.
    input.advanceCursor(input.readArrayLength());
    const auto exMsg = input.readString();  ////4.1

    std::shared_ptr<Exception> ex;
    if (exMsg == "org.apache.geode.security.NotAuthorizedException") {
      auto message = "Authorization exception at server: " + exMsg;
      ex = std::make_shared<NotAuthorizedException>(message);
    } else {
      auto message = "Exception at remote server: " + exMsg;
      ex = std::make_shared<CacheServerException>(message);
    }
    m_exceptions->emplace(keyPtr, ex);
  } else if (m_serializeValues) {
    // read length
    int32_t skipLen = input.readArrayLength();
    int8_t* bytes = nullptr;
    if (skipLen > 0) {
      // readObject
      bytes = new int8_t[skipLen];
      input.readBytesOnly(bytes, skipLen);
    }
    m_values->emplace(keyPtr, CacheableBytes::create(
                                  std::vector<int8_t>(bytes, bytes + skipLen)));

    /* adongre
     * CID 29377: Resource leak (RESOURCE_LEAK)Calling allocation function
     * "apache::geode::client::DataInput::readBytes(unsigned char **, int *)" on
     * "bytes".
     */
    _GEODE_SAFE_DELETE_ARRAY(bytes);

  } else {
    // set nullptr to indicate that there is no exception for the key on this
    // index
    // readObject
    input.readObject(value);
    if (m_values) m_values->emplace(keyPtr, value);
  }
}

void VersionedCacheableObjectPartList::fromData(DataInput& input) {
  std::lock_guard<decltype(m_responseLock)> guard(m_responseLock);
  LOGDEBUG("VersionedCacheableObjectPartList::fromData");
  uint8_t flags = input.read();
  m_hasKeys = (flags & 0x01) == 0x01;
  bool hasObjects = (flags & 0x02) == 0x02;
  m_hasTags = (flags & 0x04) == 0x04;
  m_regionIsVersioned = (flags & 0x08) == 0x08;
  m_serializeValues = (flags & 0x10) == 0x10;
  bool persistent = (flags & 0x20) == 0x20;
  std::shared_ptr<CacheableString> exMsgPtr;
  int32_t len = 0;
  bool valuesNULL = false;
  int32_t keysOffset = (m_keysOffset != nullptr ? *m_keysOffset : 0);
  // bool readObjLen = false;
  // int32_t lenOfObjects = 0;
  if (m_values == nullptr) {
    m_values = std::make_shared<HashMapOfCacheable>();
    valuesNULL = true;
  }

  if (!m_hasKeys && !hasObjects && !m_hasTags) {
    LOGDEBUG(
        "VersionedCacheableObjectPartList::fromData: Looks like message has no "
        "data. Returning,");
  }

  auto localKeys =
      std::make_shared<std::vector<std::shared_ptr<CacheableKey>>>();
  if (m_hasKeys) {
    len = static_cast<int32_t>(input.readUnsignedVL());

    for (int32_t index = 0; index < len; ++index) {
      auto key = std::dynamic_pointer_cast<CacheableKey>(input.readObject());
      if (m_resultKeys != nullptr) {
        m_resultKeys->push_back(key);
      }
      m_tempKeys->push_back(key);
      localKeys->push_back(key);
    }
  } else if (m_keys != nullptr) {
    LOGDEBUG("VersionedCacheableObjectPartList::fromData: m_keys NOT nullptr");
    /*
       if (m_hasKeys) {
       int64_t tempLen;
       input.readUnsignedVL(&tempLen);
       len = (int32_t)tempLen;
       }else{
       len = m_keys->size();
       }
       lenOfObjects = len;
       readObjLen = true;
       for (int32_t index = keysOffset; index < keysOffset + len; ++index) {
       key = m_keys->at(index);
       if (m_resultKeys != nullptr) {
       m_resultKeys->push_back(key);
       }
       }*/
  } else if (hasObjects) {
    if (m_keys == nullptr && m_resultKeys == nullptr) {
      LOGERROR(
          "VersionedCacheableObjectPartList::fromData: Exception: hasObjects "
          "is true and m_keys and m_resultKeys are also nullptr");
      throw FatalInternalException(
          "VersionedCacheableObjectPartList: "
          "hasObjects is true and m_keys is also nullptr");
    } else {
      LOGDEBUG(
          "VersionedCacheableObjectPartList::fromData m_keys or m_resultKeys "
          "not null");
    }
  } else {
    LOGDEBUG(
        "VersionedCacheableObjectPartList::fromData m_hasKeys, m_keys, "
        "hasObjects all are nullptr");
  }  // m_hasKeys else ends here

  if (hasObjects) {
    len = static_cast<int32_t>(input.readUnsignedVL());
    m_byteArray.resize(len);
    for (int32_t index = 0; index < len; ++index) {
      if (m_keys != nullptr && !m_hasKeys) {
        readObjectPart(index, input, m_keys->at(index + keysOffset));
      } else /*if (m_resultKeys != nullptr && m_resultKeys->size() > 0)*/ {
        readObjectPart(index, input, localKeys->at(index));
      } /*else{
         LOGERROR("VersionedCacheableObjectPartList::fromData: hasObjects = true
       but m_keys is nullptr and m_resultKeys== nullptr or m_resultKeys->size=0"
       );
       }*/
    }
  }  // hasObjects ends here

  if (m_hasTags) {
    len = static_cast<int32_t>(input.readUnsignedVL());
    ;
    m_versionTags.resize(len);
    std::vector<uint16_t> ids;
    MemberListForVersionStamp& memberListForVersionStamp =
        *(m_region->getCacheImpl()->getMemberListForVersionStamp());
    for (int32_t index = 0; index < len; index++) {
      uint8_t entryType = input.read();
      std::shared_ptr<VersionTag> versionTag;
      switch (entryType) {
        case FLAG_NULL_TAG: {
          break;
        }
        case FLAG_FULL_TAG: {
          if (persistent) {
            versionTag = std::shared_ptr<VersionTag>(
                new DiskVersionTag(memberListForVersionStamp));
          } else {
            versionTag = std::shared_ptr<VersionTag>(
                new VersionTag(memberListForVersionStamp));
          }
          versionTag->fromData(input);
          versionTag->replaceNullMemberId(getEndpointMemId());
          break;
        }

        case FLAG_TAG_WITH_NEW_ID: {
          if (persistent) {
            versionTag = std::shared_ptr<VersionTag>(
                new DiskVersionTag(memberListForVersionStamp));
          } else {
            versionTag = std::shared_ptr<VersionTag>(
                new VersionTag(memberListForVersionStamp));
          }
          versionTag->fromData(input);
          ids.push_back(versionTag->getInternalMemID());
          break;
        }

        case FLAG_TAG_WITH_NUMBER_ID: {
          if (persistent) {
            versionTag = std::shared_ptr<VersionTag>(
                new DiskVersionTag(memberListForVersionStamp));
          } else {
            versionTag = std::shared_ptr<VersionTag>(
                new VersionTag(memberListForVersionStamp));
          }
          versionTag->fromData(input);
          auto idNumber = input.readUnsignedVL();
          versionTag->setInternalMemID(ids.at(idNumber));
          break;
        }
        default: { break; }
      }
      m_versionTags[index] = versionTag;
    }
  } else {  // if consistancyEnabled=false, we need to pass empty or
            // std::shared_ptr<NULL> m_versionTags
    for (int32_t index = 0; index < len; ++index) {
      std::shared_ptr<VersionTag> versionTag;
      m_versionTags[index] = versionTag;
    }
  }

  if (hasObjects) {
    std::shared_ptr<CacheableKey> key;
    std::shared_ptr<VersionTag> versionTag;
    std::shared_ptr<Cacheable> value;

    for (int32_t index = 0; index < len; ++index) {
      if (m_keys != nullptr && !m_hasKeys) {
        key = m_keys->at(index + keysOffset);
      } else /*if (m_resultKeys != nullptr && m_resultKeys->size() > 0)*/ {
        key = localKeys->at(index);
      } /*else{
         LOGERROR("VersionedCacheableObjectPartList::fromData: hasObjects = true
       but m_keys is nullptr AND m_resultKeys=nullptr or m_resultKeys->size=0"
       );
       }*/

      const auto& iter = m_values->find(key);
      value = iter == m_values->end() ? nullptr : iter->second;
      if (m_byteArray[index] != 3) {  // 3 - key not found on server
        std::shared_ptr<Cacheable> oldValue;
        if (m_addToLocalCache) {
          int updateCount = -1;
          versionTag = m_versionTags[index];

          GfErrType err =
              m_region->putLocal("getAll", false, key, value, oldValue, true,
                                 updateCount, m_destroyTracker, versionTag);
          if (err == GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION) {
            LOGDEBUG(
                "VersionedCacheableObjectPartList::fromData putLocal for key [%s] failed because the cache \
                  already contains an entry with higher version.",
                Utils::nullSafeToString(key).c_str());
            // replace the value with higher version tag
            (*m_values)[key] = oldValue;
          }
        }       // END::m_addToLocalCache
        else {  // m_addToLocalCache = false
          m_region->getEntry(key, oldValue);
          // if value has already been received via notification or put by
          // another thread, then return that
          if (oldValue != nullptr && !CacheableToken::isInvalid(oldValue)) {
            // replace the value with new value
            (*m_values)[key] = oldValue;
          }
        }
      }
    }
  }
  if (m_keysOffset != nullptr) *m_keysOffset += len;
  if (valuesNULL) m_values = nullptr;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
