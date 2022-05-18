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
    : PdxInstanceImpl(std::move(fields), std::move(buffer), std::move(pdxType),
                      cache) {}

WritablePdxInstanceImpl::~WritablePdxInstanceImpl() noexcept = default;

size_t WritablePdxInstanceImpl::objectSize() const {
  auto size = sizeof(WritablePdxInstanceImpl);
  size += buffer_.size();
  size += pdxType_->objectSize();
  for (const auto& field : fields_) {
    size += field->objectSize();
  }

  return size;
}

void WritablePdxInstanceImpl::setField(const std::string& name, bool value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::BOOLEAN) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int8_t value) {
  auto field = pdxType_->getField(name);

  if (!field || field->getType() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, uint8_t value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::BYTE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int16_t value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::SHORT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int32_t value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::INT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, int64_t value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::LONG) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, float value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::FLOAT) {
    throw IllegalStateException(
        "PdxInstance doesn't have field " + name +
        " or type of field not matched " +
        (field != nullptr ? field->toString().c_str() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name, double value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::DOUBLE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       char16_t value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::CHAR) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       std::shared_ptr<CacheableDate> value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::DATE) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, value);
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       std::shared_ptr<Cacheable> value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::OBJECT) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, value);
}

void WritablePdxInstanceImpl::setField(
    const std::string& name, std::shared_ptr<CacheableObjectArray> value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::OBJECT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, value);
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<bool>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::BOOLEAN_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int8_t>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::BYTE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int16_t>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::SHORT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int32_t>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::INT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<int64_t>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::LONG_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<float>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::FLOAT_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<double>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::DOUBLE_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::vector<char16_t>& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::CHAR_ARRAY) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       const std::string& value) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::STRING) {
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
  if (!field || field->getType() != PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS) {
    throw IllegalStateException("PdxInstance doesn't have field " + name +
                                " or type of field not matched " +
                                (field != nullptr ? field->toString() : ""));
  }

  updateFieldValue(field, toCacheableField(value, arrayLength, elementLength));
}

void WritablePdxInstanceImpl::setField(const std::string& name,
                                       std::string* value, int32_t length) {
  auto field = pdxType_->getField(name);
  if (!field || field->getType() != PdxFieldTypes::STRING_ARRAY) {
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
