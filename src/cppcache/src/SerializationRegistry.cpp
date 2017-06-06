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

#include "config.h"
#include <geode/geode_globals.hpp>
#include "SerializationRegistry.hpp"
#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/CacheableDate.hpp>
#include <geode/CacheableFileName.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheableUndefined.hpp>
#include <geode/Struct.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/GeodeTypeIds.hpp>
#include "CacheableToken.hpp"
#include <geode/Region.hpp>
#include "EventId.hpp"
#include <geode/Properties.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/RegionAttributes.hpp>
#include "CacheableObjectPartList.hpp"
#include "ClientConnectionResponse.hpp"
#include "QueueConnectionResponse.hpp"
#include "LocatorListResponse.hpp"
#include "GatewayEventCallbackArgument.hpp"
#include "GatewaySenderEventCallbackArgument.hpp"
#include "ClientProxyMembershipID.hpp"
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "GetAllServersResponse.hpp"
#include "TXCommitMessage.hpp"
#include <geode/PoolManager.hpp>
#include "ThinClientPoolDM.hpp"
#include "PdxType.hpp"
#include <geode/PdxWrapper.hpp>
#include "EnumInfo.hpp"
#include "VersionTag.hpp"
#include "DiskStoreId.hpp"
#include "DiskVersionTag.hpp"
#include "CachedDeserializableHelper.hpp"
#include <mutex>
#include <functional>

namespace apache {
namespace geode {
namespace client {

void TheTypeMap::setup() {
  // Register Geode builtins here!!
  // update type ids in GeodeTypeIds.hpp

  bind(CacheableByte::createDeserializable);
  bind(CacheableBoolean::createDeserializable);
  bind(BooleanArray::createDeserializable);
  bind(CacheableBytes::createDeserializable);
  bind(CacheableFloat::createDeserializable);
  bind(CacheableFloatArray::createDeserializable);
  bind(CacheableDouble::createDeserializable);
  bind(CacheableDoubleArray::createDeserializable);
  bind(CacheableDate::createDeserializable);
  bind(CacheableFileName::createDeserializable);
  bind(CacheableHashMap::createDeserializable);
  bind(CacheableHashSet::createDeserializable);
  bind(CacheableHashTable::createDeserializable);
  bind(CacheableIdentityHashMap::createDeserializable);
  bind(CacheableLinkedHashSet::createDeserializable);
  bind(CacheableInt16::createDeserializable);
  bind(CacheableInt16Array::createDeserializable);
  bind(CacheableInt32::createDeserializable);
  bind(CacheableInt32Array::createDeserializable);
  bind(CacheableInt64::createDeserializable);
  bind(CacheableInt64Array::createDeserializable);
  bind(CacheableObjectArray::createDeserializable);
  bind(CacheableString::createDeserializable);
  bind(CacheableString::createDeserializableHuge);
  bind(CacheableString::createUTFDeserializable);
  bind(CacheableString::createUTFDeserializableHuge);
  bind(CacheableStringArray::createDeserializable);
  bind(CacheableVector::createDeserializable);
  bind(CacheableArrayList::createDeserializable);
  bind(CacheableLinkedList::createDeserializable);
  bind(CacheableStack::createDeserializable);
  bind(CacheableWideChar::createDeserializable);
  bind(CharArray::createDeserializable);
  bind(CacheableToken::createDeserializable);
  bind(RegionAttributes::createDeserializable);
  bind(Properties::createDeserializable);

  bind2(CacheableUndefined::createDeserializable);
  bind2(EventId::createDeserializable);
  bind2(Struct::createDeserializable);
  bind2(ClientConnectionResponse::create);
  bind2(QueueConnectionResponse::create);
  bind2(LocatorListResponse::create);
  bind2(ClientProxyMembershipID::createDeserializable);
  bind2(GatewayEventCallbackArgument::createDeserializable);
  bind2(GatewaySenderEventCallbackArgument::createDeserializable);
  bind2(GetAllServersResponse::create);
  bind2(EnumInfo::createDeserializable);

  rebind2(GeodeTypeIdsImpl::DiskStoreId, DiskStoreId::createDeserializable);

  bind2(CachedDeserializableHelper::createForVmCachedDeserializable);
  bind2(CachedDeserializableHelper::createForPreferBytesDeserializable);
}

/** This starts at reading the typeid.. assumes the length has been read. */
SerializablePtr SerializationRegistry::deserialize(DataInput& input,
                                                   int8_t typeId) const {
  bool findinternal = false;
  int8_t currentTypeId = typeId;

  if (typeId == -1) input.read(&currentTypeId);
  int64_t compId = currentTypeId;

  LOGDEBUG("SerializationRegistry::deserialize typeid = %d currentTypeId= %d ",
           typeId, currentTypeId);
  if (compId == GeodeTypeIds::NullObj) {
    return nullptr;
  } else if (compId == GeodeTypeIds::CacheableNullString) {
    return SerializablePtr(CacheableString::createDeserializable());
  } else if (compId == GeodeTypeIdsImpl::CacheableUserData) {
    int8_t classId = 0;
    input.read(&classId);
    compId |= ((static_cast<int64_t>(classId)) << 32);
  } else if (compId == GeodeTypeIdsImpl::CacheableUserData2) {
    int16_t classId = 0;
    input.readInt(&classId);
    compId |= ((static_cast<int64_t>(classId)) << 32);
  } else if (compId == GeodeTypeIdsImpl::CacheableUserData4) {
    int32_t classId = 0;
    input.readInt(&classId);
    compId |= ((static_cast<int64_t>(classId)) << 32);
  }

  TypeFactoryMethod createType = nullptr;

  if (compId == GeodeTypeIdsImpl::FixedIDByte) {
    int8_t fixedId;
    input.read(&fixedId);
    compId = fixedId;
    findinternal = true;
  } else if (compId == GeodeTypeIdsImpl::FixedIDShort) {
    int16_t fixedId;
    input.readInt(&fixedId);
    compId = fixedId;
    findinternal = true;
  } else if (compId == GeodeTypeIdsImpl::FixedIDInt) {
    int32_t fixedId;
    input.readInt(&fixedId);
    compId = fixedId;
    findinternal = true;
  }

  if (findinternal) {
    theTypeMap.find2(compId, createType);
  } else {
    theTypeMap.find(compId, createType);
  }
  if (createType == nullptr) {
    if (findinternal) {
      LOGERROR(
          "Unregistered class ID %d during deserialization: Did the "
          "application register "
          "serialization types?",
          compId);
    } else {
      LOGERROR(
          "Unregistered class ID %d during deserialization: Did the "
          "application register "
          "serialization types?",
          (compId >> 32));
    }

    // instead of a null key or null value... an Exception should be thrown..
    throw IllegalStateException("Unregistered class ID in deserialization");
  }
  SerializablePtr obj(createType());
  // This assignment allows the fromData method to return a different object.
  auto tmp = obj->fromData(input);
  if (obj.get() == tmp) {
    return obj;
  }
  return tmp->shared_from_this();
}

void SerializationRegistry::addType(TypeFactoryMethod func) {
  theTypeMap.bind(func);
}

void SerializationRegistry::addPdxType(TypeFactoryMethodPdx func) {
  theTypeMap.bindPdxType(func);
}

void SerializationRegistry::addType(int64_t compId, TypeFactoryMethod func) {
  theTypeMap.rebind(compId, func);
}

void SerializationRegistry::removeType(int64_t compId) {
  theTypeMap.unbind(compId);
}

void SerializationRegistry::addType2(TypeFactoryMethod func) {
  theTypeMap.bind2(func);
}

void SerializationRegistry::addType2(int64_t compId, TypeFactoryMethod func) {
  theTypeMap.rebind2(compId, func);
}

void SerializationRegistry::removeType2(int64_t compId) {
  theTypeMap.unbind2(compId);
}

PdxSerializablePtr SerializationRegistry::getPdxType(char* className) {
  TypeFactoryMethodPdx objectType = nullptr;
  theTypeMap.findPdxType(className, objectType);
  PdxSerializablePtr pdxObj;
  if (nullptr == objectType) {
    try {
      pdxObj =
          std::make_shared<PdxWrapper>((const char*)className, m_pdxSerializer);
    } catch (const Exception&) {
      LOGERROR(
          "Unregistered class %s during PDX deserialization: Did the "
          "application register the PDX type or serializer?",
          className);
      throw IllegalStateException(
          "Unregistered class or serializer in PDX deserialization");
    }
  } else {
    pdxObj.reset(objectType());
  }
  return pdxObj;
}

void SerializationRegistry::setPdxSerializer(PdxSerializerPtr pdxSerializer) {
  m_pdxSerializer = pdxSerializer;
}

PdxSerializerPtr SerializationRegistry::getPdxSerializer() {
  return m_pdxSerializer;
}

int32_t SerializationRegistry::GetPDXIdForType(PoolPtr pool,
                                               SerializablePtr pdxType) const {
  if (pool == nullptr) {
    throw IllegalStateException("Pool not found, Pdx operation failed");
  }

  return static_cast<ThinClientPoolDM*>(pool.get())->GetPDXIdForType(pdxType);
}

SerializablePtr SerializationRegistry::GetPDXTypeById(PoolPtr pool,
                                                      int32_t typeId) const {
  if (pool == nullptr) {
    throw IllegalStateException("Pool not found, Pdx operation failed");
  }

  return static_cast<ThinClientPoolDM*>(pool.get())->GetPDXTypeById(typeId);
}

int32_t SerializationRegistry::GetEnumValue(PoolPtr pool,
                                            SerializablePtr enumInfo) const {
  if (pool == nullptr) {
    throw IllegalStateException("Pool not found, Pdx operation failed");
  }

  return static_cast<ThinClientPoolDM*>(pool.get())->GetEnumValue(enumInfo);
}
SerializablePtr SerializationRegistry::GetEnum(PoolPtr pool,
                                               int32_t val) const {
  if (pool == nullptr) {
    throw IllegalStateException("Pool not found, Pdx operation failed");
  }

  return static_cast<ThinClientPoolDM*>(pool.get())->GetEnum(val);
}

void TheTypeMap::clear() {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_mapLock);
  m_map->unbind_all();

  std::lock_guard<util::concurrent::spinlock_mutex> guard2(m_map2Lock);
  m_map2->unbind_all();

  std::lock_guard<util::concurrent::spinlock_mutex> guard3(m_pdxTypemapLock);
  m_pdxTypemap->unbind_all();
}

void TheTypeMap::find(int64_t id, TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_mapLock);
  m_map->find(id, func);
}

void TheTypeMap::find2(int64_t id, TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_map2Lock);
  m_map2->find(id, func);
}

void TheTypeMap::bind(TypeFactoryMethod func) {
  Serializable* obj = func();
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_mapLock);
  int64_t compId = static_cast<int64_t>(obj->typeId());
  if (compId == GeodeTypeIdsImpl::CacheableUserData ||
      compId == GeodeTypeIdsImpl::CacheableUserData2 ||
      compId == GeodeTypeIdsImpl::CacheableUserData4) {
    compId |= ((static_cast<int64_t>(obj->classId())) << 32);
  }
  delete obj;
  int bindRes = m_map->bind(compId, func);
  if (bindRes == 1) {
    LOGERROR(
        "A class with "
        "ID %d is already registered.",
        compId);
    throw IllegalStateException(
        "A class with "
        "given ID is already registered.");
  } else if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding class ID %d to map.",
        compId);
    throw IllegalStateException(
        "Unknown error "
        "while adding type to map.");
  }
}

void TheTypeMap::rebind(int64_t compId, TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_mapLock);
  int bindRes = m_map->rebind(compId, func);
  if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding class ID %d to map.",
        compId);
    throw IllegalStateException(
        "Unknown error "
        "while adding type to map.");
  }
}

void TheTypeMap::unbind(int64_t compId) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_mapLock);
  m_map->unbind(compId);
}

void TheTypeMap::bind2(TypeFactoryMethod func) {
  Serializable* obj = func();
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_map2Lock);
  int8_t dsfid = obj->DSFID();

  int64_t compId = 0;
  if (dsfid == GeodeTypeIdsImpl::FixedIDShort) {
    compId = compId = static_cast<int64_t>(obj->classId());
  } else {
    compId = static_cast<int64_t>(obj->typeId());
  }
  delete obj;
  int bindRes = m_map2->bind(compId, func);
  if (bindRes == 1) {
    LOGERROR(
        "A fixed class with "
        "ID %d is already registered.",
        compId);
    throw IllegalStateException(
        "A fixed class with "
        "given ID is already registered.");
  } else if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding class ID %d to map2.",
        compId);
    throw IllegalStateException(
        "Unknown error "
        "while adding to map2.");
  }
}

void TheTypeMap::rebind2(int64_t compId, TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_map2Lock);
  m_map2->rebind(compId, func);
}

void TheTypeMap::unbind2(int64_t compId) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_map2Lock);
  m_map2->unbind(compId);
}

void TheTypeMap::bindPdxType(TypeFactoryMethodPdx func) {
  PdxSerializable* obj = func();
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  const char* objFullName = obj->getClassName();

  int bindRes = m_pdxTypemap->bind(objFullName, func);

  delete obj;

  if (bindRes == 1) {
    LOGERROR("A object with FullName %s is already registered.", objFullName);
    throw IllegalStateException(
        "A Object with "
        "given FullName is already registered.");
  } else if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding Pdx Object named %s to map.",
        objFullName);
    throw IllegalStateException(
        "Unknown error "
        "while adding type to map.");
  }
}

void TheTypeMap::findPdxType(char* objFullName, TypeFactoryMethodPdx& func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  m_pdxTypemap->find(objFullName, func);
}

void TheTypeMap::rebindPdxType(char* objFullName, TypeFactoryMethodPdx func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  int bindRes = m_pdxTypemap->rebind(objFullName, func);
  if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding Pdx Object FullName %s to map.",
        objFullName);
    throw IllegalStateException(
        "Unknown error "
        "while adding type to map.");
  }
}

void TheTypeMap::unbindPdxType(char* objFullName) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  m_pdxTypemap->unbind(objFullName);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
