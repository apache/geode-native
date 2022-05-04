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

#include "WritablePdxInstanceImpl.hpp"

#include <algorithm>

#include <geode/Cache.hpp>
#include <geode/PdxFieldTypes.hpp>
#include <geode/PdxReader.hpp>
#include <geode/internal/DataSerializablePrimitive.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "PdxType.hpp"
#include "PdxWriterImpl.hpp"
#include "Utils.hpp"
#include "util/string.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializablePrimitive;

WritablePdxInstanceImpl::WritablePdxInstanceImpl(
    Fields fields, FieldsBuffer buffer, std::shared_ptr<PdxType> pdxType,
    const CacheImpl& cache)
    : PdxInstanceImpl{std::move(fields), std::move(buffer), std::move(pdxType),
                      cache} {}

WritablePdxInstanceImpl::~WritablePdxInstanceImpl() noexcept = default;

std::shared_ptr<PdxSerializable> WritablePdxInstanceImpl::getObject() {
  return PdxInstanceImpl::getObject();
}

bool WritablePdxInstanceImpl::hasField(const std::string& name) {
  return PdxInstanceImpl::hasField(name);
}

bool WritablePdxInstanceImpl::getBooleanField(const std::string& name) const {
  return PdxInstanceImpl::getBooleanField(name);
}

int8_t WritablePdxInstanceImpl::getByteField(const std::string& name) const {
  return PdxInstanceImpl::getByteField(name);
}

int16_t WritablePdxInstanceImpl::getShortField(const std::string& name) const {
  return PdxInstanceImpl::getShortField(name);
}

int32_t WritablePdxInstanceImpl::getIntField(const std::string& name) const {
  return PdxInstanceImpl::getIntField(name);
}

int64_t WritablePdxInstanceImpl::getLongField(const std::string& name) const {
  return PdxInstanceImpl::getLongField(name);
}

float WritablePdxInstanceImpl::getFloatField(const std::string& name) const {
  return PdxInstanceImpl::getFloatField(name);
}

double WritablePdxInstanceImpl::getDoubleField(const std::string& name) const {
  return PdxInstanceImpl::getDoubleField(name);
}

char16_t WritablePdxInstanceImpl::getCharField(const std::string& name) const {
  return PdxInstanceImpl::getCharField(name);
}

std::string WritablePdxInstanceImpl::getStringField(
    const std::string& name) const {
  return PdxInstanceImpl::getStringField(name);
}

std::vector<bool> WritablePdxInstanceImpl::getBooleanArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getBooleanArrayField(name);
}

std::vector<int8_t> WritablePdxInstanceImpl::getByteArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getByteArrayField(name);
}

std::vector<int16_t> WritablePdxInstanceImpl::getShortArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getShortArrayField(name);
}

std::vector<int32_t> WritablePdxInstanceImpl::getIntArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getIntArrayField(name);
}

std::vector<int64_t> WritablePdxInstanceImpl::getLongArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getLongArrayField(name);
}

std::vector<float> WritablePdxInstanceImpl::getFloatArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getFloatArrayField(name);
}

std::vector<double> WritablePdxInstanceImpl::getDoubleArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getDoubleArrayField(name);
}

std::vector<char16_t> WritablePdxInstanceImpl::getCharArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getCharArrayField(name);
}

std::vector<std::string> WritablePdxInstanceImpl::getStringArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getStringArrayField(name);
}

std::shared_ptr<CacheableDate> WritablePdxInstanceImpl::getCacheableDateField(
    const std::string& name) const {
  return PdxInstanceImpl::getCacheableDateField(name);
}

void WritablePdxInstanceImpl::getField(const std::string& name, int8_t*** value,
                                       int32_t& arrayLength,
                                       int32_t*& elementLength) const {
  PdxInstanceImpl::getField(name, value, arrayLength, elementLength);
}

std::shared_ptr<Cacheable> WritablePdxInstanceImpl::getCacheableField(
    const std::string& name) const {
  return PdxInstanceImpl::getCacheableField(name);
}

std::shared_ptr<CacheableObjectArray>
WritablePdxInstanceImpl::getCacheableObjectArrayField(
    const std::string& name) const {
  return PdxInstanceImpl::getCacheableObjectArrayField(name);
}

bool WritablePdxInstanceImpl::isIdentityField(const std::string& name) {
  return PdxInstanceImpl::isIdentityField(name);
}

std::shared_ptr<WritablePdxInstance> WritablePdxInstanceImpl::createWriter() {
  return PdxInstanceImpl::createWriter();
}

int32_t WritablePdxInstanceImpl::hashcode() const {
  return PdxInstanceImpl::hashcode();
}

std::string WritablePdxInstanceImpl::toString() const {
  return PdxInstanceImpl::toString();
}

bool WritablePdxInstanceImpl::operator==(const CacheableKey& other) const {
  return PdxInstanceImpl::operator==(other);
}

size_t WritablePdxInstanceImpl::objectSize() const {
  auto size = sizeof(WritablePdxInstanceImpl);
  size += buffer_.size();
  size += pdxType_->objectSize();
  for (const auto& field : fields_) {
    size += field->objectSize();
  }

  return size;
}

std::shared_ptr<CacheableStringArray> WritablePdxInstanceImpl::getFieldNames() {
  return PdxInstanceImpl::getFieldNames();
}

void WritablePdxInstanceImpl::toData(PdxWriter& writer) const {
  PdxInstanceImpl::toData(writer);
}

void WritablePdxInstanceImpl::fromData(PdxReader& reader) {
  PdxInstanceImpl::fromData(reader);
}

const std::string& WritablePdxInstanceImpl::getClassName() const {
  return PdxInstanceImpl::getClassName();
}

PdxFieldTypes WritablePdxInstanceImpl::getFieldType(
    const std::string& name) const {
  return PdxInstanceImpl::getFieldType(name);
}

void WritablePdxInstanceImpl::setField(const std::string& name, bool value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::BOOLEAN) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int8_t value) {
  auto field = pdxType_->getField(name);

  if (field != nullptr && field->getType() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, uint8_t value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int16_t value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::SHORT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int32_t value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::INT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int64_t value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::LONG) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, float value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::FLOAT) {
    throw IllegalStateException(
        "PdxInstance doesn't have field " + name +
        " or type of field not matched " +
        (field != nullptr ? field->toString().c_str() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, double value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::DOUBLE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       char16_t value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::CHAR) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       std::shared_ptr<CacheableDate> value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::DATE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, value);
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       std::shared_ptr<Cacheable> value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::OBJECT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, value);
}

void WritablePdxInstanceImpl::setField(
    const std::string& name, std::shared_ptr<CacheableObjectArray> value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::OBJECT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, value);
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<bool>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::BOOLEAN_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int8_t>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::BYTE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int16_t>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::SHORT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int32_t>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::INT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int64_t>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::LONG_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<float>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::FLOAT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<double>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::DOUBLE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<char16_t>& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::CHAR_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::string& value) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::STRING) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int8_t** value,
                                       int32_t arrayLength,
                                       int32_t* elementLength) {
  auto field = pdxType_->getField(name);
  if (field != nullptr &&
      field->getType() != PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value, arrayLength, elementLength));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       std::string* value, int32_t length) {
  auto field = pdxType_->getField(name);
  if (field != nullptr && field->getType() != PdxFieldTypes::STRING_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value, length));
}

void WritablePdxInstanceImpl::updateFieldValue(
    std::shared_ptr<PdxFieldType> field, std::shared_ptr<Cacheable> value) {
  buffer_.clear();
  fields_[field->getIndex()] = std::move(value);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
