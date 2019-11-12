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

#include "SerializationRegistry.hpp"

#include <functional>
#include <mutex>

#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableDate.hpp>
#include <geode/CacheableEnum.hpp>
#include <geode/CacheableFileName.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/CacheableString.hpp>
#include <geode/CacheableUndefined.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>
#include <geode/Properties.hpp>
#include <geode/Region.hpp>
#include <geode/RegionAttributes.hpp>
#include <geode/Struct.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheRegionHelper.hpp"
#include "CacheableObjectPartList.hpp"
#include "CacheableToken.hpp"
#include "ClientConnectionResponse.hpp"
#include "ClientProxyMembershipID.hpp"
#include "DiskStoreId.hpp"
#include "DiskVersionTag.hpp"
#include "EnumInfo.hpp"
#include "EventId.hpp"
#include "GetAllServersResponse.hpp"
#include "LocatorListResponse.hpp"
#include "PdxHelper.hpp"
#include "PdxType.hpp"
#include "QueueConnectionResponse.hpp"
#include "TXCommitMessage.hpp"
#include "ThinClientPoolDM.hpp"
#include "VersionTag.hpp"
#include "config.h"

namespace apache {
namespace geode {
namespace client {

void TheTypeMap::setup() {
  // Register Geode builtins here!!
  // update type ids in DSCode.hpp

  bindDataSerializablePrimitive(CacheableByte::createDeserializable,
                                DSCode::CacheableByte);
  bindDataSerializablePrimitive(CacheableBoolean::createDeserializable,
                                DSCode::CacheableBoolean);
  bindDataSerializablePrimitive(BooleanArray::createDeserializable,
                                DSCode::BooleanArray);
  bindDataSerializablePrimitive(CacheableBytes::createDeserializable,
                                DSCode::CacheableBytes);
  bindDataSerializablePrimitive(CacheableFloat::createDeserializable,
                                DSCode::CacheableFloat);
  bindDataSerializablePrimitive(CacheableFloatArray::createDeserializable,
                                DSCode::CacheableFloatArray);
  bindDataSerializablePrimitive(CacheableDouble::createDeserializable,
                                DSCode::CacheableDouble);
  bindDataSerializablePrimitive(CacheableDoubleArray::createDeserializable,
                                DSCode::CacheableDoubleArray);
  bindDataSerializablePrimitive(CacheableDate::createDeserializable,
                                DSCode::CacheableDate);
  bindDataSerializablePrimitive(CacheableFileName::createDeserializable,
                                DSCode::CacheableFileName);
  bindDataSerializablePrimitive(CacheableHashMap::createDeserializable,
                                DSCode::CacheableHashMap);
  bindDataSerializablePrimitive(CacheableHashSet::createDeserializable,
                                DSCode::CacheableHashSet);
  bindDataSerializablePrimitive(CacheableHashTable::createDeserializable,
                                DSCode::CacheableHashTable);
  bindDataSerializablePrimitive(CacheableIdentityHashMap::createDeserializable,
                                DSCode::CacheableIdentityHashMap);
  bindDataSerializablePrimitive(CacheableLinkedHashSet::createDeserializable,
                                DSCode::CacheableLinkedHashSet);
  bindDataSerializablePrimitive(CacheableInt16::createDeserializable,
                                DSCode::CacheableInt16);
  bindDataSerializablePrimitive(CacheableInt16Array::createDeserializable,
                                DSCode::CacheableInt16Array);
  bindDataSerializablePrimitive(CacheableInt32::createDeserializable,
                                DSCode::CacheableInt32);
  bindDataSerializablePrimitive(CacheableInt32Array::createDeserializable,
                                DSCode::CacheableInt32Array);
  bindDataSerializablePrimitive(CacheableInt64::createDeserializable,
                                DSCode::CacheableInt64);
  bindDataSerializablePrimitive(CacheableInt64Array::createDeserializable,
                                DSCode::CacheableInt64Array);
  bindDataSerializablePrimitive(CacheableObjectArray::createDeserializable,
                                DSCode::CacheableObjectArray);
  bindDataSerializablePrimitive(CacheableString::createDeserializable,
                                DSCode::CacheableASCIIString);
  bindDataSerializablePrimitive(CacheableString::createDeserializableHuge,
                                DSCode::CacheableASCIIStringHuge);
  bindDataSerializablePrimitive(CacheableString::createUTFDeserializable,
                                DSCode::CacheableString);
  bindDataSerializablePrimitive(CacheableString::createUTFDeserializableHuge,
                                DSCode::CacheableStringHuge);
  bindDataSerializablePrimitive(CacheableStringArray::createDeserializable,
                                DSCode::CacheableStringArray);
  bindDataSerializablePrimitive(CacheableVector::createDeserializable,
                                DSCode::CacheableVector);
  bindDataSerializablePrimitive(CacheableArrayList::createDeserializable,
                                DSCode::CacheableArrayList);
  bindDataSerializablePrimitive(CacheableLinkedList::createDeserializable,
                                DSCode::CacheableLinkedList);
  bindDataSerializablePrimitive(CacheableStack::createDeserializable,
                                DSCode::CacheableStack);
  bindDataSerializablePrimitive(CacheableCharacter::createDeserializable,
                                DSCode::CacheableCharacter);
  bindDataSerializablePrimitive(CharArray::createDeserializable,
                                DSCode::CharArray);
  bindDataSerializablePrimitive(Properties::createDeserializable,
                                DSCode::Properties);

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
  auto dsCode = static_cast<DSCode>(typeId);

  if (typeId == -1) {
    dsCode = static_cast<DSCode>(input.read());
  }

  LOGDEBUG("SerializationRegistry::deserialize typeId = %" PRId8
           " dsCode = % " PRId8,
           typeId, dsCode);

  switch (dsCode) {
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
      return std::move(enumObject);
    }
    case DSCode::CacheableUserData:
    case DSCode::CacheableUserData2:
    case DSCode::CacheableUserData4: {
      return dataSerializeableHandler->deserialize(input, dsCode);
    }
    case DSCode::FixedIDByte:
    case DSCode::FixedIDShort:
    case DSCode::FixedIDInt: {
      return deserializeDataSerializableFixedId(input, dsCode);
    }
    case DSCode::NullObj: {
      return nullptr;
    }
    default:
      break;
  }

  TypeFactoryMethod createType = nullptr;

  theTypeMap.findDataSerializablePrimitive(dsCode, createType);

  if (createType == nullptr) {
    throw IllegalStateException("Unregistered type in deserialization");
  }

  std::shared_ptr<Serializable> obj(createType());

  deserialize(input, obj);

  return obj;
}

std::shared_ptr<Serializable>
SerializationRegistry::deserializeDataSerializableFixedId(DataInput& input,
                                                          DSCode dsCode) const {
  int32_t fixedId = 0;
  switch (dsCode) {
    case DSCode::FixedIDByte: {
      fixedId = input.read();
      break;
    }
    case DSCode::FixedIDShort: {
      fixedId = input.readInt16();
      break;
    }
    case DSCode::FixedIDInt: {
      fixedId = input.readInt32();
      break;
    }
    default:
      throw IllegalStateException("Invalid fixed ID");
  }

  TypeFactoryMethod createType = nullptr;

  theTypeMap.findDataSerializableFixedId(static_cast<DSFid>(fixedId),
                                         createType);

  if (createType == nullptr) {
    throw IllegalStateException("Unregistered type in deserialization");
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

void SerializationRegistry::addDataSerializableType(TypeFactoryMethod func,
                                                    int32_t id) {
  theTypeMap.bindDataSerializable(func, id);
}

void SerializationRegistry::addPdxSerializableType(TypeFactoryMethodPdx func) {
  theTypeMap.bindPdxSerializable(func);
}

void SerializationRegistry::removeDataSerializableType(int32_t id) {
  theTypeMap.unbindDataSerializable(id);
}

void SerializationRegistry::addDataSerializableFixedIdType(
    TypeFactoryMethod func) {
  theTypeMap.bindDataSerializableFixedId(func);
}

void SerializationRegistry::addDataSerializableFixedIdType(
    internal::DSFid id, TypeFactoryMethod func) {
  theTypeMap.rebindDataSerializableFixedId(id, func);
}

void SerializationRegistry::removeDataSerializableFixeIdType(
    internal::DSFid id) {
  theTypeMap.unbindDataSerializableFixedId(id);
}

void SerializationRegistry::setDataSerializablePrimitiveType(
    TypeFactoryMethod func, DSCode dsCode) {
  theTypeMap.rebindDataSerializablePrimitive(dsCode, func);
}

std::shared_ptr<PdxSerializable> SerializationRegistry::getPdxSerializableType(
    const std::string& className) const {
  std::shared_ptr<PdxSerializable> pdxSerializable;

  if (auto typeFactoryMethodPdx = theTypeMap.findPdxSerializable(className)) {
    pdxSerializable = typeFactoryMethodPdx();
  } else {
    pdxSerializable = std::make_shared<PdxWrapper>(nullptr, className);
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
      m_dataSerializableMapLock);
  m_dataSerializableMap.clear();

  std::lock_guard<util::concurrent::spinlock_mutex> guard2(
      m_dataSerializableFixedIdMapLock);
  m_dataSerializableFixedIdMap.clear();

  std::lock_guard<util::concurrent::spinlock_mutex> guard3(
      m_pdxSerializableMapLock);
  m_pdxSerializableMap.clear();
}

void TheTypeMap::findDataSerializable(int32_t id,
                                      TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableMapLock);
  const auto& found = m_dataSerializableMap.find(id);
  if (found != m_dataSerializableMap.end()) {
    func = found->second;
  }
}

void TheTypeMap::findDataSerializableFixedId(DSFid dsfid,
                                             TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableFixedIdMapLock);
  const auto& found = m_dataSerializableFixedIdMap.find(dsfid);
  if (found != m_dataSerializableFixedIdMap.end()) {
    func = found->second;
  }
}

void TheTypeMap::findDataSerializablePrimitive(DSCode dsCode,
                                               TypeFactoryMethod& func) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializablePrimitiveMapLock);
  const auto& found = m_dataSerializablePrimitiveMap.find(dsCode);
  if (found != m_dataSerializablePrimitiveMap.end()) {
    func = found->second;
  }
}

void TheTypeMap::bindDataSerializable(TypeFactoryMethod func, int32_t id) {
  auto obj = func();

  if (const auto dataSerializable =
          std::dynamic_pointer_cast<DataSerializable>(obj)) {
    typeToClassId.emplace(dataSerializable->getType(), id);
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bind: Serialization type not implemented.");
  }

  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableMapLock);
  const auto& result = m_dataSerializableMap.emplace(id, func);
  if (!result.second) {
    LOGERROR("A class with ID %d is already registered.", id);
    throw IllegalStateException("A class with given ID is already registered.");
  }
}

void TheTypeMap::rebindDataSerializable(int32_t id, TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableMapLock);
  m_dataSerializableMap[id] = func;
}

void TheTypeMap::unbindDataSerializable(int32_t id) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableMapLock);
  m_dataSerializableMap.erase(id);
}

void TheTypeMap::bindDataSerializablePrimitive(TypeFactoryMethod func,
                                               DSCode dsCode) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializablePrimitiveMapLock);
  const auto& result = m_dataSerializablePrimitiveMap.emplace(dsCode, func);
  if (!result.second) {
    LOGERROR("A class with DSCode %d is already registered.", dsCode);
    throw IllegalStateException(
        "A class with given DSCode is already registered.");
  }
}

void TheTypeMap::rebindDataSerializablePrimitive(DSCode dsCode,
                                                 TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializablePrimitiveMapLock);
  m_dataSerializablePrimitiveMap[dsCode] = func;
}

void TheTypeMap::bindDataSerializableFixedId(TypeFactoryMethod func) {
  auto obj = func();

  DSFid id;
  if (const auto dataSerializableFixedId =
          std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    id = dataSerializableFixedId->getDSFID();
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bindDataSerializableInternal: Unknown serialization "
        "type.");
  }

  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableFixedIdMapLock);
  const auto& result = m_dataSerializableFixedIdMap.emplace(id, func);
  if (!result.second) {
    LOGERROR("A fixed class with ID %d is already registered.", id);
    throw IllegalStateException(
        "A fixed class with given ID is already registered.");
  }
}

void TheTypeMap::rebindDataSerializableFixedId(internal::DSFid id,
                                               TypeFactoryMethod func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableFixedIdMapLock);
  m_dataSerializableFixedIdMap[id] = func;
}

void TheTypeMap::unbindDataSerializableFixedId(internal::DSFid id) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_dataSerializableFixedIdMapLock);
  m_dataSerializableFixedIdMap.erase(id);
}

void TheTypeMap::bindPdxSerializable(TypeFactoryMethodPdx func) {
  auto obj = func();
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_pdxSerializableMapLock);
  auto&& objFullName = obj->getClassName();

  const auto& result = m_pdxSerializableMap.emplace(objFullName, func);
  if (!result.second) {
    LOGERROR("A object with FullName " + objFullName +
             " is already registered.");
    throw IllegalStateException(
        "A Object with given FullName is already registered.");
  }
}

TypeFactoryMethodPdx TheTypeMap::findPdxSerializable(
    const std::string& objFullName) const {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_pdxSerializableMapLock);

  const auto& found = m_pdxSerializableMap.find(objFullName);
  if (found != m_pdxSerializableMap.end()) {
    return found->second;
  }

  return nullptr;
}

void TheTypeMap::rebindPdxSerializable(std::string objFullName,
                                       TypeFactoryMethodPdx func) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_pdxSerializableMapLock);
  m_pdxSerializableMap[objFullName] = func;
}

void TheTypeMap::unbindPdxSerializable(const std::string& objFullName) {
  std::lock_guard<util::concurrent::spinlock_mutex> guard(
      m_pdxSerializableMapLock);
  m_pdxSerializableMap.erase(objFullName);
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
      cacheImpl->getSerializationRegistry()->getIdForDataSerializableType(type);
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
