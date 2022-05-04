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
 * PdxFieldType.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxFieldType.hpp"

#include <geode/PdxFieldTypes.hpp>

namespace {
using apache::geode::client::PdxFieldTypes;

static const int32_t kFieldTypeSizes[] = {
    sizeof(bool),      // BOOLEAN
    sizeof(int8_t),    // BYTE
    sizeof(char16_t),  // CHAR
    sizeof(int16_t),   // SHORT
    sizeof(int32_t),   // INT
    sizeof(int64_t),   // LONG
    sizeof(float),     // FLOAT
    sizeof(double),    // DOUBLE
    sizeof(int64_t),   // DATE
    -1,                // STRING
    -1,                // OBJECT
    -1,                // BOOLEAN_ARRAY
    -1,                // CHAR_ARRAY
    -1,                // BYTE_ARRAY
    -1,                // SHORT_ARRAY
    -1,                // INT_ARRAY
    -1,                // LONG_ARRAY
    -1,                // FLOAT_ARRAY
    -1,                // DOUBLE_ARRAY
    -1,                // STRING_ARRAY
    -1,                // OBJECT_ARRAY
    -1,                // ARRAY_OF_BYTE_ARRAYS
};

bool isFieldVariableType(PdxFieldTypes type) {
  return type > PdxFieldTypes::DATE;
}

int32_t getFieldSize(PdxFieldTypes type) {
  if (type == PdxFieldTypes::UNKNOWN) {
    return sizeof(uint8_t);
  }

  return kFieldTypeSizes[static_cast<int32_t>(type)];
}

}  // namespace

namespace apache {
namespace geode {
namespace client {

PdxFieldType::PdxFieldType()
    : Serializable{},
      name_{},
      type_{PdxFieldTypes::UNKNOWN},
      isVariable_{false},
      isIdentity_{false},
      fixedSize_{0},
      relOffset_{0},
      sequenceId_{0},
      varId_{0},
      varOffsetId_{0} {}

PdxFieldType::PdxFieldType(std::string name, PdxFieldTypes type,
                           int32_t sequenceId, int32_t varId)
    : Serializable{},
      name_{name},
      type_{type},
      isVariable_{isFieldVariableType(type)},
      isIdentity_{false},
      fixedSize_{getFieldSize(type)},
      relOffset_(0),
      sequenceId_{sequenceId},
      varId_{varId},
      varOffsetId_{0} {}

void PdxFieldType::toData(DataOutput& output) const {
  output.writeString(name_);
  output.writeInt(sequenceId_);
  output.writeInt(varId_);
  output.write(static_cast<int8_t>(type_));

  output.writeInt(relOffset_);
  output.writeInt(varOffsetId_);
  output.writeBoolean(isIdentity_);
}

void PdxFieldType::fromData(DataInput& input) {
  name_ = input.readString();
  sequenceId_ = input.readInt32();
  varId_ = input.readInt32();
  type_ = static_cast<PdxFieldTypes>(input.read());
  if (type_ < PdxFieldTypes::BOOLEAN ||
      type_ > PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS) {
    throw IllegalStateException("Invalid type while deserializing PdxType");
  }

  relOffset_ = input.readInt32();
  varOffsetId_ = input.readInt32();
  isIdentity_ = input.readBoolean();

  isVariable_ = isFieldVariableType(type_);
  fixedSize_ = getFieldSize(type_);
}

size_t PdxFieldType::objectSize() const {
  {
    auto size = sizeof(PdxFieldType);
    size += name_.length();
    return size;
  }
}

bool PdxFieldType::equals(std::shared_ptr<PdxFieldType> other) {
  return other && (this == other.get() ||
                   (type_ == other->type_ && name_ == other->name_));
}

std::string PdxFieldType::toString() const {
  return std::string("PdxFieldName=") + name_ +
         " TypeId=" + std::to_string(static_cast<int>(type_)) +
         " VarLenFieldIdx=" + std::to_string(varId_) +
         " sequenceId=" + std::to_string(sequenceId_);
}

bool PdxFieldType::operator==(const PdxFieldType& other) const {
  return type_ == other.type_ && name_ == other.name_;
}

bool PdxFieldType::operator!=(const PdxFieldType& other) const {
  return !(*this == other);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
