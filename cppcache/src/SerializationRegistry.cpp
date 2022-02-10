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

#include <limits>

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
#include <geode/Properties.hpp>
#include <geode/Region.hpp>
#include <geode/Struct.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheRegionHelper.hpp"
#include "CacheableToken.hpp"
#include "ClientConnectionResponse.hpp"
#include "ClientProxyMembershipID.hpp"
#include "DiskStoreId.hpp"
#include "EnumInfo.hpp"
#include "EventId.hpp"
#include "GatewaySenderEventCallbackArgument.hpp"
#include "GetAllServersResponse.hpp"
#include "LocatorListResponse.hpp"
#include "PdxHelper.hpp"
#include "PdxInstanceImpl.hpp"
#include "QueueConnectionResponse.hpp"
#include "ThinClientPoolDM.hpp"

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
  bindDataSerializableFixedId(GatewaySenderEventCallbackArgument::create);
}

void SerializationRegistry::serialize(const std::shared_ptr<Serializable>& obj,
                                      DataOutput& output, bool isDelta) const {
  if (obj == nullptr) {
    output.write(static_cast<int8_t>(DSCode::NullObj));
  } else if (auto&& pdxSerializable =
                 std::dynamic_pointer_cast<PdxSerializable>(obj)) {
    serialize(pdxSerializable, output);
  } else if (const auto&& dataSerializableFixedId =
                 std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    serialize(dataSerializableFixedId, output);
  } else if (const auto&& dataSerializablePrimitive =
                 std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    serialize(dataSerializablePrimitive, output);
  } else if (const auto&& dataSerializable =
                 std::dynamic_pointer_cast<DataSerializable>(obj)) {
    dataSerializableHandler_->serialize(dataSerializable, output, isDelta);
  } else if (const auto&& dataSerializableInternal =
                 std::dynamic_pointer_cast<DataSerializableInternal>(obj)) {
    serialize(dataSerializableInternal, output);
  } else {
    throw UnsupportedOperationException(
        "SerializationRegistry::serialize: Serialization type not "
        "implemented.");
  }
}

void SerializationRegistry::setPdxTypeHandler(PdxTypeHandler* handler) {
  pdxTypeHandler_ = std::unique_ptr<PdxTypeHandler>(handler);
}
void SerializationRegistry::setDataSerializableHandler(
    DataSerializableHandler* handler) {
  dataSerializableHandler_ = std::unique_ptr<DataSerializableHandler>(handler);
}

TypeFactoryMethod SerializationRegistry::getDataSerializableCreationMethod(
    int32_t objectId) {
  TypeFactoryMethod createType;
  theTypeMap_.findDataSerializable(objectId, createType);
  return createType;
}

int32_t SerializationRegistry::getIdForDataSerializableType(
    std::type_index objectType) const {
  auto&& typeIterator = theTypeMap_.typeToClassId_.find(objectType);
  return typeIterator->second;
}

DSCode SerializationRegistry::getSerializableDataDsCode(int32_t classId) {
  if (classId <= std::numeric_limits<int8_t>::max() &&
      classId >= std::numeric_limits<int8_t>::min()) {
    return DSCode::CacheableUserData;
  } else if (classId <= std::numeric_limits<int16_t>::max() &&
             classId >= std::numeric_limits<int16_t>::min()) {
    return DSCode::CacheableUserData2;
  } else {
    return DSCode::CacheableUserData4;
  }
}

void SerializationRegistry::serializeWithoutHeader(
    const std::shared_ptr<Serializable>& obj, DataOutput& output) const {
  if (const auto&& pdxSerializable =
          std::dynamic_pointer_cast<PdxSerializable>(obj)) {
    serializeWithoutHeader(pdxSerializable, output);
  } else if (const auto&& dataSerializableFixedId =
                 std::dynamic_pointer_cast<DataSerializableFixedId>(obj)) {
    serializeWithoutHeader(dataSerializableFixedId, output);
  } else if (const auto&& dataSerializablePrimitive =
                 std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    serializeWithoutHeader(dataSerializablePrimitive, output);
  } else if (const auto&& dataSerializable =
                 std::dynamic_pointer_cast<DataSerializable>(obj)) {
    serializeWithoutHeader(dataSerializable, output);
  } else if (const auto&& dataSerializableInternal =
                 std::dynamic_pointer_cast<DataSerializableInternal>(obj)) {
    serializeWithoutHeader(dataSerializableInternal, output);
  } else {
    throw UnsupportedOperationException(
        "SerializationRegistry::serializeWithoutHeader: Serialization type "
        "not implemented.");
  }
}

void SerializationRegistry::serialize(
    const std::shared_ptr<DataSerializableFixedId>& obj,
    DataOutput& output) const {
  auto id = static_cast<int32_t>(obj->getDSFID());
  if (id <= std::numeric_limits<int8_t>::max() &&
      id >= std::numeric_limits<int8_t>::min()) {
    output.write(static_cast<int8_t>(DSCode::FixedIDByte));
    output.write(static_cast<int8_t>(id));
  } else if (id <= std::numeric_limits<int16_t>::max() &&
             id >= std::numeric_limits<int16_t>::min()) {
    output.write(static_cast<int8_t>(DSCode::FixedIDShort));
    output.writeInt(static_cast<int16_t>(id));
  } else {
    output.write(static_cast<int8_t>(DSCode::FixedIDInt));
    output.writeInt(static_cast<int32_t>(id));
  }

  serializeWithoutHeader(obj, output);
}

void SerializationRegistry::serializeWithoutHeader(
    const std::shared_ptr<DataSerializableFixedId>& obj,
    DataOutput& output) const {
  obj->toData(output);
}

void SerializationRegistry::serialize(
    const std::shared_ptr<DataSerializablePrimitive>& obj,
    DataOutput& output) const {
  auto id = obj->getDsCode();
  output.write(static_cast<int8_t>(id));

  serializeWithoutHeader(obj, output);
}

void SerializationRegistry::serializeWithoutHeader(
    const std::shared_ptr<DataSerializablePrimitive>& obj,
    DataOutput& output) const {
  obj->toData(output);
}

void SerializationRegistry::serialize(
    const std::shared_ptr<PdxSerializable>& obj, DataOutput& output) const {
  output.write(static_cast<int8_t>(DSCode::PDX));
  serializeWithoutHeader(obj, output);
}

void SerializationRegistry::serialize(
    const std::shared_ptr<DataSerializableInternal>& obj,
    DataOutput& output) const {
  serializeWithoutHeader(obj, output);
}

void SerializationRegistry::serializeWithoutHeader(
    const std::shared_ptr<DataSerializableInternal>& obj,
    DataOutput& output) const {
  obj->toData(output);
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
      return pdxTypeHandler_->deserialize(input);
    }
    case DSCode::CacheableEnum: {
      auto enumObject = CacheableEnum::create(" ", " ", 0);
      enumObject->fromData(input);
      return std::move(enumObject);
    }
    case DSCode::CacheableUserData:
    case DSCode::CacheableUserData2:
    case DSCode::CacheableUserData4: {
      return dataSerializableHandler_->deserialize(input, dsCode);
    }
    case DSCode::FixedIDByte:
    case DSCode::FixedIDShort:
    case DSCode::FixedIDInt: {
      return deserializeDataSerializableFixedId(input, dsCode);
    }
    case DSCode::NullObj: {
      return nullptr;
    }
    case DSCode::FixedIDDefault:
    case DSCode::FixedIDNone:
    case DSCode::CacheableLinkedList:
    case DSCode::Properties:
    case DSCode::PdxType:
    case DSCode::BooleanArray:
    case DSCode::CharArray:
    case DSCode::CacheableString:
    case DSCode::Class:
    case DSCode::JavaSerializable:
    case DSCode::DataSerializable:
    case DSCode::CacheableBytes:
    case DSCode::CacheableInt16Array:
    case DSCode::CacheableInt32Array:
    case DSCode::CacheableInt64Array:
    case DSCode::CacheableFloatArray:
    case DSCode::CacheableDoubleArray:
    case DSCode::CacheableObjectArray:
    case DSCode::CacheableBoolean:
    case DSCode::CacheableCharacter:
    case DSCode::CacheableByte:
    case DSCode::CacheableInt16:
    case DSCode::CacheableInt32:
    case DSCode::CacheableInt64:
    case DSCode::CacheableFloat:
    case DSCode::CacheableDouble:
    case DSCode::CacheableDate:
    case DSCode::CacheableFileName:
    case DSCode::CacheableStringArray:
    case DSCode::CacheableArrayList:
    case DSCode::CacheableHashSet:
    case DSCode::CacheableHashMap:
    case DSCode::CacheableTimeUnit:
    case DSCode::CacheableHashTable:
    case DSCode::CacheableVector:
    case DSCode::CacheableIdentityHashMap:
    case DSCode::CacheableLinkedHashSet:
    case DSCode::CacheableStack:
    case DSCode::CacheableASCIIString:
    case DSCode::CacheableASCIIStringHuge:
    case DSCode::CacheableStringHuge:
    case DSCode::InternalDistributedMember:
      break;
  }

  TypeFactoryMethod createType = nullptr;

  theTypeMap_.findDataSerializablePrimitive(dsCode, createType);

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
    case DSCode::FixedIDDefault:
    case DSCode::FixedIDNone:
    case DSCode::CacheableLinkedList:
    case DSCode::Properties:
    case DSCode::PdxType:
    case DSCode::BooleanArray:
    case DSCode::CharArray:
    case DSCode::NullObj:
    case DSCode::CacheableString:
    case DSCode::Class:
    case DSCode::JavaSerializable:
    case DSCode::DataSerializable:
    case DSCode::CacheableBytes:
    case DSCode::CacheableInt16Array:
    case DSCode::CacheableInt32Array:
    case DSCode::CacheableInt64Array:
    case DSCode::CacheableFloatArray:
    case DSCode::CacheableDoubleArray:
    case DSCode::CacheableObjectArray:
    case DSCode::CacheableBoolean:
    case DSCode::CacheableCharacter:
    case DSCode::CacheableByte:
    case DSCode::CacheableInt16:
    case DSCode::CacheableInt32:
    case DSCode::CacheableInt64:
    case DSCode::CacheableFloat:
    case DSCode::CacheableDouble:
    case DSCode::CacheableDate:
    case DSCode::CacheableFileName:
    case DSCode::CacheableStringArray:
    case DSCode::CacheableArrayList:
    case DSCode::CacheableHashSet:
    case DSCode::CacheableHashMap:
    case DSCode::CacheableTimeUnit:
    case DSCode::CacheableNullString:
    case DSCode::CacheableHashTable:
    case DSCode::CacheableVector:
    case DSCode::CacheableIdentityHashMap:
    case DSCode::CacheableLinkedHashSet:
    case DSCode::CacheableStack:
    case DSCode::CacheableASCIIString:
    case DSCode::CacheableASCIIStringHuge:
    case DSCode::CacheableStringHuge:
    case DSCode::InternalDistributedMember:
    case DSCode::CacheableEnum:
    case DSCode::ClientProxyMembershipId:
    case DSCode::CacheableUserData:
    case DSCode::CacheableUserData4:
    case DSCode::PDX:
      throw IllegalStateException("Invalid fixed ID");
  }

  TypeFactoryMethod createType = nullptr;

  theTypeMap_.findDataSerializableFixedId(static_cast<DSFid>(fixedId),
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
  pdxTypeHandler_->serialize(obj, output);
}

void SerializationRegistry::addDataSerializableType(TypeFactoryMethod func,
                                                    int32_t id) {
  theTypeMap_.bindDataSerializable(func, id);
}

void SerializationRegistry::addPdxSerializableType(TypeFactoryMethodPdx func) {
  theTypeMap_.bindPdxSerializable(func);
}

void SerializationRegistry::removeDataSerializableType(int32_t id) {
  theTypeMap_.unbindDataSerializable(id);
}

void SerializationRegistry::addDataSerializableFixedIdType(
    TypeFactoryMethod func) {
  theTypeMap_.bindDataSerializableFixedId(func);
}

void SerializationRegistry::addDataSerializableFixedIdType(
    internal::DSFid id, TypeFactoryMethod func) {
  theTypeMap_.rebindDataSerializableFixedId(id, func);
}

void SerializationRegistry::removeDataSerializableFixeIdType(
    internal::DSFid id) {
  theTypeMap_.unbindDataSerializableFixedId(id);
}

void SerializationRegistry::setDataSerializablePrimitiveType(
    TypeFactoryMethod func, DSCode dsCode) {
  theTypeMap_.rebindDataSerializablePrimitive(dsCode, func);
}

std::shared_ptr<PdxSerializable> SerializationRegistry::getPdxSerializableType(
    const std::string& className) const {
  std::shared_ptr<PdxSerializable> pdxSerializable;

  if (auto typeFactoryMethodPdx = theTypeMap_.findPdxSerializable(className)) {
    pdxSerializable = typeFactoryMethodPdx();
  } else {
    pdxSerializable = std::make_shared<PdxWrapper>(nullptr, className);
  }

  return pdxSerializable;
}

void SerializationRegistry::setPdxSerializer(
    std::shared_ptr<PdxSerializer> serializer) {
  this->pdxSerializer_ = serializer;
}

std::shared_ptr<PdxSerializer> SerializationRegistry::getPdxSerializer() {
  return pdxSerializer_;
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
    auto result = poolDM->GetPDXTypeById(typeId);
    if (result == nullptr) {
      throw IllegalStateException("Unknown pdx type=" + std::to_string(typeId));
    }

    return result;
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
  const std::lock_guard<std::mutex> guard(dataSerializableMapMutex_);
  dataSerializableMap_.clear();

  const std::lock_guard<std::mutex> guard2(dataSerializableFixedIdMapMutex_);
  dataSerializableFixedIdMap_.clear();

  const std::lock_guard<std::mutex> guard3(pdxSerializableMapMutex_);
  pdxSerializableMap_.clear();
}

void TheTypeMap::findDataSerializable(int32_t id,
                                      TypeFactoryMethod& func) const {
  const std::lock_guard<std::mutex> guard(dataSerializableMapMutex_);
  const auto& found = dataSerializableMap_.find(id);
  if (found != dataSerializableMap_.end()) {
    func = found->second;
  }
}

void TheTypeMap::findDataSerializableFixedId(DSFid dsfid,
                                             TypeFactoryMethod& func) const {
  const std::lock_guard<std::mutex> guard(dataSerializableFixedIdMapMutex_);
  const auto& found = dataSerializableFixedIdMap_.find(dsfid);
  if (found != dataSerializableFixedIdMap_.end()) {
    func = found->second;
  }
}

void TheTypeMap::findDataSerializablePrimitive(DSCode dsCode,
                                               TypeFactoryMethod& func) const {
  const std::lock_guard<std::mutex> guard(dataSerializablePrimitiveMapMutex_);
  const auto& found = dataSerializablePrimitiveMap_.find(dsCode);
  if (found != dataSerializablePrimitiveMap_.end()) {
    func = found->second;
  }
}

void TheTypeMap::bindDataSerializable(TypeFactoryMethod func, int32_t id) {
  auto obj = func();

  if (const auto dataSerializable =
          std::dynamic_pointer_cast<DataSerializable>(obj)) {
    typeToClassId_.emplace(dataSerializable->getType(), id);
  } else {
    throw UnsupportedOperationException(
        "TheTypeMap::bind: Serialization type not implemented.");
  }

  const std::lock_guard<std::mutex> guard(dataSerializableMapMutex_);
  const auto& result = dataSerializableMap_.emplace(id, func);
  if (!result.second) {
    LOGERROR("A class with ID %d is already registered.", id);
    throw IllegalStateException("A class with given ID is already registered.");
  }
}

void TheTypeMap::rebindDataSerializable(int32_t id, TypeFactoryMethod func) {
  const std::lock_guard<std::mutex> guard(dataSerializableMapMutex_);
  dataSerializableMap_[id] = func;
}

void TheTypeMap::unbindDataSerializable(int32_t id) {
  const std::lock_guard<std::mutex> guard(dataSerializableMapMutex_);
  dataSerializableMap_.erase(id);
}

void TheTypeMap::bindDataSerializablePrimitive(TypeFactoryMethod func,
                                               DSCode dsCode) {
  const std::lock_guard<std::mutex> guard(dataSerializablePrimitiveMapMutex_);
  const auto& result = dataSerializablePrimitiveMap_.emplace(dsCode, func);
  if (!result.second) {
    LOGERROR("A class with DSCode %d is already registered.", dsCode);
    throw IllegalStateException(
        "A class with given DSCode is already registered.");
  }
}

void TheTypeMap::rebindDataSerializablePrimitive(DSCode dsCode,
                                                 TypeFactoryMethod func) {
  const std::lock_guard<std::mutex> guard(dataSerializablePrimitiveMapMutex_);
  dataSerializablePrimitiveMap_[dsCode] = func;
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

  const std::lock_guard<std::mutex> guard(dataSerializableFixedIdMapMutex_);
  const auto& result = dataSerializableFixedIdMap_.emplace(id, func);
  if (!result.second) {
    LOGERROR("A fixed class with ID %d is already registered.", id);
    throw IllegalStateException(
        "A fixed class with given ID is already registered.");
  }
}

void TheTypeMap::rebindDataSerializableFixedId(internal::DSFid id,
                                               TypeFactoryMethod func) {
  const std::lock_guard<std::mutex> guard(dataSerializableFixedIdMapMutex_);
  dataSerializableFixedIdMap_[id] = func;
}

void TheTypeMap::unbindDataSerializableFixedId(internal::DSFid id) {
  const std::lock_guard<std::mutex> guard(dataSerializableFixedIdMapMutex_);
  dataSerializableFixedIdMap_.erase(id);
}

void TheTypeMap::bindPdxSerializable(TypeFactoryMethodPdx func) {
  auto obj = func();
  const std::lock_guard<std::mutex> guard(pdxSerializableMapMutex_);
  auto&& objFullName = obj->getClassName();

  const auto& result = pdxSerializableMap_.emplace(objFullName, func);
  if (!result.second) {
    LOGERROR("A object with FullName " + objFullName +
             " is already registered.");
    throw IllegalStateException(
        "A Object with given FullName is already registered.");
  }
}

TypeFactoryMethodPdx TheTypeMap::findPdxSerializable(
    const std::string& objFullName) const {
  const std::lock_guard<std::mutex> guard(pdxSerializableMapMutex_);

  const auto& found = pdxSerializableMap_.find(objFullName);
  if (found != pdxSerializableMap_.end()) {
    return found->second;
  }

  return nullptr;
}

void TheTypeMap::rebindPdxSerializable(std::string objFullName,
                                       TypeFactoryMethodPdx func) {
  const std::lock_guard<std::mutex> guard(pdxSerializableMapMutex_);
  pdxSerializableMap_[objFullName] = func;
}

void TheTypeMap::unbindPdxSerializable(const std::string& objFullName) {
  const std::lock_guard<std::mutex> guard(pdxSerializableMapMutex_);
  pdxSerializableMap_.erase(objFullName);
}

void PdxTypeHandler::serialize(
    const std::shared_ptr<PdxSerializable>& serializable,
    DataOutput& dataOutput) const {
  auto instance = std::dynamic_pointer_cast<PdxInstanceImpl>(serializable);

  if (instance != nullptr) {
    PdxHelper::serializePdx(dataOutput, instance.get());
  } else {
    PdxHelper::serializePdx(dataOutput, serializable);
  }
}

std::shared_ptr<PdxSerializable> PdxTypeHandler::deserialize(
    DataInput& dataInput) const {
  return PdxHelper::deserializePdx(dataInput);
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
    case DSCode::FixedIDDefault:
    case DSCode::FixedIDByte:
    case DSCode::FixedIDInt:
    case DSCode::FixedIDNone:
    case DSCode::FixedIDShort:
    case DSCode::CacheableLinkedList:
    case DSCode::Properties:
    case DSCode::PdxType:
    case DSCode::BooleanArray:
    case DSCode::CharArray:
    case DSCode::NullObj:
    case DSCode::CacheableString:
    case DSCode::Class:
    case DSCode::JavaSerializable:
    case DSCode::DataSerializable:
    case DSCode::CacheableBytes:
    case DSCode::CacheableInt16Array:
    case DSCode::CacheableInt32Array:
    case DSCode::CacheableInt64Array:
    case DSCode::CacheableFloatArray:
    case DSCode::CacheableDoubleArray:
    case DSCode::CacheableObjectArray:
    case DSCode::CacheableBoolean:
    case DSCode::CacheableCharacter:
    case DSCode::CacheableByte:
    case DSCode::CacheableInt16:
    case DSCode::CacheableInt32:
    case DSCode::CacheableInt64:
    case DSCode::CacheableFloat:
    case DSCode::CacheableDouble:
    case DSCode::CacheableDate:
    case DSCode::CacheableFileName:
    case DSCode::CacheableStringArray:
    case DSCode::CacheableArrayList:
    case DSCode::CacheableHashSet:
    case DSCode::CacheableHashMap:
    case DSCode::CacheableTimeUnit:
    case DSCode::CacheableNullString:
    case DSCode::CacheableHashTable:
    case DSCode::CacheableVector:
    case DSCode::CacheableIdentityHashMap:
    case DSCode::CacheableLinkedHashSet:
    case DSCode::CacheableStack:
    case DSCode::CacheableASCIIString:
    case DSCode::CacheableASCIIStringHuge:
    case DSCode::CacheableStringHuge:
    case DSCode::InternalDistributedMember:
    case DSCode::CacheableEnum:
    case DSCode::PDX:
      IllegalStateException("Invalid DS Code.");
  }

  if (isDelta) {
    const Delta* ptr = dynamic_cast<const Delta*>(dataSerializable.get());
    ptr->toDelta(dataOutput);
  } else {
    dataSerializable->toData(dataOutput);
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
    case DSCode::FixedIDDefault:
    case DSCode::FixedIDByte:
    case DSCode::FixedIDInt:
    case DSCode::FixedIDNone:
    case DSCode::FixedIDShort:
    case DSCode::CacheableLinkedList:
    case DSCode::Properties:
    case DSCode::PdxType:
    case DSCode::BooleanArray:
    case DSCode::CharArray:
    case DSCode::NullObj:
    case DSCode::CacheableString:
    case DSCode::Class:
    case DSCode::JavaSerializable:
    case DSCode::DataSerializable:
    case DSCode::CacheableBytes:
    case DSCode::CacheableInt16Array:
    case DSCode::CacheableInt32Array:
    case DSCode::CacheableInt64Array:
    case DSCode::CacheableFloatArray:
    case DSCode::CacheableDoubleArray:
    case DSCode::CacheableObjectArray:
    case DSCode::CacheableBoolean:
    case DSCode::CacheableCharacter:
    case DSCode::CacheableByte:
    case DSCode::CacheableInt16:
    case DSCode::CacheableInt32:
    case DSCode::CacheableInt64:
    case DSCode::CacheableFloat:
    case DSCode::CacheableDouble:
    case DSCode::CacheableDate:
    case DSCode::CacheableFileName:
    case DSCode::CacheableStringArray:
    case DSCode::CacheableArrayList:
    case DSCode::CacheableHashSet:
    case DSCode::CacheableHashMap:
    case DSCode::CacheableTimeUnit:
    case DSCode::CacheableNullString:
    case DSCode::CacheableHashTable:
    case DSCode::CacheableVector:
    case DSCode::CacheableIdentityHashMap:
    case DSCode::CacheableLinkedHashSet:
    case DSCode::CacheableStack:
    case DSCode::CacheableASCIIString:
    case DSCode::CacheableASCIIStringHuge:
    case DSCode::CacheableStringHuge:
    case DSCode::InternalDistributedMember:
    case DSCode::CacheableEnum:
    case DSCode::PDX:
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
