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

bool sortFunc(std::shared_ptr<PdxFieldType> field1,
              std::shared_ptr<PdxFieldType> field2) {
  const auto diff = field1->getFieldName().compare(field2->getFieldName());
  if (diff < 0) {
    return true;
  } else {
    return false;
  }
}

PdxInstanceImpl::~PdxInstanceImpl() noexcept {}

PdxInstanceImpl::PdxInstanceImpl(const uint8_t* buffer, size_t length,
                                 int typeId, CachePerfStats& cacheStats,
                                 PdxTypeRegistry& pdxTypeRegistry,
                                 const CacheImpl& cacheImpl,
                                 bool enableTimeStatistics)
    : m_buffer(buffer, buffer + length),
      m_typeId(typeId),
      m_pdxType(nullptr),
      m_cacheStats(cacheStats),
      m_pdxTypeRegistry(pdxTypeRegistry),
      m_cacheImpl(cacheImpl),
      m_enableTimeStatistics(enableTimeStatistics) {
  LOG_DEBUG("PdxInstanceImpl::m_bufferLength = %zu ", m_buffer.size());
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
  m_pdxType->InitializeType();  // to generate static position map
}

void PdxInstanceImpl::writeField(PdxWriter& writer,
                                 const std::string& fieldName,
                                 PdxFieldTypes typeId,
                                 std::shared_ptr<Cacheable> value) {
  switch (typeId) {
    case PdxFieldTypes::INT: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt32>(value)) {
        writer.writeInt(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::STRING: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableString>(value)) {
        writer.writeString(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::BOOLEAN: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableBoolean>(value)) {
        writer.writeBoolean(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::FLOAT: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableFloat>(value)) {
        writer.writeFloat(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::DOUBLE: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableDouble>(value)) {
        writer.writeDouble(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::CHAR: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableCharacter>(value)) {
        writer.writeChar(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::BYTE: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableByte>(value)) {
        writer.writeByte(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::SHORT: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt16>(value)) {
        writer.writeShort(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::LONG: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt64>(value)) {
        writer.writeLong(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::BYTE_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableBytes>(value)) {
        writer.writeByteArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::DOUBLE_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableDoubleArray>(value)) {
        writer.writeDoubleArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::FLOAT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableFloatArray>(value)) {
        writer.writeFloatArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::SHORT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt16Array>(value)) {
        writer.writeShortArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::INT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt32Array>(value)) {
        writer.writeIntArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::LONG_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableInt64Array>(value)) {
        writer.writeLongArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::BOOLEAN_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<BooleanArray>(value)) {
        writer.writeBooleanArray(fieldName, val->value());
      }
      break;
    }
    case PdxFieldTypes::CHAR_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CharArray>(value)) {
        writer.writeCharArray(fieldName, val->value());
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
        writer.writeStringArray(fieldName, strings);
      }
      break;
    }
    case PdxFieldTypes::DATE: {
      if (auto&& date = std::dynamic_pointer_cast<CacheableDate>(value)) {
        writer.writeDate(fieldName, date);
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
        writer.writeArrayOfByteArrays(fieldName, values, static_cast<int>(size),
                                      lengths);
        delete[] values;
        delete[] lengths;
      }
      break;
    }
    case PdxFieldTypes::OBJECT_ARRAY: {
      if (auto&& val = std::dynamic_pointer_cast<CacheableObjectArray>(value)) {
        writer.writeObjectArray(fieldName, val);
      }
      break;
    }
    case PdxFieldTypes::UNKNOWN:
    case PdxFieldTypes::OBJECT: {
      writer.writeObject(fieldName, value);
    }
  }
}
std::shared_ptr<WritablePdxInstance> PdxInstanceImpl::createWriter() {
  LOG_DEBUG("PdxInstanceImpl::createWriter m_bufferLength = %zu m_typeId = %d ",
            m_buffer.size(), m_typeId);
  return std::make_shared<PdxInstanceImpl>(
      m_buffer.data(), m_buffer.size(), m_typeId, m_cacheStats,
      m_pdxTypeRegistry, m_cacheImpl,
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
  int hashCode = 1;

  auto pt = getPdxType();

  auto pdxIdentityFieldList = getIdentityPdxFields(pt);

  auto dataInput =
      m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());

  for (uint32_t i = 0; i < pdxIdentityFieldList.size(); i++) {
    auto pField = pdxIdentityFieldList.at(i);

    LOG_DEBUG("hashcode for pdxfield %s  hashcode is %d ",
              pField->getFieldName().c_str(), hashCode);
    switch (pField->getTypeId()) {
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
        int retH = getRawHashCode(pt, pField, dataInput);
        if (retH != 0) hashCode = 31 * hashCode + retH;
        break;
      }
      case PdxFieldTypes::OBJECT: {
        setOffsetForObject(dataInput, pt, pField->getSequenceId());
        std::shared_ptr<Cacheable> object = nullptr;
        dataInput.readObject(object);
        if (object != nullptr) {
          hashCode = 31 * hashCode + deepArrayHashCode(object);
        }
        break;
      }
      case PdxFieldTypes::OBJECT_ARRAY: {
        setOffsetForObject(dataInput, pt, pField->getSequenceId());
        auto objectArray = CacheableObjectArray::create();
        objectArray->fromData(dataInput);
        hashCode =
            31 * hashCode +
            ((objectArray != nullptr) ? deepArrayHashCode(objectArray) : 0);
        break;
      }
      case PdxFieldTypes::UNKNOWN: {
        char excpStr[256] = {0};
        std::snprintf(excpStr, 256, "PdxInstance not found typeid %d ",
                      static_cast<int>(pField->getTypeId()));
        throw IllegalStateException(excpStr);
      }
    }
  }
  return hashCode;
}

void PdxInstanceImpl::updatePdxStream(uint8_t* newPdxStream, int len) {
  m_buffer.resize(len);
  memcpy(m_buffer.data(), newPdxStream, len);
}

std::shared_ptr<PdxType> PdxInstanceImpl::getPdxType() const {
  if (m_typeId == 0) {
    if (m_pdxType == nullptr) {
      throw IllegalStateException("PdxType should not be null..");
    }
    return m_pdxType;
  }
  auto pType = getPdxTypeRegistry().getPdxType(m_typeId);
  return pType;
}

bool PdxInstanceImpl::isIdentityField(const std::string& fieldname) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldname.c_str());
  if (pft != nullptr) {
    return pft->getIdentityField();
  }
  return false;
}

bool PdxInstanceImpl::hasField(const std::string& fieldname) {
  auto pf = getPdxType();
  auto pft = pf->getPdxField(fieldname.c_str());
  return (pft != nullptr);
}

bool PdxInstanceImpl::getBooleanField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readBoolean();
}

int8_t PdxInstanceImpl::getByteField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.read();
}

int16_t PdxInstanceImpl::getShortField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readInt16();
}

int32_t PdxInstanceImpl::getIntField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readInt32();
}

int64_t PdxInstanceImpl::getLongField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readInt64();
}

float PdxInstanceImpl::getFloatField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readFloat();
}

double PdxInstanceImpl::getDoubleField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readDouble();
}

char16_t PdxInstanceImpl::getCharField(const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readInt16();
}

std::string PdxInstanceImpl::getStringField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readString();
}

std::vector<bool> PdxInstanceImpl::getBooleanArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readBooleanArray();
}

std::vector<int8_t> PdxInstanceImpl::getByteArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readByteArray();
}

std::vector<int16_t> PdxInstanceImpl::getShortArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readShortArray();
}

std::vector<int32_t> PdxInstanceImpl::getIntArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readIntArray();
}

std::vector<int64_t> PdxInstanceImpl::getLongArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readLongArray();
}

std::vector<float> PdxInstanceImpl::getFloatArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readFloatArray();
}

std::vector<double> PdxInstanceImpl::getDoubleArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readDoubleArray();
}

std::vector<char16_t> PdxInstanceImpl::getCharArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readCharArray();
}

std::vector<std::string> PdxInstanceImpl::getStringArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  return dataInput.readStringArray();
}

std::shared_ptr<CacheableDate> PdxInstanceImpl::getCacheableDateField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  auto value = CacheableDate::create();
  value->fromData(dataInput);
  return value;
}

std::shared_ptr<Cacheable> PdxInstanceImpl::getCacheableField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  std::shared_ptr<Cacheable> value;
  dataInput.readObject(value);
  return value;
}
std::shared_ptr<CacheableObjectArray>
PdxInstanceImpl::getCacheableObjectArrayField(
    const std::string& fieldname) const {
  auto dataInput = getDataInputForField(fieldname);
  auto value = CacheableObjectArray::create();
  value->fromData(dataInput);
  return value;
}

void PdxInstanceImpl::getField(const std::string& fieldname, int8_t*** value,
                               int32_t& arrayLength,
                               int32_t*& elementLength) const {
  auto dataInput = getDataInputForField(fieldname);
  dataInput.readArrayOfByteArrays(value, arrayLength, &elementLength);
}

std::string PdxInstanceImpl::toString() const {
  auto pt = getPdxType();
  std::string toString = "PDX[" + std::to_string(pt->getTypeId()) + "," +
                         pt->getPdxClassName() + "]{";
  bool firstElement = true;
  auto identityFields = getIdentityPdxFields(pt);
  for (size_t i = 0; i < identityFields.size(); i++) {
    if (firstElement) {
      firstElement = false;
    } else {
      toString += ",";
    }
    toString += identityFields.at(i)->getFieldName();
    toString += "=";

    switch (identityFields.at(i)->getTypeId()) {
      case PdxFieldTypes::BOOLEAN: {
        auto&& value = getBooleanField(identityFields.at(i)->getFieldName());
        toString += value ? "true" : "false";
        break;
      }
      case PdxFieldTypes::BYTE: {
        auto&& value = getByteField(identityFields.at(i)->getFieldName());
        toString += std::to_string(value);
        break;
      }
      case PdxFieldTypes::SHORT: {
        int16_t value = getShortField(identityFields.at(i)->getFieldName());
        toString += std::to_string(value);
        break;
      }
      case PdxFieldTypes::INT: {
        int32_t value = getIntField(identityFields.at(i)->getFieldName());
        toString += std::to_string(value);
        break;
      }
      case PdxFieldTypes::LONG: {
        int64_t value = getLongField(identityFields.at(i)->getFieldName());
        toString += std::to_string(value);
        break;
      }
      case PdxFieldTypes::FLOAT: {
        float value = getFloatField(identityFields.at(i)->getFieldName());
        toString += std::to_string(value);
        break;
      }
      case PdxFieldTypes::DOUBLE: {
        double value = getDoubleField(identityFields.at(i)->getFieldName());
        toString += std::to_string(value);
        break;
      }
      case PdxFieldTypes::CHAR: {
        auto value = getCharField(identityFields.at(i)->getFieldName());
        toString += to_utf8(std::u16string{value});
        break;
      }
      case PdxFieldTypes::STRING: {
        auto value = getStringField(identityFields.at(i)->getFieldName());
        toString += value;
        break;
      }
      case PdxFieldTypes::CHAR_ARRAY: {
        auto value = getCharArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          toString += to_utf8(std::u16string(value.data(), length));
        }
        break;
      }
      case PdxFieldTypes::STRING_ARRAY: {
        auto value = getStringArrayField(identityFields.at(i)->getFieldName());
        for (auto&& v : value) {
          toString += v;
        }
        break;
      }
      case PdxFieldTypes::BYTE_ARRAY: {
        auto value = getByteArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::SHORT_ARRAY: {
        auto value = getShortArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::INT_ARRAY: {
        auto value = getIntArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::LONG_ARRAY: {
        auto value = getLongArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::FLOAT_ARRAY: {
        auto value = getFloatArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::DOUBLE_ARRAY: {
        auto value = getDoubleArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::DATE: {
        auto value =
            getCacheableDateField(identityFields.at(i)->getFieldName());
        if (value != nullptr) {
          toString += value->toString().c_str();
        }
        break;
      }
      case PdxFieldTypes::BOOLEAN_ARRAY: {
        auto value = getBooleanArrayField(identityFields.at(i)->getFieldName());
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            toString += v ? "true" : "false";
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
              toString += std::to_string(value[j][k]);
            }
          }
        }
        break;
      }
      case PdxFieldTypes::OBJECT_ARRAY: {
        auto value =
            getCacheableObjectArrayField(identityFields.at(i)->getFieldName());
        if (value != nullptr) {
          toString += value->toString().c_str();
        }
        break;
      }
      case PdxFieldTypes::OBJECT:
      case PdxFieldTypes::UNKNOWN: {
        auto value = getCacheableField(identityFields.at(i)->getFieldName());
        if (value != nullptr) {
          toString += value->toString().c_str();
        }
      }
    }
  }
  toString += "}";

  return toString;
}

std::shared_ptr<PdxSerializable> PdxInstanceImpl::getObject() {
  auto dataInput =
      m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());
  int64_t sampleStartNanos =
      m_enableTimeStatistics ? Utils::startStatOpTime() : 0;
  //[ToDo] do we have to call incPdxDeSerialization here?
  auto ret = PdxHelper::deserializePdx(dataInput, m_typeId,
                                       static_cast<int32_t>(m_buffer.size()));

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

bool PdxInstanceImpl::operator==(const CacheableKey& other) const {
  CacheableKey& temp = const_cast<CacheableKey&>(other);
  PdxInstanceImpl* otherPdx = dynamic_cast<PdxInstanceImpl*>(&temp);

  if (otherPdx == nullptr) {
    return false;
  }

  auto myPdxType = getPdxType();
  auto otherPdxType = otherPdx->getPdxType();

  auto&& myPdxClassName = myPdxType->getPdxClassName();
  auto&& otherPdxClassName = otherPdxType->getPdxClassName();

  if (otherPdxClassName != myPdxClassName) {
    return false;
  }

  auto myPdxIdentityFieldList = getIdentityPdxFields(myPdxType);
  auto otherPdxIdentityFieldList = otherPdx->getIdentityPdxFields(otherPdxType);

  equatePdxFields(myPdxIdentityFieldList, otherPdxIdentityFieldList);
  equatePdxFields(otherPdxIdentityFieldList, myPdxIdentityFieldList);

  auto myDataInput =
      m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());
  auto otherDataInput = m_cacheImpl.createDataInput(otherPdx->m_buffer.data(),
                                                    otherPdx->m_buffer.size());

  PdxFieldTypes fieldTypeId;
  for (size_t i = 0; i < myPdxIdentityFieldList.size(); i++) {
    auto myPFT = myPdxIdentityFieldList.at(i);
    auto otherPFT = otherPdxIdentityFieldList.at(i);

    LOG_DEBUG("pdxfield %s ",
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
        if (!compareRawBytes(*otherPdx, myPdxType, myPFT, myDataInput,
                             otherPdxType, otherPFT, otherDataInput)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::OBJECT: {
        std::shared_ptr<Cacheable> object = nullptr;
        std::shared_ptr<Cacheable> otherObject = nullptr;
        if (!myPFT->equals(m_DefaultPdxFieldType)) {
          setOffsetForObject(myDataInput, myPdxType, myPFT->getSequenceId());
          myDataInput.readObject(object);
        }

        if (!otherPFT->equals(m_DefaultPdxFieldType)) {
          otherPdx->setOffsetForObject(otherDataInput, otherPdxType,
                                       otherPFT->getSequenceId());
          otherDataInput.readObject(otherObject);
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
          setOffsetForObject(myDataInput, myPdxType, myPFT->getSequenceId());
          objectArray->fromData(myDataInput);
        }

        if (!otherPFT->equals(m_DefaultPdxFieldType)) {
          otherPdx->setOffsetForObject(otherDataInput, otherPdxType,
                                       otherPFT->getSequenceId());
          otherObjectArray->fromData(otherDataInput);
        }
        if (!deepArrayEquals(objectArray, otherObjectArray)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::UNKNOWN: {
        char excpStr[256] = {0};
        std::snprintf(excpStr, 256, "PdxInstance not found typeid  %d ",
                      static_cast<int>(myPFT->getTypeId()));
        throw IllegalStateException(excpStr);
      }
    }
  }
  return true;
}

bool PdxInstanceImpl::compareRawBytes(PdxInstanceImpl& other,
                                      std::shared_ptr<PdxType> myPT,
                                      std::shared_ptr<PdxFieldType> myF,
                                      DataInput& myDataInput,
                                      std::shared_ptr<PdxType> otherPT,
                                      std::shared_ptr<PdxFieldType> otherF,
                                      DataInput& otherDataInput) const {
  if (!myF->equals(m_DefaultPdxFieldType) &&
      !otherF->equals(m_DefaultPdxFieldType)) {
    int pos = getOffset(myDataInput, myPT, myF->getSequenceId());
    int nextpos =
        getNextFieldPosition(myDataInput, myF->getSequenceId() + 1, myPT);
    myDataInput.reset();
    myDataInput.advanceCursor(pos);

    int otherPos =
        other.getOffset(otherDataInput, otherPT, otherF->getSequenceId());
    int otherNextpos = other.getNextFieldPosition(
        otherDataInput, otherF->getSequenceId() + 1, otherPT);
    otherDataInput.reset();
    otherDataInput.advanceCursor(otherPos);

    if ((nextpos - pos) != (otherNextpos - otherPos)) {
      return false;
    }

    for (int i = pos; i < nextpos; i++) {
      if (myDataInput.read() != otherDataInput.read()) {
        return false;
      }
    }

    return true;
  } else {
    if (myF->equals(m_DefaultPdxFieldType)) {
      int otherPos =
          other.getOffset(otherDataInput, otherPT, otherF->getSequenceId());
      int otherNextpos = other.getNextFieldPosition(
          otherDataInput, otherF->getSequenceId() + 1, otherPT);
      return hasDefaultBytes(otherF, otherDataInput, otherPos, otherNextpos);
    } else {
      int pos = getOffset(myDataInput, myPT, myF->getSequenceId());
      int nextpos =
          getNextFieldPosition(myDataInput, myF->getSequenceId() + 1, myPT);
      return hasDefaultBytes(myF, myDataInput, pos, nextpos);
    }
  }
}
std::shared_ptr<CacheableStringArray> PdxInstanceImpl::getFieldNames() {
  auto pt = getPdxType();
  std::vector<std::shared_ptr<PdxFieldType>>* vectorOfFieldTypes =
      pt->getPdxFieldTypes();
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

PdxFieldTypes PdxInstanceImpl::getFieldType(
    const std::string& fieldname) const {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldname.c_str());

  if (!pft) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldname);
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
  auto pt = getPdxType();
  if (pt == nullptr) {
    m_typeId = 0;
    throw UnknownPdxTypeException("Unknown pdx type while serializing");
  }

  std::vector<std::shared_ptr<PdxFieldType>>* pdxFieldList =
      pt->getPdxFieldTypes();
  int position = 0;  // ignore typeid and length
  int nextFieldPosition = 0;
  if (m_buffer.size() != 0) {
    auto dataInput =
        m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());
    for (size_t i = 0; i < pdxFieldList->size(); i++) {
      auto currPf = pdxFieldList->at(i);
      LOG_DEBUG("toData fieldName = %s , isVarLengthType = %d ",
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
        position = getNextFieldPosition(dataInput, static_cast<int>(i) + 1, pt);
      } else {
        if (currPf->IsVariableLengthType()) {
          // need to add offset
          (static_cast<PdxLocalWriter&>(writer)).addOffset();
        }
        // write raw byte array...
        nextFieldPosition =
            getNextFieldPosition(dataInput, static_cast<int>(i) + 1, pt);
        writeUnmodifieldField(dataInput, position, nextFieldPosition,
                              static_cast<PdxLocalWriter&>(writer));
        position = nextFieldPosition;  // mark next field;
      }
    }
  } else {
    for (size_t i = 0; i < pdxFieldList->size(); i++) {
      auto currPf = pdxFieldList->at(i);
      LOG_DEBUG("toData1 fieldName = %s , isVarLengthType = %d ",
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
  if (m_typeId != 0) {
    auto pdxtype = getPdxTypeRegistry().getPdxType(m_typeId);
    if (pdxtype == nullptr) {
      throw IllegalStateException("PdxType is not defined for PdxInstance: " +
                                  std::to_string(m_typeId));
    }
    return pdxtype->getPdxClassName();
  }
  throw IllegalStateException(
      "PdxInstance typeid is not defined yet, to get classname.");
}

void PdxInstanceImpl::setPdxId(int32_t typeId) {
  m_pdxType->setTypeId(typeId);
  m_typeId = typeId;
}

std::vector<std::shared_ptr<PdxFieldType>>
PdxInstanceImpl::getIdentityPdxFields(std::shared_ptr<PdxType> pt) const {
  std::vector<std::shared_ptr<PdxFieldType>>* pdxFieldList =
      pt->getPdxFieldTypes();
  std::vector<std::shared_ptr<PdxFieldType>> retList;
  int size = static_cast<int>(pdxFieldList->size());
  for (int i = 0; i < size; i++) {
    auto pft = pdxFieldList->at(i);
    if (pft->getIdentityField()) retList.push_back(pft);
  }

  if (retList.size() > 0) {
    std::sort(retList.begin(), retList.end(), sortFunc);
    return retList;
  }

  for (int i = 0; i < size; i++) {
    auto pft = pdxFieldList->at(i);
    retList.push_back(pft);
  }

  std::sort(retList.begin(), retList.end(), sortFunc);

  return retList;
}

int PdxInstanceImpl::getOffset(DataInput& dataInput,
                               std::shared_ptr<PdxType> pt,
                               int sequenceId) const {
  dataInput.resetPdx(0);

  int offsetSize = 0;
  int serializedLength = 0;
  int pdxSerializedLength = static_cast<int32_t>(dataInput.getPdxBytes());
  LOG_DEBUG("getOffset pdxSerializedLength = %d ", pdxSerializedLength);
  if (pdxSerializedLength <= 0xff) {
    offsetSize = 1;
  } else if (pdxSerializedLength <= 0xffff) {
    offsetSize = 2;
  } else {
    offsetSize = 4;
  }

  if (pt->getNumberOfVarLenFields() > 0) {
    serializedLength = pdxSerializedLength -
                       ((pt->getNumberOfVarLenFields() - 1) * offsetSize);
  } else {
    serializedLength = pdxSerializedLength;
  }

  //[ToDo see if currentBufferPosition can correctly replace GetCursor]
  uint8_t* offsetsBuffer =
      const_cast<uint8_t*>(dataInput.currentBufferPosition()) +
      serializedLength;
  return pt->getFieldPosition(sequenceId, offsetsBuffer, offsetSize,
                              serializedLength);
}

int PdxInstanceImpl::getRawHashCode(std::shared_ptr<PdxType> pt,
                                    std::shared_ptr<PdxFieldType> pField,
                                    DataInput& dataInput) const {
  int pos = getOffset(dataInput, pt, pField->getSequenceId());
  int nextpos =
      getNextFieldPosition(dataInput, pField->getSequenceId() + 1, pt);

  LOG_DEBUG("pos = %d nextpos = %d ", pos, nextpos);

  if (hasDefaultBytes(pField, dataInput, pos, nextpos)) {
    return 0;  // matched default bytes
  }

  dataInput.reset();
  dataInput.advanceCursor(nextpos - 1);

  int h = 1;
  for (int i = nextpos - 1; i >= pos; i--) {
    h = 31 * h + static_cast<int>(dataInput.read());
    dataInput.reset();
    dataInput.advanceCursor(i - 1);
  }
  LOG_DEBUG("getRawHashCode nbytes = %d, final hashcode = %d ", (nextpos - pos),
            h);
  return h;
}

int PdxInstanceImpl::getNextFieldPosition(DataInput& dataInput, int fieldId,
                                          std::shared_ptr<PdxType> pt) const {
  LOG_DEBUG("fieldId = %d pt->getTotalFields() = %d ", fieldId,
            pt->getTotalFields());
  if (fieldId == pt->getTotalFields()) {
    // return serialized length
    return getSerializedLength(dataInput, pt);
  } else {
    return getOffset(dataInput, pt, fieldId);
  }
}

int PdxInstanceImpl::getSerializedLength(DataInput& dataInput,
                                         std::shared_ptr<PdxType> pt) const {
  dataInput.resetPdx(0);

  int offsetSize = 0;
  int serializedLength = 0;
  int pdxSerializedLength = static_cast<int32_t>(dataInput.getPdxBytes());
  LOG_DEBUG("pdxSerializedLength = %d ", pdxSerializedLength);
  if (pdxSerializedLength <= 0xff) {
    offsetSize = 1;
  } else if (pdxSerializedLength <= 0xffff) {
    offsetSize = 2;
  } else {
    offsetSize = 4;
  }

  if (pt->getNumberOfVarLenFields() > 0) {
    serializedLength = pdxSerializedLength -
                       ((pt->getNumberOfVarLenFields() - 1) * offsetSize);
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

void PdxInstanceImpl::setField(const std::string& fieldName, bool value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BOOLEAN) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableBoolean::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               signed char value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableByte::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               unsigned char value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableByte::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, int16_t value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::SHORT) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableInt16::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, int32_t value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::INT) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableInt32::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, int64_t value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::LONG) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableInt64::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, float value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::FLOAT) {
    throw IllegalStateException(
        "PdxInstance doesn't have field " + fieldName +
        " or type of field not matched " +
        (pft != nullptr ? pft->toString().c_str() : ""));
  }
  auto cacheableObject = CacheableFloat::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, double value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::DOUBLE) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableDouble::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, char16_t value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::CHAR) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableCharacter::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               std::shared_ptr<CacheableDate> value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::DATE) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = value;
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               std::shared_ptr<Cacheable> value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::OBJECT) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  m_updatedFields[fieldName] = value;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               std::shared_ptr<CacheableObjectArray> value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::OBJECT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  m_updatedFields[fieldName] = value;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<bool>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BOOLEAN_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = BooleanArray::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<int8_t>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::BYTE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableBytes::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<int16_t>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::SHORT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableInt16Array::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<int32_t>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::INT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableInt32Array::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<int64_t>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::LONG_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableInt64Array::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<float>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::FLOAT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableFloatArray::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<double>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::DOUBLE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableDoubleArray::create(value);
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::vector<char16_t>& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::CHAR_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto ptr = CharArray::create(value);
  m_updatedFields[fieldName] = ptr;
}

void PdxInstanceImpl::setField(const std::string& fieldName,
                               const std::string& value) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::STRING) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto ptr = CacheableString::create(value);
  m_updatedFields[fieldName] = ptr;
}

void PdxInstanceImpl::setField(const std::string& fieldName, int8_t** value,
                               int32_t arrayLength, int32_t* elementLength) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr &&
      pft->getTypeId() != PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  auto cacheableObject = CacheableVector::create();
  for (int i = 0; i < arrayLength; i++) {
    auto ptr = CacheableBytes::create(
        std::vector<int8_t>(value[i], value[i] + elementLength[i]));
    cacheableObject->push_back(ptr);
  }
  m_updatedFields[fieldName] = cacheableObject;
}

void PdxInstanceImpl::setField(const std::string& fieldName, std::string* value,
                               int32_t length) {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldName);

  if (pft != nullptr && pft->getTypeId() != PdxFieldTypes::STRING_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldName +
                                " or type of field not matched " +
                                (pft != nullptr ? pft->toString() : ""));
  }
  if (length > 0) {
    std::vector<std::shared_ptr<CacheableString>> tmpValues;
    tmpValues.reserve(length);
    for (int32_t i = 0; i < length; ++i) {
      tmpValues.emplace_back(CacheableString::create(value[i]));
    }
    m_updatedFields[fieldName] =
        CacheableStringArray::create(std::move(tmpValues));
  }
}

void PdxInstanceImpl::setOffsetForObject(DataInput& dataInput,
                                         std::shared_ptr<PdxType> pt,
                                         int sequenceId) const {
  int pos = getOffset(dataInput, pt, sequenceId);
  dataInput.reset();
  dataInput.advanceCursor(pos);
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

DataInput PdxInstanceImpl::getDataInputForField(
    const std::string& fieldname) const {
  auto pt = getPdxType();
  auto pft = pt->getPdxField(fieldname);

  if (!pft) {
    throw IllegalStateException("PdxInstance doesn't have field " + fieldname);
  }

  auto dataInput =
      m_cacheImpl.createDataInput(m_buffer.data(), m_buffer.size());
  auto pos = getOffset(dataInput, pt, pft->getSequenceId());

  dataInput.reset();
  dataInput.advanceCursor(pos);

  return dataInput;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
