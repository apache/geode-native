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

#include "PdxTypes.hpp"

namespace apache {
namespace geode {
namespace client {

static const int32_t kFixedTypeSizes[] = {
    PdxTypes::PDX_BOOLEAN_SIZE,  // BOOLEAN
    PdxTypes::PDX_BOOLEAN_SIZE,  // BYTE
    PdxTypes::PDX_CHAR_SIZE,     // CHAR
    PdxTypes::PDX_CHAR_SIZE,     // SHORT
    PdxTypes::PDX_INTEGER_SIZE,  // INT
    PdxTypes::PDX_LONG_SIZE,     // LONG
    PdxTypes::PDX_INTEGER_SIZE,  // FLOAT
    PdxTypes::PDX_LONG_SIZE,     // DOUBLE
    PdxTypes::PDX_DATE_SIZE,     // DATE
    -1,                          // STRING
    -1,                          // OBJECT
    -1,                          // BOOLEAN_ARRAY
    -1,                          // CHAR_ARRAY
    -1,                          // BYTE_ARRAY
    -1,                          // SHORT_ARRAY
    -1,                          // INT_ARRAY
    -1,                          // LONG_ARRAY
    -1,                          // FLOAT_ARRAY
    -1,                          // DOUBLE_ARRAY
    -1,                          // STRING_ARRAY
    -1,                          // OBJECT_ARRAY
    -1,                          // ARRAY_OF_BYTE_ARRAYS
};

PdxFieldType::PdxFieldType()
    : Serializable(),
      m_fieldName(),
      m_className(),
      m_typeId(PdxFieldTypes::UNKNOWN),
      m_sequenceId(0),
      m_isVariableLengthType(false),
      m_isIdentityField(false),
      m_fixedSize(0),
      m_varLenFieldIdx(0),
      m_vlOffsetIndex(0),
      m_relativeOffset(0) {}

PdxFieldType::PdxFieldType(std::string fieldName, std::string className,
                           PdxFieldTypes typeId, int32_t sequenceId,
                           bool isVariableLengthType, int32_t fixedSize,
                           int32_t varLenFieldIdx)
    : Serializable(),
      m_fieldName(fieldName),
      m_className(className),
      m_typeId(typeId),
      m_sequenceId(sequenceId),
      m_isVariableLengthType(isVariableLengthType),
      m_isIdentityField(false),
      m_fixedSize(fixedSize),
      m_varLenFieldIdx(varLenFieldIdx),
      m_vlOffsetIndex(0),
      m_relativeOffset(0) {}

void PdxFieldType::toData(DataOutput& output) const {
  output.writeString(m_fieldName);
  output.writeInt(m_sequenceId);
  output.writeInt(m_varLenFieldIdx);
  output.write(static_cast<int8_t>(m_typeId));

  output.writeInt(m_relativeOffset);
  output.writeInt(m_vlOffsetIndex);
  output.writeBoolean(m_isIdentityField);
}

void PdxFieldType::fromData(DataInput& input) {
  m_fieldName = input.readString();
  m_sequenceId = input.readInt32();
  m_varLenFieldIdx = input.readInt32();
  m_typeId = PdxFieldTypes(input.read());
  m_relativeOffset = input.readInt32();
  m_vlOffsetIndex = input.readInt32();
  m_isIdentityField = input.readBoolean();
  m_fixedSize = getFixedTypeSize();
  if (m_fixedSize != -1) {
    m_isVariableLengthType = false;
  } else {
    m_isVariableLengthType = true;
  }
}

bool PdxFieldType::equals(std::shared_ptr<PdxFieldType> otherObj) {
  if (otherObj == nullptr) return false;

  PdxFieldType* otherFieldType = dynamic_cast<PdxFieldType*>(otherObj.get());

  if (otherFieldType == nullptr) return false;

  if (otherFieldType == this) return true;

  if (otherFieldType->m_fieldName.compare(m_fieldName) == 0 &&
      otherFieldType->m_typeId == m_typeId) {
    return true;
  }

  return false;
}

int32_t PdxFieldType::getFixedTypeSize() const {
  int32_t result = -1;

  if (m_typeId != PdxFieldTypes::UNKNOWN) {
    result = kFixedTypeSizes[static_cast<int32_t>(m_typeId)];
  }

  return result;
}

std::string PdxFieldType::toString() const {
  char stringBuf[1024];
  std::snprintf(stringBuf, 1024,
                " PdxFieldName=%s TypeId=%d VarLenFieldIdx=%d sequenceid=%d\n",
                this->m_fieldName.c_str(), static_cast<int>(this->m_typeId),
                this->m_varLenFieldIdx, this->m_sequenceId);
  return std::string(stringBuf);
}

bool PdxFieldType::operator==(const PdxFieldType& other) const {
  if (m_className != other.m_className) {
    return false;
  }

  if (m_fieldName != other.m_fieldName) {
    return false;
  }

  return true;
}

bool PdxFieldType::operator!=(const PdxFieldType& other) const {
  return !(*this == other);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
