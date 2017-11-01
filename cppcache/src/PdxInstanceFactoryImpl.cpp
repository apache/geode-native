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
#include "PdxInstanceFactoryImpl.hpp"
#include "PdxType.hpp"
#include "PdxTypes.hpp"
#include "PdxInstanceImpl.hpp"
#include <ace/OS_NS_stdio.h>

namespace apache {
namespace geode {
namespace client {

PdxInstanceFactoryImpl::~PdxInstanceFactoryImpl() {}

PdxInstanceFactoryImpl::PdxInstanceFactoryImpl(
    const char* className, CachePerfStats* cachePerfStats,
    std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry, const Cache* cache,
    bool enableTimeStatistics)
    : m_pdxType(std::make_shared<PdxType>(pdxTypeRegistry, className, false)),
      m_created(false),
      m_cachePerfStats(cachePerfStats),
      m_pdxTypeRegistry(pdxTypeRegistry),
      m_cache(cache),
      m_enableTimeStatistics(enableTimeStatistics) {
  if (className == nullptr ||
      *className == '\0') {  // COVERITY ---> 30289 Same on both sides
    throw IllegalStateException("className should not be null.");
  }
}

std::unique_ptr<PdxInstance> PdxInstanceFactoryImpl::create() {
  if (m_created) {
    throw IllegalStateException(
        "The PdxInstanceFactory.Create() method can only be called once.");
  }
  auto pi = std::unique_ptr<PdxInstance>(
      new PdxInstanceImpl(m_FieldVsValues, m_pdxType, m_cachePerfStats,
                          m_pdxTypeRegistry, m_cache, m_enableTimeStatistics));
  m_created = true;
  return pi;
}
 std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeChar(const char* fieldName,
                                                        char16_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,
                                     PdxTypes::CHAR_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableCharacter::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeChar(const char* fieldName,
                                                        char value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,
                                     PdxTypes::CHAR_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableCharacter::create(value);
  m_FieldVsValues.emplace(fieldName, cacheableObject);
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeBoolean(
    const char* fieldName, bool value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "bool", PdxFieldTypes::BOOLEAN,
                                     PdxTypes::BOOLEAN_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableBoolean::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeByte(const char* fieldName,
                                                        int8_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE,
                                     PdxTypes::BYTE_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableByte::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeShort(const char* fieldName,
                                                         int16_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT,
                                     PdxTypes::SHORT_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableInt16::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeInt(const char* fieldName,
                                                       int32_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT,
                                     PdxTypes::INTEGER_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableInt32::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeLong(const char* fieldName,
                                                        int64_t value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG,
                                     PdxTypes::LONG_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableInt64::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeFloat(const char* fieldName,
                                                         float value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "float", PdxFieldTypes::FLOAT,
                                     PdxTypes::FLOAT_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableFloat::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeDouble(const char* fieldName,
                                                          double value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "double", PdxFieldTypes::DOUBLE,
                                     PdxTypes::DOUBLE_SIZE);
  std::shared_ptr<Cacheable> cacheableObject = CacheableDouble::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeWideString(
    const char* fieldName, const wchar_t* value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string",
                                        PdxFieldTypes::STRING);
  std::shared_ptr<Cacheable> cacheableObject = CacheableString::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeString(const char* fieldName,
                                                          const char* value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string",
                                        PdxFieldTypes::STRING);
  std::shared_ptr<Cacheable> cacheableObject = CacheableString::create(value);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeObject(const char* fieldName,
                                                          std::shared_ptr<Cacheable> value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "Object",
                                        PdxFieldTypes::OBJECT);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, value));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeObjectArray(
    const char* fieldName, std::shared_ptr<CacheableObjectArray> value) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "Object[]",
                                        PdxFieldTypes::OBJECT_ARRAY);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, value));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeBooleanArray(
    const char* fieldName, bool* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "bool[]",
                                        PdxFieldTypes::BOOLEAN_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = BooleanArray::create(value, length);
  m_FieldVsValues.insert(
      std::pair<std::string, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeWideCharArray(
    const char* fieldName, wchar_t* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "char[]",
                                        PdxFieldTypes::CHAR_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CharArray::create(value, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeCharArray(
    const char* fieldName, char* value, int32_t length) {
  isFieldAdded(fieldName);
  size_t size = strlen(value) + 1;
  wchar_t* tempWideCharArray = new wchar_t[size];
  mbstowcs(tempWideCharArray, value, size);
  m_pdxType->addVariableLengthTypeField(fieldName, "char[]",
                                        PdxFieldTypes::CHAR_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CharArray::create(tempWideCharArray, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  delete[] tempWideCharArray;
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeByteArray(
    const char* fieldName, int8_t* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "byte[]",
                                        PdxFieldTypes::BYTE_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject =
      CacheableBytes::create(reinterpret_cast<uint8_t*>(value), length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeShortArray(
    const char* fieldName, int16_t* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "short[]",
                                        PdxFieldTypes::SHORT_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CacheableInt16Array::create(value, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeIntArray(
    const char* fieldName, int32_t* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "int32[]",
                                        PdxFieldTypes::INT_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CacheableInt32Array::create(value, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeLongArray(
    const char* fieldName, int64_t* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "int64[]",
                                        PdxFieldTypes::LONG_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CacheableInt64Array::create(value, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeFloatArray(
    const char* fieldName, float* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "float[]",
                                        PdxFieldTypes::FLOAT_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CacheableFloatArray::create(value, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeDoubleArray(
    const char* fieldName, double* value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "double[]",
                                        PdxFieldTypes::DOUBLE_ARRAY);
  std::shared_ptr<Cacheable> cacheableObject = CacheableDoubleArray::create(value, length);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeStringArray(
    const char* fieldName, char** value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string[]",
                                        PdxFieldTypes::STRING_ARRAY);
  std::shared_ptr<CacheableString>* ptrArr = nullptr;
  if (length > 0) {
    ptrArr = new std::shared_ptr<CacheableString>[length];
    for (int32_t i = 0; i < length; i++) {
      ptrArr[i] = CacheableString::create(value[i]);
    }
  }
  if (length > 0) {
    std::shared_ptr<Cacheable> cacheableObject = CacheableStringArray::create(ptrArr, length);
    m_FieldVsValues.insert(
        std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  }
  /* adongre  - Coverity II
   * CID 29199: Resource leak (RESOURCE_LEAK)
   */
  if (ptrArr) {
    delete[] ptrArr;
  }
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeWideStringArray(
    const char* fieldName, wchar_t** value, int32_t length) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "string[]",
                                        PdxFieldTypes::STRING_ARRAY);
  std::shared_ptr<CacheableString>* ptrArr = nullptr;
  if (length > 0) {
    ptrArr = new std::shared_ptr<CacheableString>[length];
    for (int32_t i = 0; i < length; i++) {
      ptrArr[i] = CacheableString::create(value[i]);
    }
  }
  if (length > 0) {
    std::shared_ptr<Cacheable> cacheableObject = CacheableStringArray::create(ptrArr, length);
    m_FieldVsValues.insert(
        std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  }
  /* adongre - Coverity II
   * CID 29200: Resource leak (RESOURCE_LEAK)
   */
  if (ptrArr) {
    delete[] ptrArr;
  }
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeDate(
    const char* fieldName, std::shared_ptr<CacheableDate> value) {
  isFieldAdded(fieldName);
  m_pdxType->addFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE,
                                     PdxTypes::DATE_SIZE /*+ 1*/);
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, value));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::writeArrayOfByteArrays(
    const char* fieldName, int8_t** value, int32_t arrayLength,
    int32_t* elementLength) {
  isFieldAdded(fieldName);
  m_pdxType->addVariableLengthTypeField(fieldName, "byte[][]",
                                        PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  std::shared_ptr<CacheableVector> cacheableObject = CacheableVector::create();
  for (int i = 0; i < arrayLength; i++) {
    std::shared_ptr<CacheableBytes> ptr = CacheableBytes::create(
        reinterpret_cast<uint8_t*>(value[i]), elementLength[i]);
    cacheableObject->push_back(ptr);
  }
  m_FieldVsValues.insert(
      std::pair<const char*, std::shared_ptr<Cacheable>>(fieldName, cacheableObject));
  return shared_from_this();
}

std::shared_ptr<PdxInstanceFactory> PdxInstanceFactoryImpl::markIdentityField(
    const char* fieldName) {
  auto pfType = m_pdxType->getPdxField(fieldName);
  if (pfType == nullptr) {
    throw IllegalStateException(
        "Field must be added before calling MarkIdentityField ");
  }
  pfType->setIdentityField(true);
  return shared_from_this();
}

void PdxInstanceFactoryImpl::isFieldAdded(const char* fieldName) {
  if (fieldName == nullptr ||
      /**fieldName == '\0' ||*/ m_FieldVsValues.find(fieldName) !=
          m_FieldVsValues.end()) {
    char excpStr[256] = {0};
    /* adongre - Coverity II
     * CID 29209: Calling risky function (SECURE_CODING)[VERY RISKY]. Using
     * "sprintf" can cause a
     * buffer overflow when done incorrectly. Because sprintf() assumes an
     * arbitrarily long string,
     * callers must be careful not to overflow the actual space of the
     * destination.
     * Use snprintf() instead, or correct precision specifiers.
     * Fix : using ACE_OS::snprintf
     */
    ACE_OS::snprintf(excpStr, 256,
                     "Field: %s is either already added into "
                     "PdxInstanceFactory or it is null ",
                     fieldName);
    throw IllegalStateException(excpStr);
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
