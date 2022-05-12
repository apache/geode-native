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

#include "PdxWriterImpl.hpp"

#include <geode/CacheableEnum.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/TypeRegistry.hpp>

#include "PdxHelper.hpp"
#include "PdxType.hpp"
#include "PdxUnreadData.hpp"

namespace apache {
namespace geode {
namespace client {

PdxWriterImpl::PdxWriterImpl(DataOutput& output)
    : PdxWriterImpl{nullptr, output} {}

PdxWriterImpl::PdxWriterImpl(std::shared_ptr<PdxType> pdxType,
                             DataOutput& output)
    : dataOutput_{&output}, pdxType_{pdxType} {
  initialize();
}

void PdxWriterImpl::initialize() {
  dataOutput_->advanceCursor(PdxHelper::HeaderSize);
  startPosition_ = static_cast<int32_t>(dataOutput_->getBufferLength());
}

std::shared_ptr<PdxFieldType> PdxWriterImpl::addPdxField(
    const std::string& name, PdxFieldTypes type) {
  if (pdxType_) {
    return pdxType_->addField(name, type);
  }

  return std::shared_ptr<PdxFieldType>{};
}

void PdxWriterImpl::addOffset() {
  offsets_.push_back(
      static_cast<int32_t>(dataOutput_->getBufferLength() - startPosition_));
}

void PdxWriterImpl::setUnreadData(std::shared_ptr<PdxUnreadData> data) {
  unreadData_ = std::move(data);
}

void PdxWriterImpl::completeSerialization() {
  if (unreadData_) {
    unreadData_->write(*this);
  }

  int32_t len = getTotalLength();
  auto delta = dataOutput_->getBufferLength() - startPosition_;

  dataOutput_->rewindCursor(delta + PdxHelper::HeaderSize);
  dataOutput_->writeInt(len);  // PDX payload

  // Set back the pointer and also skip PdxType as will be written later
  dataOutput_->advanceCursor(delta + sizeof(uint32_t));

  writeOffsets(len);
}

void PdxWriterImpl::writeOffsets(int32_t len) {
  int32_t count = offsets_.size() - 1;
  if(count <= 0) {
    return;
  }

  if (len <= std::numeric_limits<uint8_t>::max()) {
    for (auto i = count; i > 0; ) {
      dataOutput_->write(static_cast<uint8_t>(offsets_[i--]));
    }
  } else if (len <= std::numeric_limits<uint16_t>::max()) {
    for (auto i = count; i > 0; ) {
      dataOutput_->writeInt(static_cast<uint16_t>(offsets_[i--]));
    }
  } else {
    for (auto i = count; i > 0; ) {
      dataOutput_->writeInt(static_cast<uint32_t>(offsets_[i--]));
    }
  }
}

std::vector<int8_t> PdxWriterImpl::getFieldsBuffer() const {
  auto len = dataOutput_->getBufferLength() - startPosition_;
  auto buffer = &dataOutput_->getBuffer()[startPosition_];

  return std::vector<int8_t>{buffer, buffer + len};
}

PdxWriter& PdxWriterImpl::writeUnreadFields(
    std::shared_ptr<PdxUnreadFields> unread) {
  if (hasWrittenFields()) {
    throw IllegalStateException(
        "WriteUnreadFields must be called before any other fields are "
        "written.");
  }

  if (!(unreadData_ = std::dynamic_pointer_cast<PdxUnreadData>(unread))) {
    throw IllegalStateException(
        "PdxLocalWriter::writeUnreadFields: preservedData should not be "
        "nullptr");
  }

  return *this;
}

std::shared_ptr<PdxSerializer> PdxWriterImpl::getPdxSerializer() const {
  return dataOutput_->getCache()->getTypeRegistry().getPdxSerializer();
}

int32_t PdxWriterImpl::getTotalLength() const {
  int32_t offsetsCount = std::max(static_cast<int32_t>(offsets_.size() - 1), 0);
  int32_t len = dataOutput_->getBufferLength() - startPosition_ + offsetsCount;

  if (len <= std::numeric_limits<uint8_t>::max()) {
    return len;
  } else if (len + offsetsCount <= std::numeric_limits<uint16_t>::max()) {
    return len + offsetsCount;
  }

  return len + offsetsCount * 3;
}

bool PdxWriterImpl::hasWrittenFields() const {
  return (dataOutput_->getBufferLength() - startPosition_) != 0;
}

PdxWriter& PdxWriterImpl::writeChar(const std::string& name, char16_t value) {
  addPdxField(name, PdxFieldTypes::CHAR);
  writeChar(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeBoolean(const std::string& name, bool value) {
  addPdxField(name, PdxFieldTypes::BOOLEAN);
  writeBoolean(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeByte(const std::string& name, int8_t value) {
  addPdxField(name, PdxFieldTypes::BYTE);
  writeByte(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeShort(const std::string& name, int16_t value) {
  addPdxField(name, PdxFieldTypes::SHORT);
  writeShort(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeInt(const std::string& name, int32_t value) {
  addPdxField(name, PdxFieldTypes::INT);
  writeInt(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeLong(const std::string& name, int64_t value) {
  addPdxField(name, PdxFieldTypes::LONG);
  writeLong(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeFloat(const std::string& name, float value) {
  addPdxField(name, PdxFieldTypes::FLOAT);
  writeFloat(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeDouble(const std::string& name, double value) {
  addPdxField(name, PdxFieldTypes::DOUBLE);
  writeDouble(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeDate(const std::string& name,
                                    std::shared_ptr<CacheableDate> date) {
  addPdxField(name, PdxFieldTypes::DATE);
  writeDate(date);
  return *this;
}

PdxWriter& PdxWriterImpl::writeString(const std::string& name,
                                      const std::string& value) {
  addPdxField(name, PdxFieldTypes::STRING);
  writeString(value);
  return *this;
}

PdxWriter& PdxWriterImpl::writeStringArray(
    const std::string& name, const std::vector<std::string>& array) {
  addPdxField(name, PdxFieldTypes::STRING_ARRAY);
  writeStringArray(array);
  return *this;
}

PdxWriter& PdxWriterImpl::writeObject(const std::string& name,
                                      std::shared_ptr<Serializable> value) {
  addPdxField(name, PdxFieldTypes::OBJECT);
  writeObject(value);
  return *this;
}
PdxWriter& PdxWriterImpl::writeBooleanArray(const std::string& name,
                                            const std::vector<bool>& array) {
  addPdxField(name, PdxFieldTypes::BOOLEAN_ARRAY);
  writeBooleanArray(array);
  return *this;
}

PdxWriter& PdxWriterImpl::writeCharArray(const std::string& name,
                                         const std::vector<char16_t>& array) {
  addPdxField(name, PdxFieldTypes::CHAR_ARRAY);
  writeCharArray(array);
  return *this;
}

PdxWriter& PdxWriterImpl::writeByteArray(const std::string& name,
                                         const std::vector<int8_t>& array) {
  addPdxField(name, PdxFieldTypes::BYTE_ARRAY);
  writeByteArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeShortArray(const std::string& name,
                                          const std::vector<int16_t>& array) {
  addPdxField(name, PdxFieldTypes::SHORT_ARRAY);
  writeShortArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeIntArray(const std::string& name,
                                        const std::vector<int32_t>& array) {
  addPdxField(name, PdxFieldTypes::INT_ARRAY);
  writeIntArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeLongArray(const std::string& name,
                                         const std::vector<int64_t>& array) {
  addPdxField(name, PdxFieldTypes::LONG_ARRAY);
  writeLongArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeFloatArray(const std::string& name,
                                          const std::vector<float>& array) {
  addPdxField(name, PdxFieldTypes::FLOAT_ARRAY);
  writeFloatArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeDoubleArray(const std::string& name,
                                           const std::vector<double>& array) {
  addPdxField(name, PdxFieldTypes::DOUBLE_ARRAY);
  writeDoubleArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeObjectArray(
    const std::string& name, std::shared_ptr<CacheableObjectArray> array) {
  addPdxField(name, PdxFieldTypes::OBJECT_ARRAY);
  writeObjectArray(array);
  return *this;
}
PdxWriter& PdxWriterImpl::writeArrayOfByteArrays(const std::string& name,
                                                 int8_t* const* const array,
                                                 int arrayLength,
                                                 const int* elementLength) {
  addPdxField(name, PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  writeArrayOfByteArrays(array, arrayLength, elementLength);
  return *this;
}

PdxWriter& PdxWriterImpl::markIdentityField(const std::string& name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    throw IllegalStateException(
        "Field must be written to PdxWriter before calling "
        "markIdentityField ");
  }

  field->setIdentity(true);
  return *this;
}

void PdxWriterImpl::writeChar(char16_t value) { dataOutput_->writeChar(value); }

void PdxWriterImpl::writeBoolean(bool value) {
  dataOutput_->writeBoolean(value);
}

void PdxWriterImpl::writeByte(int8_t value) { dataOutput_->write(value); }

void PdxWriterImpl::writeShort(int16_t value) { dataOutput_->writeInt(value); }

void PdxWriterImpl::writeInt(int32_t value) { dataOutput_->writeInt(value); }

void PdxWriterImpl::writeLong(int64_t value) { dataOutput_->writeInt(value); }

void PdxWriterImpl::writeFloat(float value) { dataOutput_->writeFloat(value); }

void PdxWriterImpl::writeDouble(double value) {
  dataOutput_->writeDouble(value);
}

void PdxWriterImpl::writeDate(std::shared_ptr<CacheableDate> date) {
  if (date != nullptr) {
    date->toData(*dataOutput_);
  } else {
    dataOutput_->writeInt(static_cast<uint64_t>(-1L));
  }
}

void PdxWriterImpl::writeString(const std::string& value) {
  addOffset();
  dataOutput_->writeString(value);
}

void PdxWriterImpl::writeObject(std::shared_ptr<Serializable> value) {
  addOffset();
  if (auto enumValPtr = std::dynamic_pointer_cast<CacheableEnum>(value)) {
    enumValPtr->toData(*dataOutput_);
  } else if (auto objectArray =
                 std::dynamic_pointer_cast<CacheableObjectArray>(value)) {
    dataOutput_->write(static_cast<int8_t>(objectArray->getDsCode()));
    objectArray->toData(*dataOutput_);
  } else {
    dataOutput_->writeObject(value);
  }
}

void PdxWriterImpl::writeBooleanArray(const std::vector<bool>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeCharArray(const std::vector<char16_t>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeByteArray(const std::vector<int8_t>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeShortArray(const std::vector<int16_t>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeIntArray(const std::vector<int32_t>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeLongArray(const std::vector<int64_t>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeFloatArray(const std::vector<float>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeDoubleArray(const std::vector<double>& array) {
  addOffset();
  writeArrayObject(array);
}

void PdxWriterImpl::writeStringArray(const std::vector<std::string>& array) {
  addOffset();
  dataOutput_->writeArrayLen(static_cast<int32_t>(array.size()));
  for (auto&& entry : array) {
    dataOutput_->writeString(entry);
  }
}

void PdxWriterImpl::writeObjectArray(
    std::shared_ptr<CacheableObjectArray> array) {
  addOffset();
  if (array) {
    array->toData(*dataOutput_);
  } else {
    dataOutput_->write(static_cast<int8_t>(-1));
  }
}

void PdxWriterImpl::writeArrayOfByteArrays(int8_t* const* const array,
                                           int arrayLength,
                                           const int* elementLength) {
  addOffset();
  if (!array) {
    arrayLength = -1;
  }

  dataOutput_->writeArrayLen(arrayLength);
  for (int i = 0; i < arrayLength; i++) {
    dataOutput_->writeBytes(array[i], elementLength[i]);
  }
}

void PdxWriterImpl::writeRawField(std::shared_ptr<PdxFieldType> field,
                                  const std::vector<int8_t>& data) {
  if (field->isVariable()) {
    addOffset();
  }

  auto newField = addPdxField(field->getName(), field->getType());
  if (field->isIdentity()) {
    newField->setIdentity(true);
  }

  dataOutput_->writeBytesOnly(data.data(), data.size());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
