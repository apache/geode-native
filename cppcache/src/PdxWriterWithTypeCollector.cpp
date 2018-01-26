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
 * PdxWriterWithTypeCollector.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxWriterWithTypeCollector.hpp"
#include "PdxType.hpp"
#include "PdxHelper.hpp"
#include "PdxTypes.hpp"
#include <geode/PdxFieldTypes.hpp>

namespace apache {
namespace geode {
namespace client {

PdxWriterWithTypeCollector::PdxWriterWithTypeCollector(
    DataOutput& output, std::string domainClassName,
    std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : PdxLocalWriter(output, nullptr, pdxTypeRegistry) {
  m_domainClassName = domainClassName;
  initialize();
}

void PdxWriterWithTypeCollector::initialize() {
  m_pdxType = std::make_shared<PdxType>(m_pdxTypeRegistry, m_domainClassName, true);
}

PdxWriterWithTypeCollector::~PdxWriterWithTypeCollector() {}

void PdxWriterWithTypeCollector::endObjectWriting() {
  // Write header for PDX
  PdxLocalWriter::writePdxHeader();
}

void PdxWriterWithTypeCollector::writeOffsets(int32_t len) {
  if (len <= 0xff) {
    for (int i = static_cast<int>(m_offsets.size()) - 1; i > 0; i--) {
      m_dataOutput->write(static_cast<uint8_t>(m_offsets[i]));
    }
  } else if (len <= 0xffff) {
    for (int i = static_cast<int>(m_offsets.size()) - 1; i > 0; i--) {
      m_dataOutput->writeInt(static_cast<uint16_t>(m_offsets[i]));
    }
  } else {
    for (int i = static_cast<int>(m_offsets.size()) - 1; i > 0; i--) {
      m_dataOutput->writeInt(static_cast<uint32_t>(m_offsets[i]));
    }
  }
}

int32_t PdxWriterWithTypeCollector::calculateLenWithOffsets() {
  int bufferLen = m_dataOutput->getBufferLength() - m_startPositionOffset;
  int32_t totalOffsets = 0;
  if (m_offsets.size() > 0) {
    totalOffsets = static_cast<int32_t>(m_offsets.size()) -
                   1;  // for first var len no need to append offset
  }
  int32_t totalLen = bufferLen - PdxHelper::PdxHeader + totalOffsets;

  if (totalLen <= 0xff) {
    return totalLen;
  } else if (totalLen + totalOffsets <= 0xffff) {
    return totalLen + totalOffsets;
  } else {
    return totalLen + totalOffsets * 3;
  }
}

void PdxWriterWithTypeCollector::addOffset() {
  int bufferLen = m_dataOutput->getBufferLength() - m_startPositionOffset;
  int offset = bufferLen - PdxHelper::PdxHeader;

  m_offsets.push_back(offset);
}

bool PdxWriterWithTypeCollector::isFieldWritingStarted() {
  return m_pdxType->getTotalFields() > 0;
}
PdxWriter& PdxWriterWithTypeCollector::writeUnreadFields(
    std::shared_ptr<PdxUnreadFields> unread) {
  PdxLocalWriter::writeUnreadFields(unread);
  return *this;
}

PdxWriter& PdxWriterWithTypeCollector::writeChar(
    const std::string& fieldName, char16_t value) {
  m_pdxType->addFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,
                                     PdxTypes::CHAR_SIZE);
  PdxLocalWriter::writeChar(fieldName, value);
  return *this;
 }
 PdxWriter& PdxWriterWithTypeCollector::writeBoolean(
     const std::string& fieldName, bool value) {
   m_pdxType->addFixedLengthTypeField(
       fieldName, "boolean", PdxFieldTypes::BOOLEAN, PdxTypes::BOOLEAN_SIZE);
   PdxLocalWriter::writeBoolean(fieldName, value);
   return *this;
 }
 PdxWriter& PdxWriterWithTypeCollector::writeByte(const std::string& fieldName,
                                                  int8_t value) {
   m_pdxType->addFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE,
                                      PdxTypes::BYTE_SIZE);
   PdxLocalWriter::writeByte(fieldName, value);
   return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeShort(const std::string& fieldName,
                                                  int16_t value) {
  m_pdxType->addFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT,
                                     PdxTypes::SHORT_SIZE);
  PdxLocalWriter::writeShort(fieldName, value);
  return *this;
 }
 PdxWriter& PdxWriterWithTypeCollector::writeInt(const std::string& fieldName,
                                                 int32_t value) {
   m_pdxType->addFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT,
                                      PdxTypes::INTEGER_SIZE);
   PdxLocalWriter::writeInt(fieldName, value);
   return *this;
 }
 PdxWriter& PdxWriterWithTypeCollector::writeLong(const std::string& fieldName,
                                                  int64_t value) {
   m_pdxType->addFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG,
                                      PdxTypes::LONG_SIZE);
   PdxLocalWriter::writeLong(fieldName, value);
   return *this;
 }
 PdxWriter& PdxWriterWithTypeCollector::writeFloat(const std::string& fieldName,
                                                   float value) {
   m_pdxType->addFixedLengthTypeField(fieldName, "float", PdxFieldTypes::FLOAT,
                                      PdxTypes::FLOAT_SIZE);
   PdxLocalWriter::writeFloat(fieldName, value);
   return *this;
 }
 PdxWriter& PdxWriterWithTypeCollector::writeDouble(
     const std::string& fieldName, double value) {
   m_pdxType->addFixedLengthTypeField(
       fieldName, "double", PdxFieldTypes::DOUBLE, PdxTypes::DOUBLE_SIZE);
   PdxLocalWriter::writeDouble(fieldName, value);
   return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeDate(
    const std::string& fieldName, std::shared_ptr<CacheableDate> date) {
  m_pdxType->addFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE,
                                     PdxTypes::DATE_SIZE);
  PdxLocalWriter::writeDate(fieldName, date);
  return *this;
}

PdxWriter& PdxWriterWithTypeCollector::writeString(
    const std::string& fieldName, const std::string& value) {
  m_pdxType->addVariableLengthTypeField(fieldName, "String",
                                        PdxFieldTypes::STRING);
  PdxLocalWriter::writeString(fieldName, value);
  return *this;
 }

PdxWriter& PdxWriterWithTypeCollector::writeObject(
    const std::string& fieldName, std::shared_ptr<Serializable> value) {
  m_pdxType->addVariableLengthTypeField(fieldName, "Serializable",
                                        PdxFieldTypes::OBJECT);
  PdxLocalWriter::writeObject(fieldName, value);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeBooleanArray(
    const std::string& fieldName, const std::vector<bool>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "bool[]",
                                        PdxFieldTypes::BOOLEAN_ARRAY);
  PdxLocalWriter::writeBooleanArray(fieldName, array);
  return *this;
}

PdxWriter& PdxWriterWithTypeCollector::writeCharArray(
    const std::string& fieldName, const std::vector<char16_t>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "char[]",
                                        PdxFieldTypes::CHAR_ARRAY);
  PdxLocalWriter::writeCharArray(fieldName, array);
  return *this;
}

PdxWriter& PdxWriterWithTypeCollector::writeByteArray(
    const std::string& fieldName, const std::vector<int8_t>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "byte[]",
                                        PdxFieldTypes::BYTE_ARRAY);
  PdxLocalWriter::writeByteArray(fieldName, array);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeShortArray(
    const std::string& fieldName, const std::vector<int16_t>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "short[]",
                                        PdxFieldTypes::SHORT_ARRAY);
  PdxLocalWriter::writeShortArray(fieldName, array);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeIntArray(
    const std::string& fieldName, const std::vector<int32_t>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "int[]",
                                        PdxFieldTypes::INT_ARRAY);
  PdxLocalWriter::writeIntArray(fieldName, array);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeLongArray(
    const std::string& fieldName, const std::vector<int64_t>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "long[]",
                                        PdxFieldTypes::LONG_ARRAY);
  PdxLocalWriter::writeLongArray(fieldName, array);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeFloatArray(
    const std::string& fieldName, const std::vector<float>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "float[]",
                                        PdxFieldTypes::FLOAT_ARRAY);
  PdxLocalWriter::writeFloatArray(fieldName, array);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeDoubleArray(
    const std::string& fieldName, const std::vector<double>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "double[]",
                                        PdxFieldTypes::DOUBLE_ARRAY);
  PdxLocalWriter::writeDoubleArray(fieldName, array);
  return *this;
}

PdxWriter& PdxWriterWithTypeCollector::writeStringArray(
    const std::string& fieldName, const std::vector<std::string>& array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "String[]",
                                        PdxFieldTypes::STRING_ARRAY);
  PdxLocalWriter::writeStringArray(fieldName, array);
  return *this;
}

PdxWriter& PdxWriterWithTypeCollector::writeObjectArray(
    const std::string& fieldName, std::shared_ptr<CacheableObjectArray> array) {
  m_pdxType->addVariableLengthTypeField(fieldName, "Object[]",
                                        PdxFieldTypes::OBJECT_ARRAY);
  PdxLocalWriter::writeObjectArray(fieldName, array);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::writeArrayOfByteArrays(
    const std::string& fieldName, int8_t* const* const byteArrays,
    int arrayLength, const int* elementLength) {
  m_pdxType->addVariableLengthTypeField(fieldName, "byte[][]",
                                        PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  PdxLocalWriter::writeArrayOfByteArrays(fieldName, byteArrays, arrayLength,
                                         elementLength);
  return *this;
}
PdxWriter& PdxWriterWithTypeCollector::markIdentityField(
    const std::string& fieldName) {
  auto pft = m_pdxType->getPdxField(fieldName);
  if (pft == nullptr) {
    throw IllegalStateException(
        "Field, must be written to PdxWriter before calling "
        "MarkIdentityField ");
  }
  pft->setIdentityField(true);
  return *this;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
