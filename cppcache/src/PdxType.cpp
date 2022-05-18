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
/*
 * PdxType.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxType.hpp"

#include "PdxFieldType.hpp"
#include "PdxHelper.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

PdxType::PdxType(const std::string& className, bool expectDomainClass)
    : Serializable{},
      initialized_{false},
      className_{className},
      isDomainClass_{expectDomainClass},
      typeId_{0},
      lastVarFieldId_{-1} {}

PdxType::~PdxType() noexcept = default;

size_t PdxType::objectSize() const {
  auto size = sizeof(PdxType);
  for (const auto& field : fields_) {
    size += field->objectSize();
  }

  size += static_cast<uint32_t>(className_.length());
  for (auto&& iter : fieldMap_) {
    size += iter.first.length();
    size += iter.second->objectSize();
  }

  return size;
}

void PdxType::toData(DataOutput& output) const {
  output.write(static_cast<int8_t>(DSCode::DataSerializable));  // 45
  output.write(static_cast<int8_t>(DSCode::Class));             // 43
  output.writeString("org.apache.geode.pdx.internal.PdxType");

  // Class name
  output.writeString(className_);

  // Flags
  output.writeBoolean(!isDomainClass_);

  // Type ID
  output.writeInt(typeId_);

  // Offsets count
  output.writeInt(getOffsetsCount());

  // Fields
  output.writeArrayLen(static_cast<int32_t>(fields_.size()));
  for (const auto& field : fields_) {
    field->toData(output);
  }
}

void PdxType::fromData(DataInput& input) {
  input.read();        // ignore dsByte
  input.read();        // ignore classByte
  input.readString();  // ignore classtypeId

  className_ = input.readString();
  isDomainClass_ = !input.readBoolean();
  typeId_ = input.readInt32();
  lastVarFieldId_ = input.readInt32();

  auto len = input.readArrayLength();

  fields_.reserve(len);
  for (int32_t i = 0; i < len; ++i) {
    auto field = std::make_shared<PdxFieldType>();
    field->fromData(input);

    fieldMap_[field->getName()] = field;
    fields_.emplace_back(std::move(field));
  }

  initialize();
}

std::shared_ptr<PdxFieldType> PdxType::addField(const std::string& fieldName,
                                                PdxFieldTypes typeId) {
  if (fieldMap_.find(fieldName) != fieldMap_.end()) {
    throw IllegalStateException("Field: " + fieldName +
                                " is already added to PdxWriter");
  }

  int32_t varFieldIdx = typeId >= PdxFieldTypes::STRING ? ++lastVarFieldId_ : 0;

  auto field = std::make_shared<PdxFieldType>(
      fieldName, typeId, static_cast<int32_t>(fields_.size()), varFieldIdx);

  fields_.push_back(field);
  fieldMap_[fieldName] = field;

  return field;
}

std::vector<std::shared_ptr<PdxFieldType>> PdxType::getIdentityFields() const {
  std::vector<std::shared_ptr<PdxFieldType>> result;

  for (const auto& entry : fieldMap_) {
    const auto& field = entry.second;
    if (field->isIdentity()) {
      result.push_back(field);
    }
  }

  // In order to be aligned with Java code, if there is no field marked as
  // identity, all fields are considered identity
  if (result.empty()) {
    result.reserve(fieldMap_.size());
    for (const auto& entry : fieldMap_) {
      result.push_back(entry.second);
    }
  }

  return result;
}

void PdxType::initialize() {
  if (initialized_) {
    return;
  }

  bool foundVarLen = false;
  auto lastVarFieldSeqId = 0;
  std::shared_ptr<PdxFieldType> prev;

  for (auto iter = fields_.rbegin(), end = fields_.rend(); iter != end;) {
    auto field = *iter++;
    if (field->isVariable()) {
      field->setVarOffsetId(field->getVarId());
      field->setRelativeOffset(0);

      foundVarLen = true;
      lastVarFieldSeqId = field->getVarId();
    } else {
      if (foundVarLen) {
        field->setVarOffsetId(lastVarFieldSeqId);
        // relative offset is subtracted from var len offsets
        field->setRelativeOffset(prev->getRelativeOffset() -
                                 field->getFixedSize());
      } else {
        field->setVarOffsetId(-1);  // Pdx header length
        // relative offset is subtracted from var len offsets
        field->setRelativeOffset(-field->getFixedSize());
        if (prev != nullptr) {  // boundary condition
          field->setRelativeOffset(prev->getRelativeOffset() -
                                   field->getFixedSize());
        }
      }
    }

    prev = field;
  }

  auto prevFixedSizeOffsets = 0;
  // now do optimization till you don't fine var len
  for (const auto& field : fields_) {
    if (field->isVariable()) {
      field->setVarOffsetId(-1);  // first var len field
      field->setRelativeOffset(prevFixedSizeOffsets);
      break;
    } else {
      field->setVarOffsetId(0);  // no need to read offset
      field->setRelativeOffset(prevFixedSizeOffsets);
      prevFixedSizeOffsets += field->getFixedSize();
    }
  }

  initialized_ = true;
}

int32_t PdxType::getOffsetsCount() const {
  return lastVarFieldId_ == -1 ? 0 : lastVarFieldId_;
}

std::shared_ptr<PdxFieldType> PdxType::getField(const std::string& name) const {
  auto&& iter = fieldMap_.find(name);
  return iter != fieldMap_.end() ? iter->second : nullptr;
}

std::shared_ptr<PdxFieldType> PdxType::getField(int32_t idx) const {
  return fields_.at(idx);
}

int32_t PdxType::getFieldPosition(std::shared_ptr<PdxFieldType> field,
                                  uint8_t* offsets, int32_t offsetSize,
                                  int32_t length) {
  if (field->isVariable()) {
    return getVarFieldPos(field, offsets, offsetSize);
  } else {
    return getFixedFieldPos(field, offsets, offsetSize, length);
  }
}

int32_t PdxType::getFixedFieldPos(std::shared_ptr<PdxFieldType> field,
                                  uint8_t* offsets, int32_t offsetSize,
                                  int32_t length) {
  int32_t offset = field->getVarLenOffsetIndex();
  if (field->getRelativeOffset() >= 0) {
    // starting fields
    return field->getRelativeOffset();
  } else if (offset == PdxFieldType::npos) {  // Pdx length
    // there is no var len field so just subtracts relative offset from behind
    return length + field->getRelativeOffset();
  } else {
    // need to read offset and then subtract relative offset
    // TODO
    return PdxHelper::readInt(offsets + (lastVarFieldId_ - offset) * offsetSize,
                              offsetSize) +
           field->getRelativeOffset();
  }
}

int32_t PdxType::getVarFieldPos(std::shared_ptr<PdxFieldType> field,
                                uint8_t* offsets, int32_t offsetSize) {
  int32_t offset = field->getVarLenOffsetIndex();
  if (offset == -1) {
    return /*first var len field*/ field->getRelativeOffset();
  } else {
    // we write offset from behind
    return PdxHelper::readInt(offsets + (lastVarFieldId_ - offset) * offsetSize,
                              offsetSize);
  }
}

bool PdxType::operator==(const PdxType& other) const {
  if (this->isDomainClass_ != other.isDomainClass_ ||
      getFieldsCount() != other.getFieldsCount() ||
      this->className_ != other.className_) {
    return false;
  }

  for (auto iter = fields_.begin(), end = fields_.end(),
            otherIter = other.fields_.begin();
       iter != end;) {
    if (**iter++ != **otherIter++) {
      return false;
    }
  }

  return true;
}

std::shared_ptr<Serializable> PdxType::createDeserializable() {
  return std::make_shared<PdxType>(std::string{});
}

}  // namespace client
}  // namespace geode
}  // namespace apache
