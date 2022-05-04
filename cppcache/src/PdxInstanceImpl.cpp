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
#include <iomanip>

#include <geode/Cache.hpp>
#include <geode/DataOutput.hpp>
#include <geode/PdxFieldTypes.hpp>
#include <geode/PdxReader.hpp>
#include <geode/internal/DataSerializablePrimitive.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "PdxHelper.hpp"
#include "PdxReaderImpl.hpp"
#include "PdxType.hpp"
#include "PdxWriterImpl.hpp"
#include "Utils.hpp"
#include "WritablePdxInstanceImpl.hpp"

#include "util/hash.hpp"
#include "util/string.hpp"

namespace {

using apache::geode::client::Cacheable;
using apache::geode::client::CharArray;
using apache::geode::client::BooleanArray;
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableByte;
using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableFloat;
using apache::geode::client::CacheableDouble;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableCharacter;
using apache::geode::client::CacheableInt16Array;
using apache::geode::client::CacheableInt32Array;
using apache::geode::client::CacheableInt64Array;
using apache::geode::client::CacheableFloatArray;
using apache::geode::client::CacheableDoubleArray;
using apache::geode::client::CacheableStringArray;
using apache::geode::client::CacheableObjectArray;

template <typename T>
T fromCacheableField(std::shared_ptr<Cacheable> field);

template <>
bool fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableBoolean*>(field.get());
  return value->value();
}

template <>
int8_t fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableByte*>(field.get());
  return value->value();
}

template <>
int16_t fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableInt16*>(field.get());
  return value->value();
}

template <>
int32_t fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableInt32*>(field.get());
  return value->value();
}

template <>
int64_t fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableInt64*>(field.get());
  return value->value();
}

template <>
float fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableFloat*>(field.get());
  return value->value();
}

template <>
double fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableDouble*>(field.get());
  return value->value();
}

template <>
char16_t fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableCharacter*>(field.get());
  return value->value();
}

template <>
std::shared_ptr<CacheableDate> fromCacheableField(
    std::shared_ptr<Cacheable> field) {
  return std::dynamic_pointer_cast<CacheableDate>(field);
}

template <>
std::string fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableString*>(field.get());
  return value->value();
}

template <>
std::vector<bool> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<BooleanArray*>(field.get());
  return value->value();
}

template <>
std::vector<int8_t> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableBytes*>(field.get());
  return value->value();
}

template <>
std::vector<int16_t> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableInt16Array*>(field.get());
  return value->value();
}

template <>
std::vector<int32_t> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableInt32Array*>(field.get());
  return value->value();
}

template <>
std::vector<int64_t> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableInt64Array*>(field.get());
  return value->value();
}

template <>
std::vector<float> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableFloatArray*>(field.get());
  return value->value();
}

template <>
std::vector<double> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CacheableDoubleArray*>(field.get());
  return value->value();
}

template <>
std::vector<char16_t> fromCacheableField(std::shared_ptr<Cacheable> field) {
  auto value = dynamic_cast<CharArray*>(field.get());
  return value->value();
}

template <>
std::vector<std::vector<int8_t>> fromCacheableField(
    std::shared_ptr<Cacheable> field) {
  std::vector<std::vector<int8_t>> result;
  auto vector = dynamic_cast<CacheableVector*>(field.get());

  result.reserve(vector->size());
  for (const auto& entry : *vector) {
    auto&& val = dynamic_cast<CacheableBytes*>(entry.get());
    result.emplace_back(val->value());
  }

  return result;
}

template <>
std::shared_ptr<CacheableObjectArray> fromCacheableField(
    std::shared_ptr<Cacheable> field) {
  return std::dynamic_pointer_cast<CacheableObjectArray>(field);
}

template <>
std::vector<std::string> fromCacheableField(std::shared_ptr<Cacheable> field) {
  std::vector<std::string> result;
  auto array = dynamic_cast<CacheableStringArray*>(field.get());
  result.reserve(array->length());
  for (const auto& entry : array->value()) {
    result.emplace_back(entry->value());
  }

  return result;
}

int8_t** fromCacheableField(std::shared_ptr<Cacheable> field,
                            int32_t& arrayLength, int32_t*& elementLength) {
  auto vector = dynamic_cast<CacheableVector*>(field.get());
  if(vector->empty()) {
    arrayLength = 0;
    elementLength = nullptr;

    return nullptr;
  }

  const auto size = vector->size();
  int8_t** values = new int8_t*[size];
  auto lengths = new int32_t[size];

  size_t i = 0;
  for (auto&& entry : *vector) {
    int32_t len;
    int8_t *buffer;
    auto&& val = std::dynamic_pointer_cast<CacheableBytes>(entry);

    if(val) {
      len = val->length();
      if(len > 0) {
        buffer = new int8_t[len];
        std::memcpy(buffer, val->value().data(), len);
      } else {
        buffer = nullptr;
      }

      values[i] = buffer;
      lengths[i] = len;
    } else {
      len = 0;
      buffer = nullptr;
    }

    values[i] = buffer;
    lengths[i++] = len;
  }

  arrayLength = static_cast<int32_t>(size);
  elementLength = lengths;
  return values;
}

template <typename T>
bool compareArray(std::shared_ptr<Cacheable> value,
                  std::shared_ptr<Cacheable> otherValue) {
  using VectorType = std::vector<T>;
  auto vector = fromCacheableField<VectorType>(value);
  auto otherVector = fromCacheableField<VectorType>(otherValue);

  if (vector.size() != otherVector.size()) {
    return false;
  }

  return std::equal(vector.begin(), vector.end(), otherVector.begin());
}

template <typename T>
bool arrayHashCode(std::shared_ptr<Cacheable> value) {
  using VectorType = std::vector<T>;
  return std::hash<VectorType>{}(fromCacheableField<VectorType>(value));
}

}  // namespace

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializablePrimitive;

PdxInstanceImpl::PdxInstanceImpl(Fields fields,
                                 std::shared_ptr<PdxType> pdxType,
                                 const CacheImpl& cache)
    : PdxInstanceImpl{fields, FieldsBuffer{}, pdxType, cache} {}

PdxInstanceImpl::PdxInstanceImpl(FieldsBuffer buffer,
                                 std::shared_ptr<PdxType> pdxType,
                                 const CacheImpl& cache)
    : PdxInstanceImpl{Fields{}, buffer, pdxType, cache} {
  deserialize();
}

PdxInstanceImpl::PdxInstanceImpl(Fields fields, FieldsBuffer buffer,
                                 std::shared_ptr<PdxType> pdxType,
                                 const CacheImpl& cache)
    : fields_{std::move(fields)},
      pdxType_{pdxType},
      buffer_{std::move(buffer)},
      cacheStats_(const_cast<CacheImpl&>(cache).getCachePerfStats()),
      cache_(cache),
      enableTimeStatistics_(cache.getDistributedSystem()
                                .getSystemProperties()
                                .getEnableTimeStatistics()) {
  cacheStats_.incPdxInstanceCreations();
}

PdxInstanceImpl::~PdxInstanceImpl() noexcept = default;

std::shared_ptr<PdxSerializable> PdxInstanceImpl::getObject() {
  auto buffer = getFieldsBuffer();
  auto input = cache_.createDataInput(buffer.data(), buffer.size());

  int64_t sampleStartNanos =
      enableTimeStatistics_ ? Utils::startStatOpTime() : 0;

  auto object =
      PdxHelper::deserializePdxSerializable(input, pdxType_, buffer.size());

  if (enableTimeStatistics_) {
    Utils::updateStatOpTime(cacheStats_.getStat(),
                            cacheStats_.getPdxInstanceDeserializationTimeId(),
                            sampleStartNanos);
  }

  cacheStats_.incPdxInstanceDeserializations();
  return object;
}

bool PdxInstanceImpl::hasField(const std::string& name) {
  return pdxType_->getField(name) != nullptr;
}

std::shared_ptr<Cacheable> PdxInstanceImpl::getCacheableField(
    const std::string& name) const {
  return getField(name, PdxFieldTypes::OBJECT);
}

bool PdxInstanceImpl::getBooleanField(const std::string& name) const {
  auto field = dynamic_cast<CacheableBoolean*>(
      getField(name, PdxFieldTypes::BOOLEAN).get());
  return field->value();
}

int8_t PdxInstanceImpl::getByteField(const std::string& name) const {
  auto field =
      dynamic_cast<CacheableByte*>(getField(name, PdxFieldTypes::BYTE).get());
  return field->value();
}

int16_t PdxInstanceImpl::getShortField(const std::string& name) const {
  return fromCacheableField<int16_t>(getField(name, PdxFieldTypes::SHORT));
}

int32_t PdxInstanceImpl::getIntField(const std::string& name) const {
  return fromCacheableField<int32_t>(getField(name, PdxFieldTypes::INT));
}

int64_t PdxInstanceImpl::getLongField(const std::string& name) const {
  return fromCacheableField<int64_t>(getField(name, PdxFieldTypes::LONG));
}

float PdxInstanceImpl::getFloatField(const std::string& name) const {
  return fromCacheableField<float>(getField(name, PdxFieldTypes::FLOAT));
}

double PdxInstanceImpl::getDoubleField(const std::string& name) const {
  return fromCacheableField<double>(getField(name, PdxFieldTypes::DOUBLE));
}

char16_t PdxInstanceImpl::getCharField(const std::string& name) const {
  return fromCacheableField<char16_t>(getField(name, PdxFieldTypes::CHAR));
}

std::string PdxInstanceImpl::getStringField(const std::string& name) const {
  return fromCacheableField<std::string>(getField(name, PdxFieldTypes::STRING));
}

std::vector<bool> PdxInstanceImpl::getBooleanArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<bool>>(
      getField(name, PdxFieldTypes::BOOLEAN_ARRAY));
}

std::vector<int8_t> PdxInstanceImpl::getByteArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<int8_t>>(
      getField(name, PdxFieldTypes::BYTE_ARRAY));
}

std::vector<int16_t> PdxInstanceImpl::getShortArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<int16_t>>(
      getField(name, PdxFieldTypes::SHORT_ARRAY));
}

std::vector<int32_t> PdxInstanceImpl::getIntArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<int32_t>>(
      getField(name, PdxFieldTypes::INT_ARRAY));
}

std::vector<int64_t> PdxInstanceImpl::getLongArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<int64_t>>(
      getField(name, PdxFieldTypes::LONG_ARRAY));
}

std::vector<float> PdxInstanceImpl::getFloatArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<float>>(
      getField(name, PdxFieldTypes::FLOAT_ARRAY));
}

std::vector<double> PdxInstanceImpl::getDoubleArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<double>>(
      getField(name, PdxFieldTypes::DOUBLE_ARRAY));
}

std::vector<char16_t> PdxInstanceImpl::getCharArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<char16_t>>(
      getField(name, PdxFieldTypes::CHAR_ARRAY));
}

std::vector<std::string> PdxInstanceImpl::getStringArrayField(
    const std::string& name) const {
  return fromCacheableField<std::vector<std::string>>(
      getField(name, PdxFieldTypes::STRING_ARRAY));
}

std::shared_ptr<CacheableDate> PdxInstanceImpl::getCacheableDateField(
    const std::string& name) const {
  return fromCacheableField<std::shared_ptr<CacheableDate>>(
      getField(name, PdxFieldTypes::DATE));
}

void PdxInstanceImpl::getField(const std::string& name, int8_t*** value,
                               int32_t& arrayLength,
                               int32_t*& elementLength) const {
  *value =
      fromCacheableField(getField(name, PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS),
                         arrayLength, elementLength);
}

std::shared_ptr<CacheableObjectArray>
PdxInstanceImpl::getCacheableObjectArrayField(const std::string& name) const {
  return fromCacheableField<std::shared_ptr<CacheableObjectArray>>(
      getField(name, PdxFieldTypes::OBJECT_ARRAY));
}

bool PdxInstanceImpl::isIdentityField(const std::string& name) {
  auto field = pdxType_->getField(name);
  return field != nullptr && field->isIdentity();
}

std::shared_ptr<WritablePdxInstance> PdxInstanceImpl::createWriter() {
  LOGDEBUG("PdxInstanceImpl::createWriter fields_.size() = %zu type_ = %d ",
           fields_.size(), pdxType_->getTypeId());
  return std::make_shared<WritablePdxInstanceImpl>(fields_, buffer_, pdxType_,
                                                   cache_);
}

int32_t PdxInstanceImpl::hashcode() const {
  int hashCode = 1;
  auto input = cache_.createDataInput(buffer_.data(), buffer_.size());

  for (const auto& entry : pdxType_->getFieldMap()) {
    const auto& field = entry.second;
    if (!field->isIdentity()) {
      continue;
    }

    auto value = fields_.at(field->getIndex());
    switch (field->getType()) {
      case PdxFieldTypes::CHAR:
      case PdxFieldTypes::BOOLEAN:
      case PdxFieldTypes::BYTE:
      case PdxFieldTypes::SHORT:
      case PdxFieldTypes::INT:
      case PdxFieldTypes::LONG:
      case PdxFieldTypes::DATE:
      case PdxFieldTypes::FLOAT:
      case PdxFieldTypes::DOUBLE:
      case PdxFieldTypes::STRING: {
        hashCode = 31 * hashCode +
                   dynamic_cast<CacheableKey*>(value.get())->hashcode();
        break;
      }
      case PdxFieldTypes::BOOLEAN_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<bool>(value);
        break;
      case PdxFieldTypes::CHAR_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<char16_t>(value);
        break;
      case PdxFieldTypes::BYTE_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<int8_t>(value);
        break;
      case PdxFieldTypes::SHORT_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<int16_t>(value);
        break;
      case PdxFieldTypes::INT_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<int32_t>(value);
        break;
      case PdxFieldTypes::LONG_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<int64_t>(value);
        break;
      case PdxFieldTypes::FLOAT_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<float>(value);
        break;
      case PdxFieldTypes::DOUBLE_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<double>(value);
        break;
      case PdxFieldTypes::STRING_ARRAY:
        hashCode = 31 * hashCode + arrayHashCode<std::string>(value);
        break;
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS:
        hashCode = 31 * hashCode + arrayHashCode<std::vector<int8_t>>(value);
        break;
      case PdxFieldTypes::OBJECT:
      case PdxFieldTypes::OBJECT_ARRAY: {
        if (value != nullptr) {
          hashCode = 31 * hashCode + deepHashCode(value);
        }
        break;
      }
      case PdxFieldTypes::UNKNOWN: {
        throw IllegalStateException(
            "PdxInstance not found typeid " +
            std::to_string(static_cast<int>(field->getType())));
      }
    }
  }

  return hashCode;
}

std::string PdxInstanceImpl::toString() const {
  std::string result = "PDX[" + std::to_string(pdxType_->getTypeId()) + "," +
                       pdxType_->getPdxClassName() + "]{";

  bool firstElement = true;
  for (const auto& entry : pdxType_->getFieldMap()) {
    const auto& field = entry.second;
    if (!field->isIdentity()) {
      continue;
    }

    if (firstElement) {
      firstElement = false;
    } else {
      result += ",";
    }
    result += field->getName();
    result += "=";

    auto fieldValue = fields_.at(field->getIndex());
    switch (field->getType()) {
      case PdxFieldTypes::BOOLEAN: {
        auto&& value = fromCacheableField<bool>(fieldValue);
        result += value ? "true" : "false";
        break;
      }
      case PdxFieldTypes::BYTE: {
        auto&& value = fromCacheableField<int8_t>(fieldValue);
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::SHORT: {
        int16_t value = fromCacheableField<int16_t>(fieldValue);
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::INT: {
        int32_t value = fromCacheableField<int32_t>(fieldValue);
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::LONG: {
        int64_t value = fromCacheableField<int64_t>(fieldValue);
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::FLOAT: {
        float value = fromCacheableField<float>(fieldValue);
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::DOUBLE: {
        double value = fromCacheableField<double>(fieldValue);
        result += std::to_string(value);
        break;
      }
      case PdxFieldTypes::CHAR: {
        auto value = fromCacheableField<char16_t>(fieldValue);
        result += to_utf8(std::u16string{value});
        break;
      }
      case PdxFieldTypes::DATE: {
        auto value =
            fromCacheableField<std::shared_ptr<CacheableDate>>(fieldValue);
        if (value != nullptr) {
          result += value->toString();
        }
        break;
      }
      case PdxFieldTypes::STRING: {
        auto value = fromCacheableField<std::string>(fieldValue);
        result += value;
        break;
      }
      case PdxFieldTypes::BOOLEAN_ARRAY: {
        auto value = fromCacheableField<std::vector<bool>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += v ? "true" : "false";
          }
        }
        break;
      }
      case PdxFieldTypes::BYTE_ARRAY: {
        auto value = fromCacheableField<std::vector<int8_t>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::SHORT_ARRAY: {
        auto value = fromCacheableField<std::vector<int16_t>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::INT_ARRAY: {
        auto value = fromCacheableField<std::vector<int32_t>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::LONG_ARRAY: {
        auto value = fromCacheableField<std::vector<int64_t>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::FLOAT_ARRAY: {
        auto value = fromCacheableField<std::vector<float>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::DOUBLE_ARRAY: {
        auto value = fromCacheableField<std::vector<double>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          for (auto&& v : value) {
            result += std::to_string(v);
          }
        }
        break;
      }
      case PdxFieldTypes::CHAR_ARRAY: {
        auto value = fromCacheableField<std::vector<char16_t>>(fieldValue);
        auto length = value.size();
        if (length > 0) {
          result += to_utf8(std::u16string(value.data(), length));
        }
        break;
      }
      case PdxFieldTypes::STRING_ARRAY: {
        auto value = fromCacheableField<std::vector<std::string>>(fieldValue);
        for (auto&& v : value) {
          result += v;
        }
        break;
      }
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
        auto value =
            fromCacheableField<std::vector<std::vector<int8_t>>>(fieldValue);
        auto len = value.size();
        if (len > 0) {
          for (const auto& row : value) {
            for (auto x : row) {
              result += std::to_string(x);
            }
          }
        }
        break;
      }
      case PdxFieldTypes::OBJECT_ARRAY: {
        auto value = getCacheableObjectArrayField(field->getName());
        if (value != nullptr) {
          result += value->toString();
        }
        break;
      }
      case PdxFieldTypes::OBJECT:
      case PdxFieldTypes::UNKNOWN: {
        auto value = getCacheableField(field->getName());
        if (value != nullptr) {
          result += value->toString();
        }
      }
    }
  }
  result += "}";

  return result;
}

bool PdxInstanceImpl::operator==(const CacheableKey& o) const {
  PdxInstanceImpl* other =
      dynamic_cast<PdxInstanceImpl*>(const_cast<CacheableKey*>(&o));

  if (other == nullptr) {
    return false;
  }

  if (this == other) {
    return true;
  }

  auto otherType = other->pdxType_;
  if (pdxType_->getPdxClassName() != otherType->getPdxClassName()) {
    return false;
  }

  std::vector<std::shared_ptr<PdxFieldType>> identityFields;
  for (const auto& entry : pdxType_->getFieldMap()) {
    const auto& field = entry.second;
    if (field->isIdentity()) {
      identityFields.emplace_back(field);
    }
  }

  std::vector<std::shared_ptr<PdxFieldType>> otherIdentityFields;
  for (const auto& entry : otherType->getFieldMap()) {
    const auto& field = entry.second;
    if (field->isIdentity()) {
      otherIdentityFields.emplace_back(field);
    }
  }

  if (identityFields.size() != otherIdentityFields.size()) {
    return false;
  }

  for (auto iter = identityFields.begin(),
            otherIter = otherIdentityFields.begin(), end = identityFields.end();
       iter != end;) {
    const auto& field = **iter++;
    const auto& otherField = **otherIter++;
    if (field != otherField) {
      return false;
    }

    auto value = fields_.at(field.getIndex());
    auto otherValue = other->fields_.at(field.getIndex());
    switch (field.getType()) {
      case PdxFieldTypes::CHAR:
      case PdxFieldTypes::BOOLEAN:
      case PdxFieldTypes::BYTE:
      case PdxFieldTypes::SHORT:
      case PdxFieldTypes::INT:
      case PdxFieldTypes::LONG:
      case PdxFieldTypes::DATE:
      case PdxFieldTypes::FLOAT:
      case PdxFieldTypes::DOUBLE:
      case PdxFieldTypes::STRING: {
        if (!(dynamic_cast<CacheableKey&>(*value) ==
              dynamic_cast<CacheableKey&>(*otherValue))) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::BOOLEAN_ARRAY: {
        if (!compareArray<bool>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::CHAR_ARRAY: {
        if (!compareArray<char16_t>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::BYTE_ARRAY: {
        if (!compareArray<int8_t>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::SHORT_ARRAY: {
        if (!compareArray<int16_t>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::INT_ARRAY: {
        if (!compareArray<int32_t>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::LONG_ARRAY: {
        if (!compareArray<int64_t>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::FLOAT_ARRAY: {
        if (!compareArray<float>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::DOUBLE_ARRAY: {
        if (!compareArray<double>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::STRING_ARRAY: {
        if (!compareArray<std::string>(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
        using VectorType = std::vector<std::vector<int8_t>>;

        auto vector = fromCacheableField<VectorType>(value);
        auto otherVector = fromCacheableField<VectorType>(otherValue);

        if (vector.size() != otherVector.size()) {
          return false;
        }

        for (auto vectorIter = vector.begin(),
                  otherVectorIter = otherVector.begin(),
                  vectorEnd = vector.end();
             vectorIter != vectorEnd;) {
          const auto& row = *vectorIter++;
          const auto& otherRow = *otherVectorIter++;
          if (row.size() != otherRow.size() ||
              !std::equal(row.begin(), row.end(), otherRow.begin())) {
            return false;
          }
        }
        break;
      }
      case PdxFieldTypes::OBJECT:
      case PdxFieldTypes::OBJECT_ARRAY: {
        if (!deepArrayEquals(value, otherValue)) {
          return false;
        }
        break;
      }
      case PdxFieldTypes::UNKNOWN: {
        throw IllegalStateException(
            std::string{"Field \""} + field.getName() +
            std::string{"\" has an unknown type ("} +
            std::to_string(static_cast<int32_t>(field.getType())) + ')');
      }
    }
  }

  return true;
}

size_t PdxInstanceImpl::objectSize() const {
  auto size = sizeof(PdxInstanceImpl);
  size += buffer_.size();
  size += pdxType_->objectSize();

  for (const auto& field : fields_) {
    size += field->objectSize();
  }

  return size;
}

std::shared_ptr<CacheableStringArray> PdxInstanceImpl::getFieldNames() {
  const auto& fields = pdxType_->getFields();

  if (fields.empty()) {
    return nullptr;
  }

  std::vector<std::shared_ptr<CacheableString>> result;
  result.reserve(fields.size());
  for (auto&& field : fields) {
    result.emplace_back(CacheableString::create(field->getName()));
  }
  return CacheableStringArray::create(std::move(result));
}

void PdxInstanceImpl::toData(PdxWriter& writer) const {
  auto& writerImpl = dynamic_cast<PdxWriterImpl&>(writer);
  serialize(writerImpl);

  boost::unique_lock<decltype(serializationMutex_)> guard{serializationMutex_};
  buffer_ = writerImpl.getFieldsBuffer();
}

void PdxInstanceImpl::fromData(PdxReader&) {
  throw IllegalStateException(
      "PdxInstance::fromData( .. ) shouldn't have called");
}

const std::string& PdxInstanceImpl::getClassName() const {
  return pdxType_->getPdxClassName();
}

PdxFieldTypes PdxInstanceImpl::getFieldType(const std::string& name) const {
  auto field = pdxType_->getField(name);

  if (!field) {
    throw IllegalStateException("PdxInstance doesn't have field " + name);
  }

  return field->getType();
}

std::shared_ptr<Cacheable> PdxInstanceImpl::getField(const std::string& name,
                                                     PdxFieldTypes type) const {
  auto field = pdxType_->getField(name);
  if (!field) {
    throw IllegalStateException("PdxInstance doesn't have field \"" + name +
                                "\"");
  }

  if (field->getType() != type) {
    throw IllegalStateException(
        "PdxInstance field \"" + name + "\" type mismatch. Expected is " +
        std::to_string(static_cast<int32_t>(type)) + " but got " +
        std::to_string(static_cast<int32_t>(field->getType())));
  }

  return fields_.at(field->getIndex());
}

std::vector<uint8_t> PdxInstanceImpl::getFieldsBuffer() const {
  boost::upgrade_lock<decltype(serializationMutex_)> lock{serializationMutex_};

  if (!buffer_.empty()) {
    return buffer_;
  }

  auto buffer = serialize();
  boost::upgrade_to_unique_lock<decltype(serializationMutex_)> uniqueLock{lock};
  return buffer_ = buffer;
}

std::vector<uint8_t> PdxInstanceImpl::serialize() const {
  auto output = cache_.createDataOutput();
  PdxWriterImpl writer{output};
  serialize(writer);

  return writer.getFieldsBuffer();
}

void PdxInstanceImpl::serialize(PdxWriterImpl& writer) const {
  int32_t idx = 0;
  for (const auto& value : fields_) {
    auto&& field = pdxType_->getField(idx++);
    writeField(writer, field->getType(), value);
  }

  writer.completeSerialization();
}

void PdxInstanceImpl::writeField(PdxWriterImpl& writer, PdxFieldTypes type,
                                 std::shared_ptr<Cacheable> value) {
  switch (type) {
    case PdxFieldTypes::BOOLEAN:
      writer.writeBoolean(fromCacheableField<bool>(value));
      break;
    case PdxFieldTypes::BYTE:
      writer.writeByte(fromCacheableField<int8_t>(value));
      break;
    case PdxFieldTypes::SHORT:
      writer.writeShort(fromCacheableField<int16_t>(value));
      break;
    case PdxFieldTypes::INT:
      writer.writeInt(fromCacheableField<int32_t>(value));
      break;
    case PdxFieldTypes::LONG:
      writer.writeLong(fromCacheableField<int64_t>(value));
      break;
    case PdxFieldTypes::FLOAT:
      writer.writeFloat(fromCacheableField<float>(value));
      break;
    case PdxFieldTypes::DOUBLE:
      writer.writeDouble(fromCacheableField<double>(value));
      break;
    case PdxFieldTypes::CHAR:
      writer.writeChar(fromCacheableField<char16_t>(value));
      break;
    case PdxFieldTypes::DATE:
      writer.writeDate(
          fromCacheableField<std::shared_ptr<CacheableDate>>(value));
      break;
    case PdxFieldTypes::STRING:
      writer.writeString(fromCacheableField<std::string>(value));
      break;
    case PdxFieldTypes::BOOLEAN_ARRAY:
      writer.writeBooleanArray(fromCacheableField<std::vector<bool>>(value));
      break;
    case PdxFieldTypes::BYTE_ARRAY:
      writer.writeByteArray(fromCacheableField<std::vector<int8_t>>(value));
      break;
    case PdxFieldTypes::SHORT_ARRAY:
      writer.writeShortArray(fromCacheableField<std::vector<int16_t>>(value));
      break;
    case PdxFieldTypes::INT_ARRAY:
      writer.writeIntArray(fromCacheableField<std::vector<int32_t>>(value));
      break;
    case PdxFieldTypes::LONG_ARRAY:
      writer.writeLongArray(fromCacheableField<std::vector<int64_t>>(value));
      break;
    case PdxFieldTypes::FLOAT_ARRAY:
      writer.writeFloatArray(fromCacheableField<std::vector<float>>(value));
      break;
    case PdxFieldTypes::DOUBLE_ARRAY:
      writer.writeDoubleArray(fromCacheableField<std::vector<double>>(value));
      break;
    case PdxFieldTypes::CHAR_ARRAY:
      writer.writeCharArray(fromCacheableField<std::vector<char16_t>>(value));
      break;
    case PdxFieldTypes::STRING_ARRAY:
      writer.writeStringArray(
          fromCacheableField<std::vector<std::string>>(value));
      break;
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
        writer.writeArrayOfByteArrays(values, static_cast<int>(size), lengths);
        delete[] values;
        delete[] lengths;
      }
      break;
    }
    case PdxFieldTypes::OBJECT_ARRAY:
      writer.writeObjectArray(
          fromCacheableField<std::shared_ptr<CacheableObjectArray>>(value));
      break;
    case PdxFieldTypes::UNKNOWN:
    case PdxFieldTypes::OBJECT: {
      writer.writeObject(value);
    }
  }
}

void PdxInstanceImpl::deserialize() {
  auto input = cache_.createDataInput(buffer_.data(), buffer_.size());
  PdxReaderImpl reader{input, pdxType_, static_cast<int32_t>(buffer_.size())};

  fields_.resize(pdxType_->getFieldsCount());
  for (const auto& field : pdxType_->getFields()) {
    auto& fieldRef = fields_[field->getIndex()];
    switch (field->getType()) {
      case PdxFieldTypes::BOOLEAN:
        fieldRef = toCacheableField(reader.readBoolean(field));
        break;
      case PdxFieldTypes::BYTE:
        fieldRef = toCacheableField(reader.readByte(field));
        break;
      case PdxFieldTypes::CHAR:
        fieldRef = toCacheableField(reader.readChar(field));
        break;
      case PdxFieldTypes::SHORT:
        fieldRef = toCacheableField(reader.readShort(field));
        break;
      case PdxFieldTypes::INT:
        fieldRef = toCacheableField(reader.readInt(field));
        break;
      case PdxFieldTypes::LONG:
        fieldRef = toCacheableField(reader.readLong(field));
        break;
      case PdxFieldTypes::FLOAT:
        fieldRef = toCacheableField(reader.readFloat(field));
        break;
      case PdxFieldTypes::DOUBLE:
        fieldRef = toCacheableField(reader.readDouble(field));
        break;
      case PdxFieldTypes::DATE:
        fieldRef = reader.readDate(field);
        break;
      case PdxFieldTypes::STRING:
        fieldRef = toCacheableField(reader.readString(field));
        break;
      case PdxFieldTypes::OBJECT:
        fieldRef = reader.readObject(field);
        break;
      case PdxFieldTypes::BOOLEAN_ARRAY:
        fieldRef = toCacheableField(reader.readBooleanArray(field));
        break;
      case PdxFieldTypes::CHAR_ARRAY:
        fieldRef = toCacheableField(reader.readCharArray(field));
        break;
      case PdxFieldTypes::BYTE_ARRAY:
        fieldRef = toCacheableField(reader.readByteArray(field));
        break;
      case PdxFieldTypes::SHORT_ARRAY:
        fieldRef = toCacheableField(reader.readShortArray(field));
        break;
      case PdxFieldTypes::INT_ARRAY:
        fieldRef = toCacheableField(reader.readIntArray(field));
        break;
      case PdxFieldTypes::LONG_ARRAY:
        fieldRef = toCacheableField(reader.readLongArray(field));
        break;
      case PdxFieldTypes::FLOAT_ARRAY:
        fieldRef = toCacheableField(reader.readFloatArray(field));
        break;
      case PdxFieldTypes::DOUBLE_ARRAY:
        fieldRef = toCacheableField(reader.readDoubleArray(field));
        break;
      case PdxFieldTypes::STRING_ARRAY:
        fieldRef = toCacheableField(reader.readStringArray(field));
        break;
      case PdxFieldTypes::OBJECT_ARRAY:
        fieldRef = reader.readObjectArray(field);
        break;
      case PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS: {
        int32_t length;
        int32_t* elementsLength;
        auto array =
            reader.readArrayOfByteArrays(field, length, &elementsLength);
        fieldRef = toCacheableField(array, length, elementsLength);
        for (auto i = 0; i < length;) {
          delete array[i++];
        }

        delete elementsLength;
        delete array;
        break;
      }
      default:
      case PdxFieldTypes::UNKNOWN:
        throw IllegalStateException(
            "Field \"" + field->getName() + "\" has an invalid type " +
            std::to_string(static_cast<int32_t>(field->getType())));
        break;
    }
  }
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(bool value) {
  return CacheableBoolean::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(int8_t value) {
  return CacheableByte::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(uint8_t value) {
  return CacheableByte::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(int16_t value) {
  return CacheableInt16::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(int32_t value) {
  return CacheableInt32::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(int64_t value) {
  return CacheableInt64::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(float value) {
  return CacheableFloat::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(double value) {
  return CacheableDouble::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(char value) {
  return CacheableCharacter::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(char16_t value) {
  return CacheableCharacter::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<bool>& value) {
  return BooleanArray::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<int8_t>& value) {
  return CacheableBytes::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<int16_t>& value) {
  return CacheableInt16Array::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<int32_t>& value) {
  return CacheableInt32Array::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<int64_t>& value) {
  return CacheableInt64Array::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<float>& value) {
  return CacheableFloatArray::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<double>& value) {
  return CacheableDoubleArray::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<char16_t>& value) {
  return CharArray::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::string& value) {
  return CacheableString::create(value);
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    int8_t** value, int32_t arrayLength, int32_t* elementLength) {
  auto vector = CacheableVector::create();
  for (int i = 0; i < arrayLength; i++) {
    vector->emplace_back(CacheableBytes::create(
        std::vector<int8_t>(value[i], value[i] + elementLength[i])));
  }

  return vector;
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(
    const std::vector<std::string>& value) {
  std::vector<std::shared_ptr<CacheableString>> array;

  array.reserve(value.size());
  for (const auto& entry : value) {
    array.emplace_back(CacheableString::create(entry));
  }

  return CacheableStringArray::create(std::move(array));
}

std::shared_ptr<Cacheable> PdxInstanceImpl::toCacheableField(std::string* value,
                                                             int32_t length) {
  std::vector<std::shared_ptr<CacheableString>> array;

  array.reserve(length);
  for (int32_t i = 0; i < length;) {
    array.emplace_back(CacheableString::create(value[i++]));
  }

  return CacheableStringArray::create(std::move(array));
}

int PdxInstanceImpl::enumerateMapHashCode(
    std::shared_ptr<CacheableHashMap> map) {
  int h = 0;
  for (const auto& itr : *map) {
    h = h + ((deepHashCode(itr.first)) ^
             ((itr.second) ? deepHashCode(itr.second) : 0));
  }
  return h;
}

int PdxInstanceImpl::enumerateSetHashCode(
    std::shared_ptr<CacheableHashSet> set) {
  int h = 0;
  for (const auto& itr : *set) {
    h = h + deepHashCode(itr);
  }
  return h;
}

int PdxInstanceImpl::enumerateLinkedSetHashCode(
    std::shared_ptr<CacheableLinkedHashSet> set) {
  int h = 0;
  for (const auto& itr : *set) {
    h = h + deepHashCode(itr);
  }
  return h;
}

int PdxInstanceImpl::enumerateHashTableCode(
    std::shared_ptr<CacheableHashTable> hashTable) {
  int h = 0;
  for (const auto& itr : *hashTable) {
    h = h + ((deepHashCode(itr.first)) ^
             ((itr.second) ? deepHashCode(itr.second) : 0));
  }
  return h;
}

int PdxInstanceImpl::enumerateObjectArrayHashCode(
    std::shared_ptr<CacheableObjectArray> objArray) {
  int h = 1;
  for (const auto& obj : *objArray) {
    h = h * 31 + deepHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::enumerateVectorHashCode(
    std::shared_ptr<CacheableVector> vec) {
  int h = 1;
  for (const auto& obj : *vec) {
    h = h * 31 + deepHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::enumerateArrayListHashCode(
    std::shared_ptr<CacheableArrayList> arrList) {
  int h = 1;
  for (const auto& obj : *arrList) {
    h = h * 31 + deepHashCode(obj);
  }
  return h;
}

int PdxInstanceImpl::enumerateLinkedListHashCode(
    std::shared_ptr<CacheableLinkedList> linkedList) {
  int h = 1;
  for (const auto& obj : *linkedList) {
    h = h * 31 + deepHashCode(obj);
  }
  return h;
}

int32_t PdxInstanceImpl::deepHashCode(std::shared_ptr<Cacheable> obj) {
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
      case DSCode::CacheableUserData:
      case DSCode::CacheableUserData2:
      case DSCode::CacheableUserData4:
      case DSCode::PDX:
      case DSCode::PDX_ENUM:
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
    std::shared_ptr<CacheableHashSet> object,
    std::shared_ptr<CacheableHashSet> otherObject) {
  auto null = object == nullptr;
  auto otherNull = otherObject == nullptr;

  if (null && otherNull) {
    return true;
  } else if (null != otherNull) {
    return false;
  }

  if (object->size() != otherObject->size()) {
    return false;
  }

  for (const auto& iter : *object) {
    if (otherObject->find(iter) == otherObject->end()) {
      return false;
    }
  }

  return true;
}

bool PdxInstanceImpl::enumerateLinkedSetEquals(
    std::shared_ptr<CacheableLinkedHashSet> object,
    std::shared_ptr<CacheableLinkedHashSet> otherObject) {
  auto null = object == nullptr;
  auto otherNull = otherObject == nullptr;

  if (null && otherNull) {
    return true;
  } else if (null != otherNull) {
    return false;
  }

  if (object->size() != otherObject->size()) {
    return false;
  }

  for (const auto& iter : *object) {
    if (otherObject->find(iter) == otherObject->end()) {
      return false;
    }
  }

  return true;
}

bool PdxInstanceImpl::deepArrayEquals(std::shared_ptr<Cacheable> object,
                                      std::shared_ptr<Cacheable> otherObject) {
  auto null = object == nullptr;
  auto otherNull = otherObject == nullptr;

  if (null && otherNull) {
    return true;
  } else if (null != otherNull) {
    return false;
  }

  if (auto primitive =
          std::dynamic_pointer_cast<DataSerializablePrimitive>(object)) {
    switch (primitive->getDsCode()) {
      case DSCode::CacheableObjectArray: {
        auto objArrayPtr =
            std::dynamic_pointer_cast<CacheableObjectArray>(object);
        auto otherObjArrayPtr =
            std::dynamic_pointer_cast<CacheableObjectArray>(otherObject);
        return enumerateObjectArrayEquals(objArrayPtr, otherObjArrayPtr);
      }
      case DSCode::CacheableVector: {
        auto vec = std::dynamic_pointer_cast<CacheableVector>(object);
        auto otherVec = std::dynamic_pointer_cast<CacheableVector>(otherObject);
        return enumerateVectorEquals(vec, otherVec);
      }
      case DSCode::CacheableArrayList: {
        auto arrList = std::dynamic_pointer_cast<CacheableArrayList>(object);
        auto otherArrList =
            std::dynamic_pointer_cast<CacheableArrayList>(otherObject);
        return enumerateArrayListEquals(arrList, otherArrList);
      }
      case DSCode::CacheableHashMap: {
        auto map = std::dynamic_pointer_cast<CacheableHashMap>(object);
        auto otherMap =
            std::dynamic_pointer_cast<CacheableHashMap>(otherObject);
        return enumerateMapEquals(map, otherMap);
      }
      case DSCode::CacheableHashSet: {
        auto hashset = std::dynamic_pointer_cast<CacheableHashSet>(object);
        auto otherHashset =
            std::dynamic_pointer_cast<CacheableHashSet>(otherObject);
        return enumerateSetEquals(hashset, otherHashset);
      }
      case DSCode::CacheableLinkedHashSet: {
        auto linkedHashset =
            std::dynamic_pointer_cast<CacheableLinkedHashSet>(object);
        auto otherLinkedHashset =
            std::dynamic_pointer_cast<CacheableLinkedHashSet>(otherObject);
        return enumerateLinkedSetEquals(linkedHashset, otherLinkedHashset);
      }
      case DSCode::CacheableHashTable: {
        auto hashTable = std::dynamic_pointer_cast<CacheableHashTable>(object);
        auto otherhashTable =
            std::dynamic_pointer_cast<CacheableHashTable>(otherObject);
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
      case DSCode::CacheableUserData:
      case DSCode::CacheableUserData2:
      case DSCode::CacheableUserData4:
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
      case DSCode::PDX:
      case DSCode::PDX_ENUM:
      case DSCode::CacheableString:
      case DSCode::CacheableNullString:
      case DSCode::CacheableASCIIString:
      case DSCode::CacheableASCIIStringHuge:
      case DSCode::CacheableStringHuge:
        break;
    }
  }

  if (auto pdxInstance = std::dynamic_pointer_cast<PdxInstance>(object)) {
    if (auto otherPdxInstance =
            std::dynamic_pointer_cast<PdxInstance>(otherObject)) {
      return *pdxInstance == *otherPdxInstance;
    }
  }

  if (auto keyType = std::dynamic_pointer_cast<CacheableKey>(object)) {
    if (auto otherKeyType =
            std::dynamic_pointer_cast<CacheableKey>(otherObject)) {
      return *keyType == *otherKeyType;
    }
  }

  throw IllegalStateException(
      "PdxInstance cannot calculate equals of the field " + object->toString() +
      " since equals is only supported for CacheableKey derived types.");
}

}  // namespace client
}  // namespace geode
}  // namespace apache
