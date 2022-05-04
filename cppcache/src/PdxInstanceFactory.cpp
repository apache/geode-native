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
#include "PdxWriterImpl.hpp"

namespace apache {
namespace geode {
namespace client {

PdxInstanceFactory::PdxInstanceFactory(const std::string& className,
                                       bool expectDomainClass,
                                       const CacheImpl& cache)
    : created_(false),
      pdxType_(std::make_shared<PdxType>(className, expectDomainClass)),
      cacheImpl_{cache} {}

std::shared_ptr<PdxInstance> PdxInstanceFactory::create() {
  if (created_) {
    throw IllegalStateException(
        "The PdxInstanceFactory.Create() method can only be called once.");
  }

  created_ = true;
  std::vector<std::shared_ptr<Cacheable>> fields;

  fields.resize(fields_.size());
  for (const auto& entry : fields_) {
    const auto& name = entry.first;
    auto type = entry.second.first;
    auto value = entry.second.second;
    auto fieldType = pdxType_->addField(name, type);
    fields[fieldType->getIndex()] = std::move(value);
  }

  for (const auto& name : identityFields_) {
    pdxType_->getField(name)->setIdentity(true);
  }

  pdxType_->initialize();
  return std::make_shared<PdxInstanceImpl>(std::move(fields), pdxType_,
                                           cacheImpl_);
}

PdxInstanceFactory& PdxInstanceFactory::writeChar(const std::string& fieldName,
                                                  char16_t value) {
  isFieldAdded(fieldName);
  fields_.emplace(
      fieldName,
      FieldInfo(PdxFieldTypes::CHAR, PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeChar(const std::string& fieldName,
                                                  char value) {
  isFieldAdded(fieldName);
  fields_.emplace(
      fieldName,
      FieldInfo(PdxFieldTypes::CHAR, PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeBoolean(
    const std::string& fieldName, bool value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::BOOLEAN,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeByte(const std::string& fieldName,
                                                  int8_t value) {
  isFieldAdded(fieldName);
  fields_.emplace(
      fieldName,
      FieldInfo(PdxFieldTypes::BYTE, PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeShort(const std::string& fieldName,
                                                   int16_t value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::SHORT,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeInt(const std::string& fieldName,
                                                 int32_t value) {
  isFieldAdded(fieldName);
  fields_.emplace(
      fieldName,
      FieldInfo(PdxFieldTypes::INT, PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeLong(const std::string& fieldName,
                                                  int64_t value) {
  isFieldAdded(fieldName);
  fields_.emplace(
      fieldName,
      FieldInfo(PdxFieldTypes::LONG, PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeFloat(const std::string& fieldName,
                                                   float value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::FLOAT,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeDouble(
    const std::string& fieldName, double value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::DOUBLE,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeDate(
    const std::string& fieldName, std::shared_ptr<CacheableDate> value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName, FieldInfo(PdxFieldTypes::DATE, value));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeString(
    const std::string& fieldName, const std::string& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::STRING,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeString(
    const std::string& fieldName, std::string&& value) {
  isFieldAdded(fieldName);
  fields_.emplace(
      fieldName,
      FieldInfo(PdxFieldTypes::STRING,
                PdxInstanceImpl::toCacheableField(std::move(value))));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeObject(
    const std::string& fieldName, std::shared_ptr<Cacheable> value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName, FieldInfo(PdxFieldTypes::OBJECT, value));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeObjectArray(
    const std::string& fieldName, std::shared_ptr<CacheableObjectArray> value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName, FieldInfo(PdxFieldTypes::OBJECT_ARRAY, value));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeBooleanArray(
    const std::string& fieldName, const std::vector<bool>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::BOOLEAN_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeCharArray(
    const std::string& fieldName, const std::vector<char16_t>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::CHAR_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeByteArray(
    const std::string& fieldName, const std::vector<int8_t>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::BYTE_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeShortArray(
    const std::string& fieldName, const std::vector<int16_t>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::SHORT_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeIntArray(
    const std::string& fieldName, const std::vector<int32_t>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::INT_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeLongArray(
    const std::string& fieldName, const std::vector<int64_t>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::LONG_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeFloatArray(
    const std::string& fieldName, const std::vector<float>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::FLOAT_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeDoubleArray(
    const std::string& fieldName, const std::vector<double>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::DOUBLE_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeStringArray(
    const std::string& fieldName, const std::vector<std::string>& value) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName,
                  FieldInfo(PdxFieldTypes::STRING_ARRAY,
                            PdxInstanceImpl::toCacheableField(value)));

  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::writeArrayOfByteArrays(
    const std::string& fieldName, int8_t** value, int32_t arrayLength,
    int32_t* elementLength) {
  isFieldAdded(fieldName);
  fields_.emplace(fieldName, FieldInfo(PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS,
                                       PdxInstanceImpl::toCacheableField(
                                           value, arrayLength, elementLength)));
  return *this;
}

PdxInstanceFactory& PdxInstanceFactory::markIdentityField(
    const std::string& name) {
  if (fields_.find(name) == fields_.end()) {
    throw IllegalStateException(
        "Field must be added before calling MarkIdentityField ");
  }

  identityFields_.insert(name);
  return *this;
}

void PdxInstanceFactory::isFieldAdded(const std::string& name) {
  if (fields_.find(name) != fields_.end()) {
    throw IllegalStateException("Field: " + name +
                                " is either already added into "
                                "PdxInstanceFactory or it is null");
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
