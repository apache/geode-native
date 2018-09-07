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
#include "CacheRegionHelper.hpp"

namespace apache {
namespace geode {
namespace client {

void TheTypeMap::setup() {
  // Register Geode builtins here!!
  // update type ids in DSCode.hpp

  bindDataSerializable(CacheableByte::createDeserializable,
                       DSCode::CacheableByte);
  bindDataSerializable(CacheableBoolean::createDeserializable,
                       DSCode::CacheableBoolean);
  bindDataSerializable(BooleanArray::createDeserializable,
                       DSCode::BooleanArray);
  bindDataSerializable(CacheableBytes::createDeserializable,
                       DSCode::CacheableBytes);
  bindDataSerializable(CacheableFloat::createDeserializable,
                       DSCode::CacheableFloat);
  bindDataSerializable(CacheableFloatArray::createDeserializable,
                       DSCode::CacheableFloatArray);
  bindDataSerializable(CacheableDouble::createDeserializable,
                       DSCode::CacheableDouble);
  bindDataSerializable(CacheableDoubleArray::createDeserializable,
                       DSCode::CacheableDoubleArray);
  bindDataSerializable(CacheableDate::createDeserializable,
                       DSCode::CacheableDate);
  bindDataSerializable(CacheableFileName::createDeserializable,
                       DSCode::CacheableFileName);
  bindDataSerializable(CacheableHashMap::createDeserializable,
                       DSCode::CacheableHashMap);
  bindDataSerializable(CacheableHashSet::createDeserializable,
                       DSCode::CacheableHashSet);
  bindDataSerializable(CacheableHashTable::createDeserializable,
                       DSCode::CacheableHashTable);
  bindDataSerializable(CacheableIdentityHashMap::createDeserializable,
                       DSCode::CacheableIdentityHashMap);
  bindDataSerializable(CacheableLinkedHashSet::createDeserializable,
                       DSCode::CacheableLinkedHashSet);
  bindDataSerializable(CacheableInt16::createDeserializable,
                       DSCode::CacheableInt16);
  bindDataSerializable(CacheableInt16Array::createDeserializable,
                       DSCode::CacheableInt16Array);
  bindDataSerializable(CacheableInt32::createDeserializable,
                       DSCode::CacheableInt32);
  bindDataSerializable(CacheableInt32Array::createDeserializable,
                       DSCode::CacheableInt32Array);
  bindDataSerializable(CacheableInt64::createDeserializable,
                       DSCode::CacheableInt64);
  bindDataSerializable(CacheableInt64Array::createDeserializable,
                       DSCode::CacheableInt64Array);
  bindDataSerializable(CacheableObjectArray::createDeserializable,
                       DSCode::CacheableObjectArray);
  bindDataSerializable(CacheableString::createDeserializable,
                       DSCode::CacheableString);
  bindDataSerializable(CacheableString::createDeserializableHuge,
                       DSCode::CacheableString);
  bindDataSerializable(CacheableString::createUTFDeserializable,
                       DSCode::CacheableString);
  bindDataSerializable(CacheableString::createUTFDeserializableHuge,
                       DSCode::CacheableString);
  bindDataSerializable(CacheableStringArray::createDeserializable,
                       DSCode::CacheableStringArray);
  bindDataSerializable(CacheableVector::createDeserializable,
                       DSCode::CacheableVector);
  bindDataSerializable(CacheableArrayList::createDeserializable,
                       DSCode::CacheableArrayList);
  bindDataSerializable(CacheableLinkedList::createDeserializable,
                       DSCode::CacheableLinkedList);
  bindDataSerializable(CacheableStack::createDeserializable,
                       DSCode::CacheableStack);
  bindDataSerializable(CacheableCharacter::createDeserializable,
                       DSCode::CacheableCharacter);
  bindDataSerializable(CharArray::createDeserializable, DSCode::CharArray);
  bindDataSerializable(Properties::createDeserializable, DSCode::Properties);
  bindDataSerializable(CacheableToken::createDeserializable,
                       DSCode::CacheableToken);
  bindDataSerializable(RegionAttributes::createDeserializable,
                       DSCode::RegionAttributes);

  bindDataSerializableFixedId(CacheableUndefined::createDeserializable);
  bindDataSerializableFixedId(EventId::createDeserializable);
  bindDataSerializableFixedId(Struct::createDeserializable);
  bindDataSerializableFixedId(ClientConnectionResponse::create);
  bindDataSerializableFixedId(QueueConnectionResponse::create);
  bindDataSerializableFixedId(LocatorListResponse::create);
  bindDataSerializableFixedId(ClientProxyMembershipID::createDeserializable);
  bindDataSerializableFixedId(GetAllServersResponse::create);
  bindDataSerializableFixedId(EnumInfo::createDeserializable);
  bindDataSerializableFixedId(DiskStoreId::createDeserializable);
}

/** This starts at reading the typeid.. assumes the length has been read. */
std::shared_ptr<Serializable> SerializationRegistry::deserialize(
    DataInput& input, int8_t typeId) const {
  bool findinternal = false;
  auto typedTypeId = static_cast<DSCode>(typeId);
  int32_t classId = typeId;

  if (typeId == -1) {
    classId = input.read();
    typedTypeId = static_cast<DSCode>(classId);
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
    case DSCode::CacheableUserData:
    case DSCode::CacheableUserData2:
    case DSCode::CacheableUserData4: {
      return dataSerializeableHandler->deserialize(input, typedTypeId);
    }
    case DSCode::FixedIDByte: {
      classId = input.read();
      findinternal = true;
      break;
    }
    case DSCode::FixedIDShort: {
      classId = input.readInt16();
      findinternal = true;
      break;
    }
    case DSCode::FixedIDInt: {
      classId = input.readInt32();
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
    theTypeMap.findDataSerializableFixedId(classId, createType);
  } else {
    theTypeMap.findDataSerializable(classId, createType);
  }

  if (createType == nullptr) {
    if (findinternal) {
      LOGERROR(
          "Unregistered class ID %d during deserialization: Did the "
          "application register serialization types?",
          classId);
    } else {
      LOGERROR(
          "Unregistered class ID %d during deserialization: Did the "
          "application register serialization types?",
          classId);
    }

    // instead of a null key or null value... an Exception should be thrown..
    throw IllegalStateException("Unregistered class ID in deserialization");
  }

  std::shared_ptr<Serializable> obj(createType());

  deserialize(input, obj);

  return obj;
}

void SerializationRegistry::deserialize(
    DataInput& input, const std::shared_ptr<Serializable>& obj) const {
  if (!obj) {
    // nothing to read
  } else if (const auto&& dataSerializableInternal =
                 std::dynamic_pointer_cast<DataSerializableInternal>(obj)) {
    dataSerializableInternal->fromData(input);
  } else if (const auto&& dataSerializableFixedId =
                 std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    dataSerializableFixedId->fromData(input);
  } else if (const auto&& dataSerializablePrimitive =
                 std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    dataSerializablePrimitive->fromData(input);
  } else {
    throw UnsupportedOperationException("Serialization type not implemented.");
  }
}

void SerializationRegistry::serializeWithoutHeader(
    const std::shared_ptr<PdxSerializable>& obj, DataOutput& output) const {
  pdxTypeHandler->serialize(obj, output);
}

void SerializationRegistry::addType(TypeFactoryMethod func, int32_t id) {
  theTypeMap.bindDataSerializable(func, id);
}

void SerializationRegistry::addPdxType(TypeFactoryMethodPdx func) {
  theTypeMap.bindPdxType(func);
}

void SerializationRegistry::addType(int32_t id, TypeFactoryMethod func) {
  theTypeMap.rebindDataSerializable(id, func);
}

void SerializationRegistry::removeType(int32_t id) {
  theTypeMap.unbindDataSerializable(id);
}

void SerializationRegistry::addType2(TypeFactoryMethod func) {
  theTypeMap.bindDataSerializableFixedId(func);
}

void SerializationRegistry::addType2(int32_t id, TypeFactoryMethod func) {
  theTypeMap.rebindDataSerializableFixedId(id, func);
}

void SerializationRegistry::removeType2(int32_t id) {
  theTypeMap.unbindDataSerializableFixedId(id);
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
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableMapLock);
  m_DataSerializableMap->unbind_all();

  std::lock_guard<util::concurrent::spinlock_mutex> guard2(
      m_DataSerializableFixedIdMapLock);
  m_DataSerializableFixedIdMap->unbind_all();

  std::lock_guard<util::concurrent::spinlock_mutex> guard3(m_pdxTypemapLock);
  m_pdxTypemap->unbind_all();
}

void TheTypeMap::findDataSerializable(int32_t id,
                                      TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableMapLock);
  m_DataSerializableMap->find(id, func);
}

void TheTypeMap::findDataSerializableFixedId(int32_t id,
                                             TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableFixedIdMapLock);
  m_DataSerializableFixedIdMap->find(id, func);
}
void TheTypeMap::bindDataSerializable(TypeFactoryMethod func, int32_t id) {
  auto obj = func();
  int32_t objectId;

  if (const auto dataSerializable =
          std::dynamic_pointer_cast<DataSerializable>(obj)) {
    typeToClassId.emplace(dataSerializable->getType(), id);
    objectId = id;
  } else if (const auto dataSerializablePrimitive =
                 std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    objectId = static_cast<int32_t>(dataSerializablePrimitive->getDsCode());
  } else if (const auto dataSerializableInternal =
                 std::dynamic_pointer_cast<DataSerializableInternal>(obj)) {
    objectId = static_cast<int32_t>(dataSerializableInternal->getInternalId());
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bind: Serialization type not implemented.");
  }

  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableMapLock);
  int bindRes = m_DataSerializableMap->bind(objectId, func);
  if (bindRes == 1) {
    LOGERROR("A class with ID %d is already registered.", objectId);
    throw IllegalStateException("A class with given ID is already registered.");
  } else if (bindRes == -1) {
    LOGERROR("Unknown error while adding class ID %d to map.", objectId);
    throw IllegalStateException("Unknown error while adding type to map.");
  }
}

void TheTypeMap::rebindDataSerializable(int32_t id, TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableMapLock);
  int bindRes = m_DataSerializableMap->rebind(id, func);
  if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding class ID %d to map.",
        id);
    throw IllegalStateException(
        "Unknown error "
        "while adding type to map.");
  }
}

void TheTypeMap::unbindDataSerializable(int32_t id) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableMapLock);
  m_DataSerializableMap->unbind(id);
}

void TheTypeMap::bindDataSerializableFixedId(TypeFactoryMethod func) {
  auto obj = func();

  int32_t id = 0;
  if (const auto dataSerializableFixedId =
          std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    id = static_cast<int64_t>(dataSerializableFixedId->getDSFID());
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bindDataSerializableInternal: Unknown serialization "
        "type.");
  }

  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableFixedIdMapLock);
  int bindRes = m_DataSerializableFixedIdMap->bind(id, func);
  if (bindRes == 1) {
    LOGERROR(
        "A fixed class with "
        "ID %d is already registered.",
        id);
    throw IllegalStateException(
        "A fixed class with "
        "given ID is already registered.");
  } else if (bindRes == -1) {
    LOGERROR(
        "Unknown error "
        "while adding class ID %d to map2.",
        id);
    throw IllegalStateException(
        "Unknown error "
        "while adding to map2.");
  }
}

void TheTypeMap::rebindDataSerializableFixedId(int32_t id,
                                               TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableFixedIdMapLock);
  m_DataSerializableFixedIdMap->rebind(id, func);
}

void TheTypeMap::unbindDataSerializableFixedId(int32_t id) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_DataSerializableFixedIdMapLock);
  m_DataSerializableFixedIdMap->unbind(id);
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

void DataSerializableHandler::serialize(
    const std::shared_ptr<DataSerializable>& dataSerializable,
    DataOutput& dataOutput, bool isDelta) const {
  auto&& cacheImpl = CacheRegionHelper::getCacheImpl(dataOutput.getCache());
  auto&& type = dataSerializable->getType();

  auto&& objectId =
      cacheImpl->getSerializationRegistry()->GetIdForDataSerializableType(type);
  auto&& dsCode = SerializationRegistry::getSerializableDataDsCode(objectId);

  dataOutput.write(static_cast<int8_t>(dsCode));
  switch (dsCode) {
    case DSCode::CacheableUserData:
      dataOutput.write(static_cast<int8_t>(objectId));
      break;
    case DSCode::CacheableUserData2:
      dataOutput.writeInt(static_cast<int16_t>(objectId));
      break;
    case DSCode::CacheableUserData4:
      dataOutput.writeInt(static_cast<int32_t>(objectId));
      break;
    default:
      IllegalStateException("Invalid DS Code.");
  }

  if (isDelta) {
    const Delta* ptr = dynamic_cast<const Delta*>(dataSerializable.get());
    ptr->toDelta(dataOutput);
  } else {
    dataSerializable->toData(dataOutput);
    ;
  }
}

std::shared_ptr<DataSerializable> DataSerializableHandler::deserialize(
    DataInput& input, DSCode typeId) const {
  int32_t classId = -1;
  switch (typeId) {
    case DSCode::CacheableUserData: {
      classId = input.read();
      break;
    }
    case DSCode::CacheableUserData2: {
      classId = input.readInt16();
      break;
    }
    case DSCode::CacheableUserData4: {
      classId = input.readInt32();
      break;
    }
    default:
      break;
  }
  TypeFactoryMethod createType =
      input.getCache()->getTypeRegistry().getCreationFunction(classId);

  if (createType == nullptr) {
    LOGERROR(
        "Unregistered class ID %d during deserialization: Did the "
        "application register serialization types?",
        classId);

    // instead of a null key or null value... an Exception should be thrown..
    throw IllegalStateException("Unregistered class ID in deserialization");
  }

  std::shared_ptr<DataSerializable> serializableObject(
      std::dynamic_pointer_cast<DataSerializable>(createType()));

  serializableObject->fromData(input);

  return serializableObject;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
