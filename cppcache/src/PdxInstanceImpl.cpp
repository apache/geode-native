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

#include "PdxInstanceImpl.hpp"

#include <algorithm>

#include <geode/Cache.hpp>
#include <geode/PdxFieldTypes.hpp>
#include <geode/PdxReader.hpp>
#include <geode/internal/DataSerializablePrimitive.hpp>

#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "PdxHelper.hpp"
#include "Utils.hpp"
#include "util/string.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializablePrimitive;

int8_t PdxInstanceImpl::m_BooleanDefaultBytes[] = {0};
int8_t PdxInstanceImpl::m_ByteDefaultBytes[] = {0};
int8_t PdxInstanceImpl::m_ShortDefaultBytes[] = {0, 0};
int8_t PdxInstanceImpl::m_CharDefaultBytes[] = {0, 0};
int8_t PdxInstanceImpl::m_IntDefaultBytes[] = {0, 0, 0, 0};
int8_t PdxInstanceImpl::m_LongDefaultBytes[] = {0, 0, 0, 0, 0, 0, 0, 0};
int8_t PdxInstanceImpl::m_FloatDefaultBytes[] = {0, 0, 0, 0};
int8_t PdxInstanceImpl::m_DoubleDefaultBytes[] = {0, 0, 0, 0, 0, 0, 0, 0};
int8_t PdxInstanceImpl::m_DateDefaultBytes[] = {-1, -1, -1, -1, -1, -1, -1, -1};
int8_t PdxInstanceImpl::m_StringDefaultBytes[] = {
    static_cast<int8_t>(apache::geode::client::DSCode::CacheableNullString)};
int8_t PdxInstanceImpl::m_ObjectDefaultBytes[] = {
    static_cast<int8_t>(apache::geode::client::DSCode::NullObj)};
int8_t PdxInstanceImpl::m_NULLARRAYDefaultBytes[] = {-1};
std::shared_ptr<PdxFieldType> PdxInstanceImpl::m_DefaultPdxFieldType(
    new PdxFieldType("default", "default", PdxFieldTypes::UNKNOWN,
                     -1 /*field index*/, false, 1, -1 /*var len field idx*/));

PdxInstanceImpl::~PdxInstanceImpl() noexcept {}

PdxInstanceImpl::PdxInstanceImpl(const uint8_t* buffer, size_t length,
                                 std::shared_ptr<PdxType> pdxType,
                                 CachePerfStats& cacheStats,
                                 PdxTypeRegistry& pdxTypeRegistry,
                                 const CacheImpl& cacheImpl,
                                 bool enableTimeStatistics)
    : m_buffer(buffer, buffer + length),
      m_typeId(pdxType->getTypeId()),
      m_pdxType(pdxType),
      m_cacheStats(cacheStats),
      m_pdxTypeRegistry(pdxTypeRegistry),
      m_cacheImpl(cacheImpl),
      m_enableTimeStatistics(enableTimeStatistics) {
  cacheStats.incPdxInstanceCreations();
  LOGDEBUG("PdxInstanceImpl::m_bufferLength = %zu ", m_buffer.size());
}

PdxInstanceImpl::PdxInstanceImpl(FieldVsValues fieldVsValue,
                                 std::shared_ptr<PdxType> pdxType,
                                 CachePerfStats& cacheStats,
                                 PdxTypeRegistry& pdxTypeRegistry,
                                 const CacheImpl& cacheImpl,
                                 bool enableTimeStatistics)
    : m_typeId(0),
      m_pdxType(pdxType),
      m_updatedFields(fieldVsValue),
      m_cacheStats(cacheStats),
      m_pdxTypeRegistry(pdxTypeRegistry),
      m_cacheImpl(cacheImpl),
      m_enableTimeStatistics(enableTimeStatistics) {
  cacheStats.incPdxInstanceCreations();
  m_pdxType->InitializeType();  // to generate static position map
}

void PdxInstanceImpl::writeField(PdxWriter& writer, const std::string& name,
                                 PdxFieldTypes typeId,
                                 std::shared_ptr<Cacheable> value) {
  switch (typeId) {
    case PdxFieldTypes::INT: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt32>(value)) {
        writer.writeInt(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::STRING: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableString>(value)) {
        writer.writeString(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::BOOLEAN: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableBoolean>(value)) {
        writer.writeBoolean(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::FLOAT: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableFloat>(value)) {
        writer.writeFloat(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::DOUBLE: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableDouble>(value)) {
        writer.writeDouble(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::CHAR: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableCharacter>(value)) {
        writer.writeChar(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::BYTE: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableByte>(value)) {
        writer.writeByte(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::SHORT: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt16>(value)) {
        writer.writeShort(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::LONG: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt64>(value)) {
        writer.writeLong(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::BYTE_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableBytes>(value)) {
        writer.writeByteArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::DOUBLE_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableDoubleArray>(value)) {
        writer.writeDoubleArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::FLOAT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableFloatArray>(value)) {
        writer.writeFloatArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::SHORT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt16Array>(value)) {
        writer.writeShortArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::INT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt32Array>(value)) {
        writer.writeIntArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::LONG_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt64Array>(value)) {
        writer.writeLongArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::BOOLEAN_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<BooleanArray>(value)) {
        writer.writeBooleanArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::CHAR_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CharArray>(value)) {
        writer.writeCharArray(name, val->value());
      }
      break;
    }
    case PdxFieldTypes::STRING_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableStringArray>(value)) {
        auto size = val->length();
        std::vector<std::string> strings;
        strings.reserve(size);
        for (int item = 0; item < size; item++) {
          strings.push_back((*val)[item]->value());
        }
        writer.writeStringArray(name, strings);
      }
      break;
    }
    case PdxFieldTypes::DATE: {
      if (auto&& date = std::dynamic_pointer_cast<CacheableDate>(value)) {
        writer.writeDate(name, date);
      }
      break;
    }
    case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
      if (auto&& vector = std::dynamic_pointer_cast<CacheableVector>(value)) {
        const auto size = vector->size();
        int8_t** values = new int8_t*[size];
        auto lengths = new int[size];
        size_t i = 0;
        for (auto&& entry : *vector) {
          if (auto&& val = std::dynamic_pointer_cast<CacheableBytes>(entry)) {
            values[i] = const_cast<int8_t*>(
                reinterpret_cast<const int8_t*>(val->value().data()));
            lengths[i] = val->length();
          }
          i++;
        }
        writer.writeArrayOfByteArrays(name, values, static_cast<int>(size),
                                      lengths);
        delete[] values;
        delete[] lengths;
      }
      break;
    }
    case PdxFieldTypes::OBJECT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableObjectArray>(value)) {
        writer.writeObjectArray(name, val);
      }
      break;
    }
    case PdxFieldTypes::UNKNOWN:
    case PdxFieldTypes::OBJECT: {
      writer.writeObject(name, value);
    }
  }
}
std::shared_ptr<WritablePdxInstance> PdxInstanceImpl::createWriter() {
  LOGDEBUG("PdxInstanceImpl::createWriter m_bufferLength = %zu m_typeId = %d ",
           m_buffer.size(), m_typeId);
  const auto& stream = getPdxStream();
  return std::make_shared<PdxInstanceImpl>(
      stream.data(), stream.size(), m_pdxType, m_cacheStats, m_pdxTypeRegistry,
      m_cacheImpl,
      m_enableTimeStatistics);  // need to create duplicate byte stream);
}

bool PdxInstanceImpl::enumerateObjectArrayEquals(
    std::shared_ptr<CacheableObjectArray> Obj,
    std::shared_ptr<CacheableObjectArray> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }

  for (size_t i = 0; i < Obj->size(); i++) {
    if (!deepArrayEquals(Obj->at(i), OtherObj->at(i))) {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::enumerateVectorEquals(
    std::shared_ptr<CacheableVector> Obj,
    std::shared_ptr<CacheableVector> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }

  for (size_t i = 0; i < Obj->size(); i++) {
    if (!deepArrayEquals(Obj->at(i), OtherObj->at(i))) {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::enumerateArrayListEquals(
    std::shared_ptr<CacheableArrayList> Obj,
    std::shared_ptr<CacheableArrayList> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }

  for (size_t i = 0; i < Obj->size(); i++) {
    if (!deepArrayEquals(Obj->at(i), OtherObj->at(i))) {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::enumerateMapEquals(
    std::shared_ptr<CacheableHashMap> Obj,
    std::shared_ptr<CacheableHashMap> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }

  for (const auto& iter : *Obj) {
    const auto& otherIter = OtherObj->find(iter.first);
    if (otherIter != OtherObj->end()) {
      if (!deepArrayEquals(iter.second, otherIter->second)) {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::enumerateHashTableEquals(
    std::shared_ptr<CacheableHashTable> Obj,
    std::shared_ptr<CacheableHashTable> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }

  for (const auto& iter : *Obj) {
    const auto& otherIter = OtherObj->find(iter.first);
    if (otherIter != OtherObj->end()) {
      if (!deepArrayEquals(iter.second, otherIter->second)) {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::enumerateSetEquals(
    std::shared_ptr<CacheableHashSet> Obj,
    std::shared_ptr<CacheableHashSet> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }
  for (const auto& iter : *Obj) {
    if (OtherObj->find(iter) == OtherObj->end()) {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::enumerateLinkedSetEquals(
    std::shared_ptr<CacheableLinkedHashSet> Obj,
    std::shared_ptr<CacheableLinkedHashSet> OtherObj) {
  if (Obj == nullptr && OtherObj == nullptr) {
    return true;
  } else if (Obj == nullptr && OtherObj != nullptr) {
    return false;
  } else if (Obj != nullptr && OtherObj == nullptr) {
    return false;
  }

  if (Obj->size() != OtherObj->size()) {
    return false;
  }
  for (const auto& iter : *Obj) {
    if (OtherObj->find(iter) == OtherObj->end()) {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::deepArrayEquals(std::shared_ptr<Cacheable> obj,
                                      std::shared_ptr<Cacheable> otherObj) {
  if (obj == nullptr && otherObj == nullptr) {
    return true;
  } else if (obj == nullptr && otherObj != nullptr) {
    return false;
  } else if (obj != nullptr && otherObj == nullptr) {
    return false;
  }

  if (auto primitive =
          std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    switch (primitive->getDsCode()) {
      case DSCode::CacheableObjectArray: {
        auto objArrayPtr = std::dynamic_pointer_cast<CacheableObjectArray>(obj);
        auto otherObjArrayPtr =
            std::dynamic_pointer_cast<CacheableObjectArray>(otherObj);
        return enumerateObjectArrayEquals(objArrayPtr, otherObjArrayPtr);
      }
      case DSCode::CacheableVector: {
        auto vec = std::dynamic_pointer_cast<CacheableVector>(obj);
        auto otherVec = std::dynamic_pointer_cast<CacheableVector>(otherObj);
        return enumerateVectorEquals(vec, otherVec);
      }
      case DSCode::CacheableArrayList: {
        auto arrList = std::dynamic_pointer_cast<CacheableArrayList>(obj);
        auto otherArrList =
            std::dynamic_pointer_cast<CacheableArrayList>(otherObj);
        return enumerateArrayListEquals(arrList, otherArrList);
      }
      case DSCode::CacheableHashMap: {
        auto map = std::dynamic_pointer_cast<CacheableHashMap>(obj);
        auto otherMap = std::dynamic_pointer_cast<CacheableHashMap>(otherObj);
        return enumerateMapEquals(map, otherMap);
      }
      case DSCode::CacheableHashSet: {
        auto hashset = std::dynamic_pointer_cast<CacheableHashSet>(obj);
        auto otherHashset =
            std::dynamic_pointer_cast<CacheableHashSet>(otherObj);
        return enumerateSetEquals(hashset, otherHashset);
      }
      case DSCode::CacheableLinkedHashSet: {
        auto linkedHashset =
            std::dynamic_pointer_cast<CacheableLinkedHashSet>(obj);
        auto otherLinkedHashset =
            std::dynamic_pointer_cast<CacheableLinkedHashSet>(otherObj);
        return enumerateLinkedSetEquals(linkedHashset, otherLinkedHashset);
      }
      case DSCode::CacheableHashTable: {
        auto hashTable = std::dynamic_pointer_cast<CacheableHashTable>(obj);
        auto otherhashTable =
            std::dynamic_pointer_cast<CacheableHashTable>(otherObj);
        return enumerateHashTableEquals(hashTable, otherhashTable);
      }
      case DSCode::FixedIDDefault:
      case DSCode::FixedIDByte:
      case DSCode::FixedIDShort:
      case DSCode::FixedIDInt:
      case DSCode::FixedIDNone:
      case DSCode::CacheableLinkedList:
      case DSCode::Properties:
      case DSCode::PdxType:
      case DSCode::BooleanArray:
      case DSCode::CharArray:
      case DSCode::CacheableUserData4:
      case DSCode::ClientProxyMembershipId:
      case DSCode::CacheableUserData:
      case DSCode::NullObj:
      case DSCode::Class:
      case DSCode::JavaSerializable:
      case DSCode::DataSerializable:
      case DSCode::CacheableBytes:
      case DSCode::CacheableInt16Array:
      case DSCode::CacheableInt32Array:
      case DSCode::CacheableInt64Array:
      case DSCode::CacheableFloatArray:
      case DSCode::CacheableDoubleArray:
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
      case DSCode::CacheableTimeUnit:
      case DSCode::CacheableIdentityHashMap:
      case DSCode::CacheableStack:
      case DSCode::InternalDistributedMember:
      case DSCode::PDX:
      case DSCode::CacheableEnum:
      case DSCode::CacheableString:
      case DSCode::CacheableNullString:
      case DSCode::CacheableASCIIString:
      case DSCode::CacheableASCIIStringHuge:
      case DSCode::CacheableStringHuge:
        break;
    }
  }

  if (auto pdxInstance = std::dynamic_pointer_cast<PdxInstance>(obj)) {
    if (auto otherPdxInstance =
            std::dynamic_pointer_cast<PdxInstance>(otherObj)) {
      return *pdxInstance == *otherPdxInstance;
    }
  }

  if (auto keyType = std::dynamic_pointer_cast<CacheableKey>(obj)) {
    if (auto otherKeyType = std::dynamic_pointer_cast<CacheableKey>(otherObj)) {
      return *keyType == *otherKeyType;
    }
  }

  throw IllegalStateException(
      "PdxInstance cannot calculate equals of the field " + obj->toString() +
      " since equals is only supported for CacheableKey derived types.");
}

int PdxInstanceImpl::enumerateMapHashCode(
    std::shared_ptr<CacheableHashMap> map) {
  int h = 0;
  for (const auto& itr : *map) {
    h = h + ((deepArrayHashCode(itr.first)) ^
             ((itr.second) ? deepArrayHashCode(itr.second) : 0));
  }
  return h;
}

int PdxInstanceImpl::enumerateSetHashCode(
    std::shared_ptr<CacheableHashSet> set) {
  int h = 0;
  for (const auto& itr : *set) {
    h = h + deepArrayHashCode(itr);
  }
  return h;
}

int PdxInstanceImpl::enumerateLinkedSetHashCode(
    std::shared_ptr<CacheableLinkedHashSet> set) {
  int h = 0;
  for (const auto& itr : *set) {
    h = h + deepArrayHashCode(itr);
  }
  return h;
}

int PdxInstanceImpl::enumerateHashTableCode(
    std::shared_ptr<CacheableHashTable> hashTable) {
  int h = 0;
  for (const auto& itr : *hashTable) {
    h = h + ((deepArrayHashCode(itr.first)) ^
             ((itr.second) ? deepArrayHashCode(itr.second) : 0));
  }
  return h;
}

int PdxInstanceImpl::enumerateObjectArrayHashCode(
    std::shared_ptr<CacheableObjectArray> objArray) {
  int h = 1;
  for (const auto& obj : *objArray) {
    h = h * 31 + deepArrayHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::enumerateVectorHashCode(
    std::shared_ptr<CacheableVector> vec) {
  int h = 1;
  for (const auto& obj : *vec) {
    h = h * 31 + deepArrayHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::enumerateArrayListHashCode(
    std::shared_ptr<CacheableArrayList> arrList) {
  int h = 1;
  for (const auto& obj : *arrList) {
    h = h * 31 + deepArrayHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::enumerateLinkedListHashCode(
    std::shared_ptr<CacheableLinkedList> linkedList) {
  int h = 1;
  for (const auto& obj : *linkedList) {
    h = h * 31 + deepArrayHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::deepArrayHashCode(std::shared_ptr<Cacheable> obj) {
  if (obj == nullptr) {
    return 0;
  }

  if (auto primitive =
          std::dynamic_pointer_cast<DataSerializablePrimitive>(obj)) {
    switch (primitive->getDsCode()) {
      case DSCode::CacheableObjectArray: {
        return enumerateObjectArrayHashCode(
            std::dynamic_pointer_cast<CacheableObjectArray>(obj));
      }
      case DSCode::CacheableVector: {
        return enumerateVectorHashCode(
            std::dynamic_pointer_cast<CacheableVector>(obj));
      }
      case DSCode::CacheableArrayList: {
        return enumerateArrayListHashCode(
            std::dynamic_pointer_cast<CacheableArrayList>(obj));
      }
      case DSCode::CacheableLinkedList: {
        return enumerateLinkedListHashCode(
            std::dynamic_pointer_cast<CacheableLinkedList>(obj));
      }
      case DSCode::CacheableHashMap: {
        return enumerateMapHashCode(
            std::dynamic_pointer_cast<CacheableHashMap>(obj));
      }
      case DSCode::CacheableHashSet: {
        return enumerateSetHashCode(
            std::dynamic_pointer_cast<CacheableHashSet>(obj));
      }
      case DSCode::CacheableLinkedHashSet: {
        auto linkedHashSet =
            std::dynamic_pointer_cast<CacheableLinkedHashSet>(obj);
        return enumerateLinkedSetHashCode(linkedHashSet);
      }
      case DSCode::CacheableHashTable: {
        return enumerateHashTableCode(
            std::dynamic_pointer_cast<CacheableHashTable>(obj));
      }
      case DSCode::FixedIDDefault:
      case DSCode::FixedIDByte:
      case DSCode::FixedIDInt:
      case DSCode::FixedIDNone:
      case DSCode::FixedIDShort:
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
      case DSCode::CacheableTimeUnit:
      case DSCode::CacheableNullString:
      case DSCode::CacheableIdentityHashMap:
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
        break;
    }
  }

  if (auto pdxInstance = std::dynamic_pointer_cast<PdxInstance>(obj)) {
    return pdxInstance->hashcode();
  }

  if (auto keyType = std::dynamic_pointer_cast<CacheableKey>(obj)) {
    return keyType->hashcode();
  }

  throw IllegalStateException(
      "PdxInstance cannot calculate hashcode of the field " + obj->toString() +
      " since equals is only supported for CacheableKey derived types.");
}

int32_t PdxInstanceImpl::hashcode() const {
  const auto& stream = getPdxStream();
  auto input = m_cacheImpl.createDataInput(stream.data(), stream.size());

  int hashCode = 1;
  for (const auto& field : getIdentityPdxFields()) {
    LOGDEBUG("hashcode for pdxfield %s  hashcode is %d ",
             field->getFieldName().c_str(), hashCode);
    switch (field->getTypeId()) {
      case PdxFieldTypes::CHAR:
      case PdxFieldTypes::BOOLEAN:
      case PdxFieldTypes::BYTE:
      case PdxFieldTypes::SHORT:
      case PdxFieldTypes::INT:
      case PdxFieldTypes::LONG:
      case PdxFieldTypes::DATE:
      case PdxFieldTypes::FLOAT:
      case PdxFieldTypes::DOUBLE:
      case PdxFieldTypes::STRING:
      case PdxFieldTypes::BOOLEAN_ARRAY:
      case PdxFieldTypes::CHAR_ARRAY:
      case PdxFieldTypes::BYTE_ARRAY:
      case PdxFieldTypes::SHORT_ARRAY:
      case PdxFieldTypes::INT_ARRAY:
      case PdxFieldTypes::LONG_ARRAY:
      case PdxFieldTypes::FLOAT_ARRAY:
      case PdxFieldTypes::DOUBLE_ARRAY:
      case PdxFieldTypes::STRING_ARRAY:
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
        int retH = getRawHashCode(input, field);
        if (retH != 0) {
          hashCode = 31 * hashCode + retH;
        }
        break;
      }
      case PdxFieldTypes::OBJECT: {
        setOffsetForObject(input, field->getSequenceId());
        std::shared_ptr<Cacheable> object = nullptr;
        input.readObject(object);
        if (object != nullptr) {
          hashCode = 31 * hashCode + deepArrayHashCode(object);
        }
        break;
      }
      case PdxFieldTypes::OBJECT_ARRAY: {
        setOffsetForObject(input, field->getSequenceId());
        auto objectArray = CacheableObjectArray::create();
        objectArray->fromData(input);
        hashCode =
            31 * hashCode +
            ((objectArray != nullptr) ? deepArrayHashCode(objectArray) : 0);
        break;
      }
      case PdxFieldTypes::UNKNOWN: {
        throw IllegalStateException(
            "PdxInstance not found typeid " +
            std::to_string(static_cast<int>(field->getTypeId())));
      }
    }
  }

  return hashCode;
}

int PdxInstanceImpl::getTypeId() const { return m_typeId; }

std::shared_ptr<PdxType> PdxInstanceImpl::getPdxType(Pool* pool) const {
  auto registry = m_cacheImpl.getPdxTypeRegistry();
  auto type = registry->getPdxType(m_typeId);
  if (type != nullptr) {
    return type;
  }

  auto id = registry->getPDXIdForType(m_pdxType, pool);
  m_pdxType->setTypeId(id);
  m_typeId = id;

  return m_pdxType;
}

void PdxInstanceImpl::updatePdxStream(std::vector<uint8_t> stream) {
  m_buffer = std::move(stream);
}

const std::vector<uint8_t>& PdxInstanceImpl::getPdxStream() const {
  if (m_buffer.empty()) {
    auto output = m_cacheImpl.createDataOutput();
    PdxLocalWriter plw{output, m_pdxType, m_cacheImpl.getPdxTypeRegistry()};

    toData(plw);
    plw.endObjectWriting();
    m_buffer = plw.getPdxStream();
  }

  return m_buffer;
}

bool PdxInstanceImpl::isIdentityField(const std::string& name) {
  auto pft = m_pdxType->getPdxField(name);
  return pft != nullptr && pft->getIdentityField();
}

bool PdxInstanceImpl::hasField(const std::string& name) {
  return m_pdxType->getPdxField(name) != nullptr;
}

bool PdxInstanceImpl::getBooleanField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readBoolean();
}

int8_t PdxInstanceImpl::getByteField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.read();
}

int16_t PdxInstanceImpl::getShortField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readInt16();
}

int32_t PdxInstanceImpl::getIntField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readInt32();
}

int64_t PdxInstanceImpl::getLongField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readInt64();
}

float PdxInstanceImpl::getFloatField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readFloat();
}

double PdxInstanceImpl::getDoubleField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readDouble();
}

char16_t PdxInstanceImpl::getCharField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readInt16();
}

std::string PdxInstanceImpl::getStringField(const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readString();
}

std::vector<bool> PdxInstanceImpl::getBooleanArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readBooleanArray();
}

std::vector<int8_t> PdxInstanceImpl::getByteArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readByteArray();
}

std::vector<int16_t> PdxInstanceImpl::getShortArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readShortArray();
}

std::vector<int32_t> PdxInstanceImpl::getIntArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readIntArray();
}

std::vector<int64_t> PdxInstanceImpl::getLongArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readLongArray();
}

std::vector<float> PdxInstanceImpl::getFloatArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readFloatArray();
}

std::vector<double> PdxInstanceImpl::getDoubleArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readDoubleArray();
}

std::vector<char16_t> PdxInstanceImpl::getCharArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readCharArray();
}

std::vector<std::string> PdxInstanceImpl::getStringArrayField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  return input.readStringArray();
}

std::shared_ptr<CacheableDate> PdxInstanceImpl::getCacheableDateField(
    const std::string& name) const {
  auto input = getDataInputForField(name);
  auto value = CacheableDate::create();
  value->fromData(input);
  return value;
}

std::shared_ptr<Cacheable> PdxInstanceImpl::getCacheableField(
    const std::string& name) const {
  auto dataInput = getDataInputForField(name);
  std::shared_ptr<Cacheable> value;
  dataInput.readObject(value);
  return value;
}
std::shared_ptr<CacheableObjectArray>
PdxInstanceImpl::getCacheableObjectArrayField(const std::string& name) const {
  auto dataInput = getDataInputForField(name);
  auto value = CacheableObjectArray::create();
  value->fromData(dataInput);
  return value;
}

void PdxInstanceImpl::getField(const std::string& name, int8_t*** value,
                               int32_t& arrayLength,
                               int32_t*& elementLength) const {
  auto dataInput = getDataInputForField(name);
  dataInput.readArrayOfByteArrays(value, arrayLength, &elementLength);
}

std::string PdxInstanceImpl::toString() const {
  std::string result = "PDX[" + std::to_string(m_pdxType->getTypeId()) + "," +
                       m_pdxType->getPdxClassName() + "]{";
  bool firstElement = true;
  auto identityFields = getIdentityPdxFields();
  for (size_t i = 0; i < identityFields.size(); i++) {
    if (firstElement) {
      firstElement = false;
    } else {
      result += ",";
    }
    result += identityFields.at(i)->getFieldName();
    result += "=";

    switch (identityFields.at(i)->getTypeId()) {
      case PdxFieldTypes::BOOLEAN: {
        auto&& value = getBooleanField(identityFields.at(i)->getFieldName());
        result += value ? "true" : "false";
        break;
      }
      case PdxFieldTypes::BYTE: {
        auto&& value = getByteField(identityFields.at(i)->getFieldName());
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::SHORT: {
        int16_t value = getShortField(identityFields.at(i)->getFieldName());
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::INT: {
        int32_t value = getIntField(identityFields.at(i)->getFieldName());
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::LONG: {
        int64_t value = getLongField(identityFields.at(i)->getFieldName());
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::FLOAT: {
        float value = getFloatField(identityFields.at(i)->getFieldName());
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::DOUBLE: {
        double value = getDoubleField(identityFields.at(i)->getFieldName());
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::CHAR: {
        auto value = getCharField(identityFields.at(i)->getFieldName());
        result += to_utf8(std::u16string{value});
        break;
      }
      case PdxFieldTypes::STRING: {
        auto value = getStringField(identityFields.at(i)->getFieldName());
        result += value;
        break;
      }
      case PdxFieldTypes::CHAR_ARRAY: {
        auto value = getCharArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          result += to_utf8(std::u16string(value.data(), length));
        }
        break;
      }
      case PdxFieldTypes::STRING_ARRAY: {
        auto value = getStringArrayField(identityFields.at(i)->getFieldName());
        for (auto&& v : value) {
          result += v;
        }
        break;
      }
      case PdxFieldTypes::BYTE_ARRAY: {
        auto value = getByteArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::SHORT_ARRAY: {
        auto value = getShortArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::INT_ARRAY: {
        auto value = getIntArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::LONG_ARRAY: {
        auto value = getLongArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::FLOAT_ARRAY: {
        auto value = getFloatArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::DOUBLE_ARRAY: {
        auto value = getDoubleArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::DATE: {
        auto value =
            getCacheableDateField(identityFields.at(i)->getFieldName());
        if (value != nullptr) {
          result += value->toString().c_str();
        }
        break;
      }
      case PdxFieldTypes::BOOLEAN_ARRAY: {
        auto value = getBooleanArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += v ? "true" : "false";
          }
        }
        break;
      }
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
        int8_t** value = nullptr;
        int32_t arrayLength;
        int32_t* elementLength;
        getField(identityFields.at(i)->getFieldName(), &value, arrayLength,
                 elementLength);
        if (arrayLength > 0) {
          for (int j = 0; j < arrayLength; j++) {
            for (int k = 0; k < elementLength[j]; k++) {
              result += std::to_string(value[j][k]);
            }
          }
        }
        break;
      }
      case PdxFieldTypes::OBJECT_ARRAY: {
        auto value =
            getCacheableObjectArrayField(identityFields.at(i)->getFieldName());
        if (value != nullptr) {
          result += value->toString().c_str();
        }
        break;
      }
      case PdxFieldTypes::OBJECT:
      case PdxFieldTypes::UNKNOWN: {
        auto value = getCacheableField(identityFields.at(i)->getFieldName());
        if (value != nullptr) {
          result += value->toString().c_str();
        }
      }
    }
  }
  result += "}";

  return result;
}

std::shared_ptr<PdxSerializable> PdxInstanceImpl::getObject() {
  const auto& stream = getPdxStream();
  auto len = stream.size();

  auto input = m_cacheImpl.createDataInput(stream.data(), len);
  getPdxType(DataInputInternal::getPool(input));

  int64_t sampleStartNanos =
      m_enableTimeStatistics ? Utils::startStatOpTime() : 0;

  //[ToDo] do we have to call incPdxDeSerialization here?
  auto ret =
      PdxHelper::deserializePdx(input, m_typeId, static_cast<int32_t>(len));

  if (m_enableTimeStatistics) {
    Utils::updateStatOpTime(m_cacheStats.getStat(),
                            m_cacheStats.getPdxInstanceDeserializationTimeId(),
                            sampleStartNanos);
  }

  m_cacheStats.incPdxInstanceDeserializations();
  return ret;
}

void PdxInstanceImpl::equatePdxFields(
    std::vector<std::shared_ptr<PdxFieldType>>& my,
    std::vector<std::shared_ptr<PdxFieldType>>& other) const {
  int otherIdx = -1;
  for (int32_t i = 0; i < static_cast<int32_t>(my.size()); i++) {
    auto myF = my.at(i);
    if (!myF->equals(m_DefaultPdxFieldType)) {
      for (int32_t j = 0; j < static_cast<int32_t>(other.size()); j++) {
        if (myF->equals(other[j])) {
          otherIdx = j;
          break;
        } else {
          otherIdx = -1;
        }
      }

      if (otherIdx == -1)  // field not there
      {
        if (i < static_cast<int32_t>(other.size())) {
          auto tmp = other.at(i);
          other.at(i) = m_DefaultPdxFieldType;
          other.push_back(tmp);
        } else {
          other.push_back(m_DefaultPdxFieldType);
        }
      } else if (otherIdx != i) {
        auto tmp = other.at(i);
        other.at(i) = other.at(otherIdx);
        other.at(otherIdx) = tmp;
      }
    }
  }
}

bool PdxInstanceImpl::operator==(const CacheableKey& o) const {
  PdxInstanceImpl* other =
      dynamic_cast<PdxInstanceImpl*>(const_cast<CacheableKey*>(&o));

  if (other == nullptr) {
    return false;
  }

  auto otherType = other->m_pdxType;
  if (m_pdxType->getPdxClassName() != otherType->getPdxClassName()) {
    return false;
  }

  auto identityFields = getIdentityPdxFields();
  auto otherIdentityFields = other->getIdentityPdxFields();

  equatePdxFields(identityFields, otherIdentityFields);
  equatePdxFields(otherIdentityFields, identityFields);

  auto stream = getPdxStream();
  auto otherStream = other->getPdxStream();

  auto input = m_cacheImpl.createDataInput(stream.data(), stream.size());
  auto otherInput =
      m_cacheImpl.createDataInput(otherStream.data(), otherStream.size());

  PdxFieldTypes fieldTypeId;
  for (size_t i = 0; i < identityFields.size(); i++) {
    auto myPFT = identityFields.at(i);
    auto otherPFT = otherIdentityFields.at(i);

    LOGDEBUG("pdxfield %s ",
             ((myPFT != m_DefaultPdxFieldType) ? myPFT->getFieldName()
                                               : otherPFT->getFieldName())
                 .c_str());
    if (myPFT->equals(m_DefaultPdxFieldType)) {
      fieldTypeId = otherPFT->getTypeId();
    } else if (otherPFT->equals(m_DefaultPdxFieldType)) {
      fieldTypeId = myPFT->getTypeId();
    } else {
      fieldTypeId = myPFT->getTypeId();
    }

    switch (fieldTypeId) {
      case PdxFieldTypes::CHAR:
      case PdxFieldTypes::BOOLEAN:
      case PdxFieldTypes::BYTE:
      case PdxFieldTypes::SHORT:
      case PdxFieldTypes::INT:
      case PdxFieldTypes::LONG:
      case PdxFieldTypes::DATE:
      case PdxFieldTypes::FLOAT:
      case PdxFieldTypes::DOUBLE:
      case PdxFieldTypes::STRING:
      case PdxFieldTypes::BOOLEAN_ARRAY:
      case PdxFieldTypes::CHAR_ARRAY:
      case PdxFieldTypes::BYTE_ARRAY:
      case PdxFieldTypes::SHORT_ARRAY:
      case PdxFieldTypes::INT_ARRAY:
      case PdxFieldTypes::LONG_ARRAY:
      case PdxFieldTypes::FLOAT_ARRAY:
      case PdxFieldTypes::DOUBLE_ARRAY:
      case PdxFieldTypes::STRING_ARRAY:
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
        if (!compareRawBytes(input, otherInput, *other, myPFT, otherPFT)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::OBJECT: {
        std::shared_ptr<Cacheable> object = nullptr;
        std::shared_ptr<Cacheable> otherObject = nullptr;
        if (!myPFT->equals(m_DefaultPdxFieldType)) {
          setOffsetForObject(input, myPFT->getSequenceId());
          input.readObject(object);
        }

        if (!otherPFT->equals(m_DefaultPdxFieldType)) {
          other->setOffsetForObject(otherInput, otherPFT->getSequenceId());
          otherInput.readObject(otherObject);
        }

        if (object != nullptr) {
          if (!deepArrayEquals(object, otherObject)) {
            return false;
          }
        } else if (otherObject != nullptr) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::OBJECT_ARRAY: {
        auto otherObjectArray = CacheableObjectArray::create();
        auto objectArray = CacheableObjectArray::create();

        if (!myPFT->equals(m_DefaultPdxFieldType)) {
          setOffsetForObject(input, myPFT->getSequenceId());
          objectArray->fromData(input);
        }

        if (!otherPFT->equals(m_DefaultPdxFieldType)) {
          other->setOffsetForObject(otherInput, otherPFT->getSequenceId());
          otherObjectArray->fromData(otherInput);
        }
        if (!deepArrayEquals(objectArray, otherObjectArray)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::UNKNOWN: {
        throw IllegalStateException(
            std::string("PdxInstance not found typeid ") +
            std::to_string(static_cast<int>(myPFT->getTypeId())));
      }
    }
  }

  return true;
}

bool PdxInstanceImpl::compareRawBytes(
    DataInput& input, DataInput& otherInput, PdxInstanceImpl& other,
    std::shared_ptr<PdxFieldType> field,
    std::shared_ptr<PdxFieldType> otherField) const {
  auto otherType = other.m_pdxType;
  if (!field->equals(m_DefaultPdxFieldType) &&
      !otherField->equals(m_DefaultPdxFieldType)) {
    auto pos = getOffset(input, field->getSequenceId());
    auto nextpos = getNextFieldPosition(input, field->getSequenceId() + 1);
    input.reset();
    input.advanceCursor(pos);

    auto otherPos = other.getOffset(otherInput, otherField->getSequenceId());
    int otherNextpos =
        other.getNextFieldPosition(otherInput, otherField->getSequenceId() + 1);
    otherInput.reset();
    otherInput.advanceCursor(otherPos);

    if ((nextpos - pos) != (otherNextpos - otherPos)) {
      return false;
    }

    for (int i = pos; i < nextpos; i++) {
      if (input.read() != otherInput.read()) {
        return false;
      }
    }

    return true;
  } else {
    if (field->equals(m_DefaultPdxFieldType)) {
      int otherPos = other.getOffset(otherInput, otherField->getSequenceId());
      int otherNextpos = other.getNextFieldPosition(
          otherInput, otherField->getSequenceId() + 1);
      return hasDefaultBytes(otherField, otherInput, otherPos, otherNextpos);
    } else {
      int pos = getOffset(input, field->getSequenceId());
      int nextpos = getNextFieldPosition(input, field->getSequenceId() + 1);
      return hasDefaultBytes(field, input, pos, nextpos);
    }
  }
}

std::shared_ptr<CacheableStringArray> PdxInstanceImpl::getFieldNames() {
  std::vector<std::shared_ptr<PdxFieldType>>* vectorOfFieldTypes =
      m_pdxType->getPdxFieldTypes();
  auto size = vectorOfFieldTypes->size();
  if (size == 0) {
    return nullptr;
  }

  std::vector<std::shared_ptr<CacheableString>> tmpFieldNames;
  tmpFieldNames.reserve(size);
  for (auto&& fieldType : *vectorOfFieldTypes) {
    tmpFieldNames.emplace_back(
        CacheableString::create(fieldType->getFieldName()));
  }
  return CacheableStringArray::create(std::move(tmpFieldNames));
}

PdxFieldTypes PdxInstanceImpl::getFieldType(const std::string& name) const {
  auto pft = m_pdxType->getPdxField(name);

  if (!pft) {
    throw IllegalStateException("PdxInstance doesn't have field " + name);
  }

  return pft->getTypeId();
}

void PdxInstanceImpl::writeUnmodifieldField(DataInput& dataInput, int startPos,
                                            int endPos,
                                            PdxLocalWriter& localWriter) {
  dataInput.reset(startPos);
  for (; startPos < endPos; startPos++) {
    localWriter.writeByte(dataInput.read());
  }
}

void PdxInstanceImpl::toData(PdxWriter& writer) const {
  const_cast<PdxInstanceImpl*>(this)->toDataMutable(writer);
}

void PdxInstanceImpl::toDataMutable(PdxWriter& writer) {
  std::vector<std::shared_ptr<PdxFieldType>>* pdxFieldList =
      m_pdxType->getPdxFieldTypes();
  int position = 0;  // ignore typeid and length
  int nextFieldPosition = 0;
  if (m_buffer.size() != 0) {
    auto dataInput =
        m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());
    for (size_t i = 0; i < pdxFieldList->size(); i++) {
      auto currPf = pdxFieldList->at(i);
      LOGDEBUG("toData fieldName = %s , isVarLengthType = %d ",
               currPf->getFieldName().c_str(), currPf->IsVariableLengthType());
      std::shared_ptr<Cacheable> value = nullptr;

      auto&& iter = m_updatedFields.find(currPf->getFieldName());
      if (iter != m_updatedFields.end()) {
        value = iter->second;
      } else {
        value = nullptr;
      }
      if (value != nullptr) {
        writeField(writer, currPf->getFieldName(), currPf->getTypeId(), value);
        position = getNextFieldPosition(dataInput, static_cast<int>(i) + 1);
      } else {
        if (currPf->IsVariableLengthType()) {
          // need to add offset
          (static_cast<PdxLocalWriter&>(writer)).addOffset();
        }
        // write raw byte array...
        nextFieldPosition =
            getNextFieldPosition(dataInput, static_cast<int>(i) + 1);
        writeUnmodifieldField(dataInput, position, nextFieldPosition,
                              static_cast<PdxLocalWriter&>(writer));
        position = nextFieldPosition;  // mark next field;
      }
    }
  } else {
    for (size_t i = 0; i < pdxFieldList->size(); i++) {
      auto currPf = pdxFieldList->at(i);
      LOGDEBUG("toData1 fieldName = %s , isVarLengthType = %d ",
               currPf->getFieldName().c_str(), currPf->IsVariableLengthType());
      auto value = m_updatedFields[currPf->getFieldName()];
      writeField(writer, currPf->getFieldName(), currPf->getTypeId(), value);
    }
  }
  m_updatedFields.clear();
}

void PdxInstanceImpl::fromData(PdxReader&) {
  throw IllegalStateException(
      "PdxInstance::FromData( .. ) shouldn't have called");
}

const std::string& PdxInstanceImpl::getClassName() const {
  return m_pdxType->getPdxClassName();
}

std::vector<std::shared_ptr<PdxFieldType>>
PdxInstanceImpl::getIdentityPdxFields() const {
  std::vector<std::shared_ptr<PdxFieldType>> result;
  const auto& fields = *m_pdxType->getPdxFieldTypes();

  for (const auto& field : fields) {
    if (field->getIdentityField()) {
      result.push_back(field);
    }
  }

  if (result.empty()) {
    result = fields;
  }

  std::sort(result.begin(), result.end(),
            [](std::shared_ptr<PdxFieldType> first,
               std::shared_ptr<PdxFieldType> second) {
              return first->getFieldName() < second->getFieldName();
            });

  return result;
}

int PdxInstanceImpl::getOffset(DataInput& input, int sequenceId) const {
  input.resetPdx(0);

  int offsetSize = 0;
  int serializedLength = 0;
  int pdxSerializedLength = static_cast<int32_t>(input.getPdxBytes());

  LOGDEBUG("getOffset pdxSerializedLength = %d ", pdxSerializedLength);

  if (pdxSerializedLength <= 0xff) {
    offsetSize = 1;
  } else if (pdxSerializedLength <= 0xffff) {
    offsetSize = 2;
  } else {
    offsetSize = 4;
  }

  if (m_pdxType->getNumberOfVarLenFields() > 0) {
    serializedLength =
        pdxSerializedLength -
        ((m_pdxType->getNumberOfVarLenFields() - 1) * offsetSize);
  } else {
    serializedLength = pdxSerializedLength;
  }

  //[ToDo see if currentBufferPosition can correctly replace GetCursor]
  uint8_t* offsetsBuffer =
      const_cast<uint8_t*>(input.currentBufferPosition()) + serializedLength;
  return m_pdxType->getFieldPosition(sequenceId, offsetsBuffer, offsetSize,
                                     serializedLength);
}

int PdxInstanceImpl::getRawHashCode(DataInput& input,
                                    std::shared_ptr<PdxFieldType> field) const {
  auto pos = getOffset(input, field->getSequenceId());
  auto nextpos = getNextFieldPosition(input, field->getSequenceId() + 1);

  LOGDEBUG("pos = %d nextpos = %d ", pos, nextpos);

  if (hasDefaultBytes(field, input, pos, nextpos)) {
    return 0;  // matched default bytes
  }

  input.reset();
  input.advanceCursor(nextpos - 1);

  int h = 1;
  for (int i = nextpos - 1; i >= pos; i--) {
    h = 31 * h + static_cast<int>(input.read());
    input.reset();
    input.advanceCursor(i - 1);
  }
  LOGDEBUG("getRawHashCode nbytes = %d, final hashcode = %d ", (nextpos - pos),
           h);
  return h;
}

int PdxInstanceImpl::getNextFieldPosition(DataInput& input, int fieldId) const {
  LOGDEBUG("fieldId = %d pt->getTotalFields() = %d ", fieldId,
           m_pdxType->getTotalFields());

  return fieldId == m_pdxType->getTotalFields() ? getSerializedLength(input)
                                                : getOffset(input, fieldId);
}

int PdxInstanceImpl::getSerializedLength(DataInput& input) const {
  input.resetPdx(0);

  int offsetSize = 0;
  int serializedLength = 0;
  int pdxSerializedLength = static_cast<int32_t>(input.getPdxBytes());

  LOGDEBUG("pdxSerializedLength = %d ", pdxSerializedLength);

  if (pdxSerializedLength <= 0xff) {
    offsetSize = 1;
  } else if (pdxSerializedLength <= 0xffff) {
    offsetSize = 2;
  } else {
    offsetSize = 4;
  }

  if (m_pdxType->getNumberOfVarLenFields() > 0) {
    serializedLength =
        pdxSerializedLength -
        ((m_pdxType->getNumberOfVarLenFields() - 1) * offsetSize);
  } else {
    serializedLength = pdxSerializedLength;
  }

  return serializedLength;
}

bool PdxInstanceImpl::compareDefaultBytes(DataInput& dataInput, int start,
                                          int end, int8_t* defaultBytes,
                                          int32_t length) const {
  if ((end - start) != length) return false;

  dataInput.reset();
  dataInput.advanceCursor(start);
  int j = 0;
  for (int i = start; i < end; i++) {
    if (defaultBytes[j++] != dataInput.read()) {
      return false;
    }
  }
  return true;
}

bool PdxInstanceImpl::hasDefaultBytes(std::shared_ptr<PdxFieldType> pField,
                                      DataInput& dataInput, int start,
                                      int end) const {
  switch (pField->getTypeId()) {
    case PdxFieldTypes::INT: {
      return compareDefaultBytes(dataInput, start, end, m_IntDefaultBytes, 4);
    }
    case PdxFieldTypes::STRING: {
      return compareDefaultBytes(dataInput, start, end, m_StringDefaultBytes,
                                 1);
    }
    case PdxFieldTypes::BOOLEAN: {
      return compareDefaultBytes(dataInput, start, end, m_BooleanDefaultBytes,
                                 1);
    }
    case PdxFieldTypes::FLOAT: {
      return compareDefaultBytes(dataInput, start, end, m_FloatDefaultBytes, 4);
    }
    case PdxFieldTypes::DOUBLE: {
      return compareDefaultBytes(dataInput, start, end, m_DoubleDefaultBytes,
                                 8);
    }
    case PdxFieldTypes::CHAR: {
      return compareDefaultBytes(dataInput, start, end, m_CharDefaultBytes, 2);
    }
    case PdxFieldTypes::BYTE: {
      return compareDefaultBytes(dataInput, start, end, m_ByteDefaultBytes, 1);
    }
    case PdxFieldTypes::SHORT: {
      return compareDefaultBytes(dataInput, start, end, m_ShortDefaultBytes, 2);
    }
    case PdxFieldTypes::LONG: {
      return compareDefaultBytes(dataInput, start, end, m_LongDefaultBytes, 8);
    }
    case PdxFieldTypes::BYTE_ARRAY:
    case PdxFieldTypes::DOUBLE_ARRAY:
    case PdxFieldTypes::FLOAT_ARRAY:
    case PdxFieldTypes::SHORT_ARRAY:
    case PdxFieldTypes::INT_ARRAY:
    case PdxFieldTypes::LONG_ARRAY:
    case PdxFieldTypes::BOOLEAN_ARRAY:
    case PdxFieldTypes::CHAR_ARRAY:
    case PdxFieldTypes::STRING_ARRAY:
    case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
    case PdxFieldTypes::OBJECT_ARRAY: {
      return compareDefaultBytes(dataInput, start, end, m_NULLARRAYDefaultBytes,
                                 1);
    }
    case PdxFieldTypes::DATE: {
      return compareDefaultBytes(dataInput, start, end, m_DateDefaultBytes, 8);
    }
    case PdxFieldTypes::OBJECT: {
      return compareDefaultBytes(dataInput, start, end, m_ObjectDefaultBytes,
                                 1);
    }
    case PdxFieldTypes::UNKNOWN: {
      throw IllegalStateException("hasDefaultBytes unable to find typeID ");
    }
  }
  throw IllegalStateException("hasDefaultBytes unable to find typeID ");
}

void PdxInstanceImpl::setField(const std::string& name, bool value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BOOLEAN) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableBoolean::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, signed char value) {
  auto pft = m_pdxType->getPdxField(name);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableByte::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, unsigned char value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableByte::create(value);
  m_updatedFields[name] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& name, int16_t value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::SHORT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableInt16::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, int32_t value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::INT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableInt32::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, int64_t value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::LONG) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableInt64::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, float value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::FLOAT) {
    throw IllegalStateException(
        "PdxInstance doesn't have field " + name +
        " or type of field not matched " +
        (pft != nullptr ? pft->toString().c_str() : ""));
  }

  m_updatedFields[name] = CacheableFloat::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, double value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::DOUBLE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableDouble::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, char16_t value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::CHAR) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableCharacter::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               std::shared_ptr<CacheableDate> value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::DATE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = value;
}

void PdxInstanceImpl::setField(const std::string& name,
                               std::shared_ptr<Cacheable> value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::OBJECT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  m_updatedFields[name] = value;
}

void PdxInstanceImpl::setField(const std::string& name,
                               std::shared_ptr<CacheableObjectArray> value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::OBJECT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  m_updatedFields[name] = value;
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<bool>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BOOLEAN_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = BooleanArray::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<int8_t>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BYTE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableBytes::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<int16_t>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::SHORT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableInt16Array::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<int32_t>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::INT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableInt32Array::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<int64_t>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::LONG_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableInt64Array::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<float>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::FLOAT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableFloatArray::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<double>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::DOUBLE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableDoubleArray::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::vector<char16_t>& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::CHAR_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CharArray::create(value);
}

void PdxInstanceImpl::setField(const std::string& name,
                               const std::string& value) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::STRING) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  m_updatedFields[name] = CacheableString::create(value);
}

void PdxInstanceImpl::setField(const std::string& name, int8_t** value,
                               int32_t arrayLength, int32_t* elementLength) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr &&
      pft->getTypeId() != PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableVector::create();
  for (int i = 0; i < arrayLength; i++) {
    auto ptr = CacheableBytes::create(
        std::vector<int8_t>(value[i], value[i] + elementLength[i]));
    cacheableObject->push_back(ptr);
  }

  m_updatedFields[name] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& name, std::string* value,
                               int32_t length) {
  auto pft = m_pdxType->getPdxField(name);
  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::STRING_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }

  if (length > 0) {
    std::vector<std::shared_ptr<CacheableString>> tmpValues;
    tmpValues.reserve(length);
    for (int32_t i = 0; i < length; ++i) {
      tmpValues.emplace_back(CacheableString::create(value[i]));
    }
    m_updatedFields[name] = CacheableStringArray::create(std::move(tmpValues));
  }
}

void PdxInstanceImpl::setOffsetForObject(DataInput& input,
                                         int sequenceId) const {
  int pos = getOffset(input, sequenceId);

  input.reset();
  input.advanceCursor(pos);
}

size_t PdxInstanceImpl::objectSize() const {
  auto size = sizeof(PdxInstanceImpl);
  size += m_buffer.size();
  size += m_pdxType->objectSize();
  for (FieldVsValues::const_iterator iter = m_updatedFields.begin();
       iter != m_updatedFields.end(); ++iter) {
    size += iter->first.length();
    size += iter->second->objectSize();
  }
  return size;
}

PdxTypeRegistry& PdxInstanceImpl::getPdxTypeRegistry() const {
  return m_pdxTypeRegistry;
}

DataInput PdxInstanceImpl::getDataInputForField(const std::string& name) const {
  auto pft = m_pdxType->getPdxField(name);

  if (!pft) {
    throw IllegalStateException("PdxInstance doesn't have field " + name);
  }

  auto dataInput =
      m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());
  auto pos = getOffset(dataInput, pft->getSequenceId());

  dataInput.reset();
  dataInput.advanceCursor(pos);

  return dataInput;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
