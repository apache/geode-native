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

#include <mutex>
#include <functional>

#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

#include <geode/internal/geode_globals.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/CacheableDate.hpp>
#include <geode/CacheableFileName.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheableUndefined.hpp>
#include <geode/CacheableEnum.hpp>
#include <geode/Struct.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/Region.hpp>
#include <geode/Properties.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/RegionAttributes.hpp>
#include <geode/PoolManager.hpp>
#include <geode/PdxWrapper.hpp>

#include "config.h"

#include "SerializationRegistry.hpp"
#include "CacheableToken.hpp"
#include "EventId.hpp"
#include "CacheableObjectPartList.hpp"
#include "ClientConnectionResponse.hpp"
#include "QueueConnectionResponse.hpp"
#include "LocatorListResponse.hpp"
#include "ClientProxyMembershipID.hpp"
#include "GetAllServersResponse.hpp"
#include "TXCommitMessage.hpp"
#include "ThinClientPoolDM.hpp"
#include "PdxType.hpp"
#include "EnumInfo.hpp"
#include "VersionTag.hpp"
#include "DiskStoreId.hpp"
#include "DiskVersionTag.hpp"
#include "PdxHelper.hpp"

namespace apache {
namespace geode {
namespace client {

void TheTypeMap::setup() {
  // Register Geode builtins here!!
  // update type ids in DSCode.hpp

  bind(CacheableByte::createDeserializable, DSCode::CacheableByte);
  bind(CacheableBoolean::createDeserializable, DSCode::CacheableBoolean);
  bind(BooleanArray::createDeserializable, DSCode::BooleanArray);
  bind(CacheableBytes::createDeserializable, DSCode::CacheableBytes);
  bind(CacheableFloat::createDeserializable, DSCode::CacheableFloat);
  bind(CacheableFloatArray::createDeserializable, DSCode::CacheableFloatArray);
  bind(CacheableDouble::createDeserializable, DSCode::CacheableDouble);
  bind(CacheableDoubleArray::createDeserializable,
       DSCode::CacheableDoubleArray);
  bind(CacheableDate::createDeserializable, DSCode::CacheableDate);
  bind(CacheableFileName::createDeserializable, DSCode::CacheableFileName);
  bind(CacheableHashMap::createDeserializable, DSCode::CacheableHashMap);
  bind(CacheableHashSet::createDeserializable, DSCode::CacheableHashSet);
  bind(CacheableHashTable::createDeserializable, DSCode::CacheableHashTable);
  bind(CacheableIdentityHashMap::createDeserializable,
       DSCode::CacheableIdentityHashMap);
  bind(CacheableLinkedHashSet::createDeserializable,
       DSCode::CacheableLinkedHashSet);
  bind(CacheableInt16::createDeserializable, DSCode::CacheableInt16);
  bind(CacheableInt16Array::createDeserializable, DSCode::CacheableInt16Array);
  bind(CacheableInt32::createDeserializable, DSCode::CacheableInt32);
  bind(CacheableInt32Array::createDeserializable, DSCode::CacheableInt32Array);
  bind(CacheableInt64::createDeserializable, DSCode::CacheableInt64);
  bind(CacheableInt64Array::createDeserializable, DSCode::CacheableInt64Array);
  bind(CacheableObjectArray::createDeserializable,
       DSCode::CacheableObjectArray);
  bind(CacheableString::createDeserializable, DSCode::CacheableString);
  bind(CacheableString::createDeserializableHuge, DSCode::CacheableString);
  bind(CacheableString::createUTFDeserializable, DSCode::CacheableString);
  bind(CacheableString::createUTFDeserializableHuge, DSCode::CacheableString);
  bind(CacheableStringArray::createDeserializable,
       DSCode::CacheableStringArray);
  bind(CacheableVector::createDeserializable, DSCode::CacheableVector);
  bind(CacheableArrayList::createDeserializable, DSCode::CacheableArrayList);
  bind(CacheableLinkedList::createDeserializable, DSCode::CacheableLinkedList);
  bind(CacheableStack::createDeserializable, DSCode::CacheableStack);
  bind(CacheableCharacter::createDeserializable, DSCode::CacheableCharacter);
  bind(CharArray::createDeserializable, DSCode::CharArray);
  bind(CacheableToken::createDeserializable, DSCode::CacheableToken);
  bind(RegionAttributes::createDeserializable, DSCode::RegionAttributes);
  bind(Properties::createDeserializable, DSCode::Properties);

  bind2(CacheableUndefined::createDeserializable);
  bind2(EventId::createDeserializable);
  bind2(Struct::createDeserializable);
  bind2(ClientConnectionResponse::create);
  bind2(QueueConnectionResponse::create);
  bind2(LocatorListResponse::create);
  bind2(ClientProxyMembershipID::createDeserializable);
  bind2(GetAllServersResponse::create);
  bind2(EnumInfo::createDeserializable);
  bind2(DiskStoreId::createDeserializable);
}

/** This starts at reading the typeid.. assumes the length has been read. */
std::shared_ptr<Serializable> SerializationRegistry::deserialize(
    DataInput& input, int8_t typeId) const {
  bool findinternal = false;
  auto typedTypeId = static_cast<DSCode>(typeId);
  int64_t compId = typeId;

  if (typeId == -1) {
    compId = input.read();
    typedTypeId = static_cast<DSCode>(compId);
  }

  LOGDEBUG(
      "SerializationRegistry::deserialize typeid = %d currentTypeId= %" PRId8,
      typeId, typedTypeId);

  switch (typedTypeId) {
    case DSCode::CacheableNullString: {
      return std::shared_ptr<Serializable>(
          CacheableString::createDeserializable());
    }
    case DSCode::PDX: {
      return pdxTypeHandler->deserialize(input);
    }
    case DSCode::CacheableEnum: {
      auto enumObject = CacheableEnum::create(" ", " ", 0);
      enumObject->fromData(input);
      return enumObject;
    }
    case DSCode::CacheableUserData: {
      compId |= ((static_cast<int64_t>(input.read())) << 32);
      break;
    }
    case DSCode::CacheableUserData2: {
      compId |= ((static_cast<int64_t>(input.readInt16())) << 32);
      break;
    }
    case DSCode::CacheableUserData4: {
      int32_t classId = input.readInt32();
      compId |= ((static_cast<int64_t>(classId)) << 32);
      break;
    }
    case DSCode::FixedIDByte: {
      compId = input.read();
      findinternal = true;
      break;
    }
    case DSCode::FixedIDShort: {
      compId = input.readInt16();
      findinternal = true;
      break;
    }
    case DSCode::FixedIDInt: {
      int32_t fixedId = input.readInt32();
      compId = fixedId;
      findinternal = true;
      break;
    }
    case DSCode::NullObj: {
      return nullptr;
    }
    default:
      break;
  }

  TypeFactoryMethod createType = nullptr;

  if (findinternal) {
    theTypeMap.find2(compId, createType);
  } else {
    theTypeMap.find(compId, createType);
  }

  if (createType == nullptr) {
    if (findinternal) {
      LOGERROR(
          "Unregistered class ID %d during deserialization: Did the "
          "application register serialization types?",
          compId);
    } else {
      LOGERROR(
          "Unregistered class ID %d during deserialization: Did the "
          "application register serialization types?",
          (compId >> 32));
    }

    // instead of a null key or null value... an Exception should be thrown..
    throw IllegalStateException("Unregistered class ID in deserialization");
  }

  std::shared_ptr<Serializable> obj(createType());

  deserialize(input, obj);

  return obj;
}

void SerializationRegistry::deserialize(
    DataInput& input, std::shared_ptr<Serializable> obj) const {
  if (!obj) {
    // nothing to read
  } else if (const auto dataSerializableInternal =
                 std::dynamic_pointer_cast<DataSerializableInternal>(obj)) {
    deserialize(input, dataSerializableInternal);
  } else if (const auto dataSerializableFixedId =
                 std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    deserialize(input, dataSerializableFixedId);
  } else if (const auto dataSerializablePrimitive =
                 std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    deserialize(input, dataSerializablePrimitive);
  } else if (const auto dataSerializable =
                 std::dynamic_pointer_cast<DataSerializable>(obj)) {
    deserialize(input, dataSerializable);
  } else if (const auto pdxSerializable =
                 std::dynamic_pointer_cast<PdxSerializable>(obj)) {
    deserialize(input, pdxSerializable);
  } else {
    throw UnsupportedOperationException("Serialization type not implemented.");
  }
}

void SerializationRegistry::deserialize(
    DataInput& input,
    std::shared_ptr<DataSerializableInternal> dataSerializableInternal) const {
  dataSerializableInternal->fromData(input);
}

void SerializationRegistry::deserialize(
    DataInput& input,
    std::shared_ptr<DataSerializableFixedId> dataSerializableFixedId) const {
  dataSerializableFixedId->fromData(input);
}

void SerializationRegistry::deserialize(
    DataInput& input,
    std::shared_ptr<DataSerializablePrimitive> dataSerializablePrimitive)
    const {
  dataSerializablePrimitive->fromData(input);
}

void SerializationRegistry::deserialize(
    DataInput& input,
    std::shared_ptr<DataSerializable> dataSerializable) const {
  dataSerializable->fromData(input);
}

void SerializationRegistry::deserialize(
    DataInput& /*input*/,
    std::shared_ptr<PdxSerializable> /*pdxSerializable*/) const {
  throw UnsupportedOperationException(
      "SerializationRegistry::deserialize<PdxSerializable> not implemented");
}

void SerializationRegistry::serializeWithoutHeader(
    const std::shared_ptr<PdxSerializable>& obj, DataOutput& output) const {
  pdxTypeHandler->serialize(obj, output);
}

void SerializationRegistry::addType(TypeFactoryMethod func, int32_t id) {
  theTypeMap.bind(func, id);
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

std::shared_ptr<PdxSerializable> SerializationRegistry::getPdxType(
    const std::string& className) const {
  TypeFactoryMethodPdx objectType = nullptr;
  theTypeMap.findPdxType(className, objectType);
  std::shared_ptr<PdxSerializable> pdxSerializable;

  if (nullptr == objectType) {
    try {
      pdxSerializable = std::make_shared<PdxWrapper>(nullptr, className);
    } catch (const Exception&) {
      LOGERROR("Unregistered class " + className +
               " during PDX deserialization: Did the application register the "
               "PDX type or serializer?");
      throw IllegalStateException(
          "Unregistered class or serializer in PDX deserialization");
    }
  } else {
    pdxSerializable = objectType();
  }
  return pdxSerializable;
}

void SerializationRegistry::setPdxSerializer(
    std::shared_ptr<PdxSerializer> serializer) {
  this->pdxSerializer = serializer;
}

std::shared_ptr<PdxSerializer> SerializationRegistry::getPdxSerializer() {
  return pdxSerializer;
}

int32_t SerializationRegistry::GetPDXIdForType(
    Pool* pool, std::shared_ptr<Serializable> pdxType) const {
  if (auto poolDM = dynamic_cast<ThinClientPoolDM*>(pool)) {
    return poolDM->GetPDXIdForType(pdxType);
  }

  throw IllegalStateException("Pool not found, Pdx operation failed");
}

std::shared_ptr<Serializable> SerializationRegistry::GetPDXTypeById(
    Pool* pool, int32_t typeId) const {
  if (auto poolDM = dynamic_cast<ThinClientPoolDM*>(pool)) {
    return poolDM->GetPDXTypeById(typeId);
  }

  throw IllegalStateException("Pool not found, Pdx operation failed");
}

int32_t SerializationRegistry::GetEnumValue(
    std::shared_ptr<Pool> pool, std::shared_ptr<Serializable> enumInfo) const {
  if (pool == nullptr) {
    throw IllegalStateException("Pool not found, Pdx operation failed");
  }

  return static_cast<ThinClientPoolDM*>(pool.get())->GetEnumValue(enumInfo);
}
std::shared_ptr<Serializable> SerializationRegistry::GetEnum(
    std::shared_ptr<Pool> pool, int32_t val) const {
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

void TheTypeMap::bind(TypeFactoryMethod func, uint32_t id) {
  auto obj = func();
  int64_t compId;

  if (const auto dataSerializablePrimitive =
          std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    compId = static_cast<int64_t>(dataSerializablePrimitive->getDsCode());
  } else if (const auto dataSerializableInternal =
                 std::dynamic_pointer_cast<DataSerializableInternal>(obj)) {
    compId = static_cast<int64_t>(dataSerializableInternal->getInternalId());
  } else if (const auto dataSerializable =
                 std::dynamic_pointer_cast<DataSerializable>(obj)) {
    std::string typeName(dataSerializable->getType());
    std::string managedType("ManagedCacheableKeyGeneric");
    std::size_t found = typeName.find(managedType);
    if (found != std::string::npos) {
      // managedTypeToClassId
      typeToClassId[(dataSerializable->getType())] = id;
      compId = static_cast<int64_t>(
                   SerializationRegistry::getSerializableDataDsCode(id)) |
               static_cast<int64_t>(id) << 32;
    } else {
      typeToClassId[(dataSerializable->getType())] = id;
      compId = static_cast<int64_t>(
                   SerializationRegistry::getSerializableDataDsCode(id)) |
               static_cast<int64_t>(id) << 32;
    }
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bind: Serialization type not implemented.");
  }

  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_mapLock);
  int bindRes = m_map->bind(compId, func);
  if (bindRes == 1) {
    LOGERROR("A class with ID %d is already registered.", compId);
    throw IllegalStateException("A class with given ID is already registered.");
  } else if (bindRes == -1) {
    LOGERROR("Unknown error while adding class ID %d to map.", compId);
    throw IllegalStateException("Unknown error while adding type to map.");
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
  auto obj = func();

  int64_t compId = 0;
  if (const auto dataSerializableFixedId =
          std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    compId = static_cast<int64_t>(dataSerializableFixedId->getDSFID());
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bind2: Unknown serialization type.");
  }

  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_map2Lock);
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
  auto obj = func();
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  auto&& objFullName = obj->getClassName();

  int bindRes = m_pdxTypemap->bind(objFullName, func);

  if (bindRes == 1) {
    LOGERROR("A object with FullName " + objFullName +
             " is already registered.");
    throw IllegalStateException(
        "A Object with given FullName is already registered.");
  } else if (bindRes == -1) {
    LOGERROR("Unknown error while adding Pdx Object named " + objFullName +
             " to map.");
    throw IllegalStateException("Unknown error while adding type to map.");
  }
}

void TheTypeMap::findPdxType(const std::string& objFullName,
                             TypeFactoryMethodPdx& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  m_pdxTypemap->find(objFullName, func);
}

void TheTypeMap::rebindPdxType(std::string objFullName,
                               TypeFactoryMethodPdx func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  int bindRes = m_pdxTypemap->rebind(objFullName, func);
  if (bindRes == -1) {
    LOGERROR("Unknown error while adding Pdx Object FullName " + objFullName +
             " to map.");
    throw IllegalStateException(
        "Unknown error "
        "while adding type to map.");
  }
}

void TheTypeMap::unbindPdxType(const std::string& objFullName) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(m_pdxTypemapLock);
  m_pdxTypemap->unbind(objFullName);
}

void PdxTypeHandler::serialize(
    const std::shared_ptr<PdxSerializable>& pdxSerializable,
    DataOutput& dataOutput) const {
  PdxHelper::serializePdx(dataOutput, pdxSerializable);
}

std::shared_ptr<PdxSerializable> PdxTypeHandler::deserialize(
    DataInput& dataInput) const {
  return PdxHelper::deserializePdx(dataInput, false);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
