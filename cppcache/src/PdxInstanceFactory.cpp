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

#include <geode/PdxInstanceFactory.hpp>

#include "PdxHelper.hpp"
#include "PdxInstanceImpl.hpp"
#include "PdxType.hpp"
#include "PdxTypes.hpp"

namespace apache {
namespace geode {
namespace client {

PdxInstanceFactory::PdxInstanceFactory(const std::string& className,
                                       bool expectDomainClass,
                                       CachePerfStats& cachePerfStats,
                                       PdxTypeRegistry& pdxTypeRegistry,
                                       const CacheImpl& cache,
                                       bool enableTimeStatistics)
    : m_created(false),
      m_pdxType(std::make_shared<PdxType>(pdxTypeRegistry, className, false,
                                          expectDomainClass)),
      m_cachePerfStats(cachePerfStats),
      m_pdxTypeRegistry(pdxTypeRegistry),
      m_cacheImpl(cache),
      m_enableTimeStatistics(enableTimeStatistics) {}

std::shared_ptr<PdxInstance> PdxInstanceFactory::create() {
  if (m_created) {
    throw IllegalStateException(
        "The PdxInstanceFactory.Create() method can only be called once.");
  }

  m_created = true;
  return std::make_shared<PdxInstanceImpl>(m_FieldVsValues, m_pdxType,
                                           m_cachePerfStats, m_pdxTypeRegistry,
                                           m_cacheImpl, m_enableTimeStatistics);
}

PdxInstanceFactory& PdxInstanceFactory::writeChar(const std::string& fieldName,
                                                  char16_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,
                                     PdxTypes::kPdxCharSize);
  auto cacheableObject = CacheableCharacter::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeChar(const std::string& fieldName,
                                                  char value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,
                                     PdxTypes::kPdxCharSize);
  auto cacheableObject = CacheableCharacter::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeBoolean(
    const std::string& fieldName, bool value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "bool", PdxFieldTypes::BOOLEAN,
                                     PdxTypes::kPdxBooleanSize);
  auto cacheableObject = CacheableBoolean::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeByte(const std::string& fieldName,
                                                  int8_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE,
                                     PdxTypes::kPdxByteSize);
  auto cacheableObject = CacheableByte::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeShort(const std::string& fieldName,
                                                   int16_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT,
                                     PdxTypes::kPdxShortSize);
  auto cacheableObject = CacheableInt16::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeInt(const std::string& fieldName,
                                                 int32_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT,
                                     PdxTypes::kPdxIntegerSize);
  auto cacheableObject = CacheableInt32::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeLong(const std::string& fieldName,
                                                  int64_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG,
                                     PdxTypes::kPdxLongSize);
  auto cacheableObject = CacheableInt64::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeFloat(const std::string& fieldName,
                                                   float value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "float", PdxFieldTypes::FLOAT,
                                     PdxTypes::kPdxFloatSize);
  auto cacheableObject = CacheableFloat::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeDouble(
    const std::string& fieldName, double value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "double", PdxFieldTypes::DOUBLE,
                                     PdxTypes::kPdxDoubleSize);
  auto cacheableObject = CacheableDouble::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeString(
    const std::string& fieldName, const std::string& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string",
                                        PdxFieldTypes::STRING);
  auto cacheableObject = CacheableString::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeString(
    const std::string& fieldName, std::string&& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string",
                                        PdxFieldTypes::STRING);
  auto cacheableObject = CacheableString::create(std::move(value));
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeObject(
    const std::string& fieldName, std::shared_ptr<Cacheable> value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "Object",
                                        PdxFieldTypes::OBJECT);
  m_FieldVsValues.emplace(fieldName, value);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeObjectArray(
    const std::string& fieldName, std::shared_ptr<CacheableObjectArray> value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "Object[]",
                                        PdxFieldTypes::OBJECT_ARRAY);
  m_FieldVsValues.emplace(fieldName, value);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeBooleanArray(
    const std::string& fieldName, const std::vector<bool>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "bool[]",
                                        PdxFieldTypes::BOOLEAN_ARRAY);
  auto cacheableObject = BooleanArray::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeCharArray(
    const std::string& fieldName, const std::vector<char16_t>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "char[]",
                                        PdxFieldTypes::CHAR_ARRAY);
  auto cacheableObject = CharArray::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeByteArray(
    const std::string& fieldName, const std::vector<int8_t>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "byte[]",
                                        PdxFieldTypes::BYTE_ARRAY);
  auto cacheableObject = CacheableBytes::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeShortArray(
    const std::string& fieldName, const std::vector<int16_t>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "short[]",
                                        PdxFieldTypes::SHORT_ARRAY);
  auto cacheableObject = CacheableInt16Array::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeIntArray(
    const std::string& fieldName, const std::vector<int32_t>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "int32[]",
                                        PdxFieldTypes::INT_ARRAY);
  auto cacheableObject = CacheableInt32Array::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeLongArray(
    const std::string& fieldName, const std::vector<int64_t>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "int64[]",
                                        PdxFieldTypes::LONG_ARRAY);
  auto cacheableObject = CacheableInt64Array::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeFloatArray(
    const std::string& fieldName, const std::vector<float>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "float[]",
                                        PdxFieldTypes::FLOAT_ARRAY);
  auto cacheableObject = CacheableFloatArray::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeDoubleArray(
    const std::string& fieldName, const std::vector<double>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "double[]",
                                        PdxFieldTypes::DOUBLE_ARRAY);
  auto cacheableObject = CacheableDoubleArray::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeStringArray(
    const std::string& fieldName, const std::vector<std::string>& value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string[]",
                                        PdxFieldTypes::STRING_ARRAY);
  if (!value.empty()) {
    std::vector<std::shared_ptr<CacheableString>> ptrArr;
    ptrArr.reserve(value.size());
    for (auto&& v : value) {
      ptrArr.push_back(CacheableString::create(v));
    }
    auto cacheableObject = CacheableStringArray::create(ptrArr);
    m_FieldVsValues.emplace(fieldName, cacheableObject);
  }
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeDate(
    const std::string& fieldName, std::shared_ptr<CacheableDate> value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE,
                                     PdxTypes::kPdxDateSize /*+ 1*/);
  m_FieldVsValues.emplace(fieldName, value);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeArrayOfByteArrays(
    const std::string& fieldName, int8_t** value, int32_t arrayLength,
    int32_t* elementLength) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "byte[][]",
                                        PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  auto cacheableObject = CacheableVector::create();
  for (int i = 0; i < arrayLength; i++) {
    auto ptr = CacheableBytes::create(
        std::vector<int8_t>(value[i], value[i] + elementLength[i]));
    cacheableObject->push_back(ptr);
  }
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::markIdentityField(
    const std::string& fieldName) {
  auto pfType = m_pdxType->getPdxField(fieldName);
  if (pfType == nullptr) {
    throw IllegalStateException(
        "Field must be added before calling MarkIdentityField ");
  }
  pfType->setIdentityField(true);
  return *this;
}

void PdxInstanceFactory::isFieldAdded(const std::string& fieldName) {
  if (m_FieldVsValues.find(fieldName) != m_FieldVsValues.end()) {
    throw IllegalStateException("Field: " + fieldName +
                                " is either already added into "
                                "PdxInstanceFactory or it is null");
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
