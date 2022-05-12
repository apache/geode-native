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

#include "PdxReaderImpl.hpp"

#include <geode/Cache.hpp>
#include <geode/TypeRegistry.hpp>

#include "PdxFieldType.hpp"
#include "PdxHelper.hpp"
#include "PdxType.hpp"

namespace apache {
namespace geode {
namespace client {

PdxReaderImpl::PdxReaderImpl()
    : dataInput_{nullptr},
      startPosition_{0},
      length_{0},
      lengthWithOffsets_{0},
      offsetSize_{0},
      offsets_{nullptr} {}

PdxReaderImpl::PdxReaderImpl(DataInput& input,
                             std::shared_ptr<PdxType> remoteType,
                             size_t length)
    : pdxType_{remoteType}, dataInput_{&input}, lengthWithOffsets_{length} {
  initialize();
}

PdxReaderImpl::~PdxReaderImpl() = default;

void PdxReaderImpl::initialize() {
  startPosition_ = static_cast<int32_t>(dataInput_->getBytesRead());
  offsetSize_ = PdxHelper::getOffsetSize(lengthWithOffsets_);

  length_ = lengthWithOffsets_ - pdxType_->getOffsetsCount() * offsetSize_;
  offsets_ =
      const_cast<uint8_t*>(dataInput_->currentBufferPosition()) + length_;
}

void PdxReaderImpl::moveInputToField(std::shared_ptr<PdxFieldType> field) const {
  auto position =
      pdxType_->getFieldPosition(field, offsets_, offsetSize_, length_);
  dataInput_->reset(startPosition_ + position);
}

void PdxReaderImpl::moveInputToEnd() {
  dataInput_->reset(startPosition_ + lengthWithOffsets_);
}

char16_t PdxReaderImpl::readChar(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readInt16();
}

char16_t PdxReaderImpl::readChar(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return '\0';
  }

  return readChar(field);
}

bool PdxReaderImpl::readBoolean(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readBoolean();
}

bool PdxReaderImpl::readBoolean(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return false;
  }

  return readBoolean(field);
}

int8_t PdxReaderImpl::readByte(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->read();
}

int8_t PdxReaderImpl::readByte(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  return readByte(field);
}

int16_t PdxReaderImpl::readShort(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readInt16();
}

int16_t PdxReaderImpl::readShort(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  return readShort(field);
}

int32_t PdxReaderImpl::readInt(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readInt32();
}

int32_t PdxReaderImpl::readInt(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  return readInt(field);
}

int64_t PdxReaderImpl::readLong(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readInt64();
}

int64_t PdxReaderImpl::readLong(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  return readLong(field);
}

float PdxReaderImpl::readFloat(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readFloat();
}

float PdxReaderImpl::readFloat(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0.f;
  }

  return readFloat(field);
}

double PdxReaderImpl::readDouble(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readDouble();
}

double PdxReaderImpl::readDouble(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0.;
  }

  return readDouble(field);
}

std::shared_ptr<CacheableDate> PdxReaderImpl::readDate(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  auto result = CacheableDate::create();

  result->fromData(*dataInput_);
  return result;
}

std::shared_ptr<CacheableDate> PdxReaderImpl::readDate(
    const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::shared_ptr<CacheableDate>{};
  }

  return readDate(field);
}

std::string PdxReaderImpl::readString(std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readString();
}

std::string PdxReaderImpl::readString(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::string{};
  }

  return readString(field);
}

std::shared_ptr<Serializable> PdxReaderImpl::readObject(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readObject();
}

std::shared_ptr<Serializable> PdxReaderImpl::readObject(
    const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::shared_ptr<Serializable>{};
  }

  return readObject(field);
}

std::vector<char16_t> PdxReaderImpl::readCharArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readCharArray();
}

std::vector<char16_t> PdxReaderImpl::readCharArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<char16_t>{};
  }

  return readCharArray(field);
}

std::vector<bool> PdxReaderImpl::readBooleanArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readBooleanArray();
}

std::vector<bool> PdxReaderImpl::readBooleanArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<bool>{};
  }

  return readBooleanArray(field);
}

std::vector<int8_t> PdxReaderImpl::readByteArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readByteArray();
}

std::vector<int8_t> PdxReaderImpl::readByteArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int8_t>{};
  }

  return readByteArray(field);
}

std::vector<int16_t> PdxReaderImpl::readShortArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readShortArray();
}

std::vector<int16_t> PdxReaderImpl::readShortArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int16_t>{};
  }

  return readShortArray(field);
}

std::vector<int32_t> PdxReaderImpl::readIntArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readIntArray();
}

std::vector<int32_t> PdxReaderImpl::readIntArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int32_t>{};
  }

  return readIntArray(field);
}

std::vector<int64_t> PdxReaderImpl::readLongArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readLongArray();
}

std::vector<int64_t> PdxReaderImpl::readLongArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int64_t>{};
  }

  return readLongArray(field);
}

std::vector<float> PdxReaderImpl::readFloatArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readFloatArray();
}

std::vector<float> PdxReaderImpl::readFloatArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<float>{};
  }

  return readFloatArray(field);
}

std::vector<double> PdxReaderImpl::readDoubleArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readDoubleArray();
}

std::vector<double> PdxReaderImpl::readDoubleArray(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<double>{};
  }

  return readDoubleArray(field);
}

std::vector<std::string> PdxReaderImpl::readStringArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  return dataInput_->readStringArray();
}

std::vector<std::string> PdxReaderImpl::readStringArray(
    const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<std::string>{};
  }

  return readStringArray(field);
}

std::shared_ptr<CacheableObjectArray> PdxReaderImpl::readObjectArray(
    std::shared_ptr<PdxFieldType> field) const {
  moveInputToField(field);
  auto result = CacheableObjectArray::create();

  result->fromData(*dataInput_);
  return result;
}

std::shared_ptr<CacheableObjectArray> PdxReaderImpl::readObjectArray(
    const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::shared_ptr<CacheableObjectArray>{};
  }

  return readObjectArray(field);
}

int8_t** PdxReaderImpl::readArrayOfByteArrays(
    std::shared_ptr<PdxFieldType> field, int32_t& arrayLength,
    int32_t** elementLength) const {
  moveInputToField(field);
  int8_t** result = nullptr;

  dataInput_->readArrayOfByteArrays(&result, arrayLength, elementLength);
  return result;
}

int8_t** PdxReaderImpl::readArrayOfByteArrays(const std::string& name,
                                              int32_t& arrayLength,
                                              int32_t** elementLength) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return nullptr;
  }

  return readArrayOfByteArrays(field, arrayLength, elementLength);
}

bool PdxReaderImpl::hasField(const std::string& fieldName) {
  return pdxType_->getField(fieldName) != nullptr;
}

bool PdxReaderImpl::isIdentityField(const std::string& fieldName) {
  auto field = pdxType_->getField(fieldName);
  return field && field->isIdentity();
}

std::shared_ptr<PdxSerializer> PdxReaderImpl::getPdxSerializer() const {
  return dataInput_->getCache()->getTypeRegistry().getPdxSerializer();
}

std::vector<int8_t> PdxReaderImpl::getRawFieldData(int32_t idx) const {
  int32_t end;
  std::vector<int8_t> result;

  auto field = pdxType_->getField(idx++);
  auto start =
      pdxType_->getFieldPosition(field, offsets_, offsetSize_, length_);

  if (idx < pdxType_->getFieldsCount()) {
    field = pdxType_->getField(idx);
    end = pdxType_->getFieldPosition(field, offsets_, offsetSize_, length_);
  } else {
    end = length_;
  }

  auto len = end - start;

  result.resize(len);
  dataInput_->reset(startPosition_ + start);
  dataInput_->readBytesOnly(result.data(), len);

  return result;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
