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
 * PdxReaderWithTypeCollector.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxReaderWithTypeCollector.hpp"

#include <geode/PdxFieldTypes.hpp>

#include "CacheImpl.hpp"
#include "PdxTypes.hpp"

namespace apache {
namespace geode {
namespace client {

PdxReaderWithTypeCollector::PdxReaderWithTypeCollector(
    DataInput& dataInput, std::shared_ptr<PdxType> pdxType, int32_t pdxlen,
    std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : PdxLocalReader(dataInput, pdxType, pdxlen, pdxTypeRegistry) {
  m_newPdxType = std::make_shared<PdxType>(
      *m_pdxTypeRegistry, pdxType->getPdxClassName().c_str(), true);
}

PdxReaderWithTypeCollector::~PdxReaderWithTypeCollector() {}

void PdxReaderWithTypeCollector::checkType(const std::string& fieldName,
                                           PdxFieldTypes typeId,
                                           const std::string& fieldType) {
  auto pft = m_pdxType->getPdxField(fieldName);
  if (pft != nullptr) {
    if (typeId != pft->getTypeId()) {
      throw IllegalStateException("Expected " + fieldType +
                                  " fieldType field but found field of type " +
                                  pft->toString().c_str());
    }
  }
}

char16_t PdxReaderWithTypeCollector::readChar(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::CHAR, "char");
  m_newPdxType->addFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,
                                        PdxTypes::CHAR_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readChar()position = %d", position);
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    auto retVal = PdxLocalReader::readChar(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::CHAR_SIZE);
    return retVal;
  } else {
    return 0;
  }
}

bool PdxReaderWithTypeCollector::readBoolean(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::BOOLEAN, "boolean");
  m_newPdxType->addFixedLengthTypeField(
      fieldName, "boolean", PdxFieldTypes::BOOLEAN, PdxTypes::BOOLEAN_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readBoolean():position = %d",
            position);
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    bool retVal = PdxLocalReader::readBoolean(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::BOOLEAN_SIZE);
    return retVal;
  } else {
    return 0;
  }
}

int8_t PdxReaderWithTypeCollector::readByte(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::BYTE, "byte");
  m_newPdxType->addFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE,
                                        PdxTypes::BYTE_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readByte(): position = %d", position);
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    int8_t retVal;
    retVal = PdxLocalReader::readByte(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::BYTE_SIZE);
    return retVal;
  } else {
    return 0;
  }
}

int16_t PdxReaderWithTypeCollector::readShort(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::SHORT, "short");
  m_newPdxType->addFixedLengthTypeField(
      fieldName, "short", PdxFieldTypes::SHORT, PdxTypes::SHORT_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readShort(): position = %d", position);
  if (position != -1) {
    int16_t value;
    m_dataInput->advanceCursor(position);
    value = PdxLocalReader::readShort(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::SHORT_SIZE);
    return value;
  } else {
    return 0;
  }
}

int32_t PdxReaderWithTypeCollector::readInt(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::INT, "int");
  m_newPdxType->addFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT,
                                        PdxTypes::INTEGER_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readInt():position = %d", position);
  if (position != -1) {
    int32_t value;
    m_dataInput->advanceCursor(position);
    value = PdxLocalReader::readInt(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::INTEGER_SIZE);
    return value;
  } else {
    return 0;
  }
}

int64_t PdxReaderWithTypeCollector::readLong(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::LONG, "long");
  m_newPdxType->addFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG,
                                        PdxTypes::LONG_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readLong(): position = %d", position);
  if (position != -1) {
    int64_t value;
    m_dataInput->advanceCursor(position);
    value = PdxLocalReader::readLong(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::LONG_SIZE);
    return value;
  } else {
    return 0;
  }
}

float PdxReaderWithTypeCollector::readFloat(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::FLOAT, "float");
  m_newPdxType->addFixedLengthTypeField(
      fieldName, "float", PdxFieldTypes::FLOAT, PdxTypes::FLOAT_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readFloat():position = %d", position);
  if (position != -1) {
    float value;
    m_dataInput->advanceCursor(position);
    value = PdxLocalReader::readFloat(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::FLOAT_SIZE);
    return value;
  } else {
    return 0.0f;
  }
}

double PdxReaderWithTypeCollector::readDouble(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::DOUBLE, "double");
  m_newPdxType->addFixedLengthTypeField(
      fieldName, "double", PdxFieldTypes::DOUBLE, PdxTypes::DOUBLE_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readDouble():position = %d", position);
  if (position != -1) {
    double value;
    m_dataInput->advanceCursor(position);
    value = PdxLocalReader::readDouble(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::DOUBLE_SIZE);
    return value;
  } else {
    return 0.0;
  }
}

std::string PdxReaderWithTypeCollector::readString(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::STRING, "String");
  m_newPdxType->addVariableLengthTypeField(fieldName, "String",
                                           PdxFieldTypes::STRING);
  int32_t position = m_pdxType->getFieldPosition(
      fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readString():position = %d", position);

  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    auto str = PdxLocalReader::readString(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
    return str;
  } else {
    return std::string();
  }
}

std::shared_ptr<Serializable> PdxReaderWithTypeCollector::readObject(
    const std::string& fieldName) {
  // field is collected after reading
  checkType(fieldName, PdxFieldTypes::OBJECT, "Serializable");
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readObject():position = %d", position);
  if (position != -1) {
    std::shared_ptr<Serializable> ptr;
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    ptr = PdxLocalReader::readObject(fieldName);
    m_newPdxType->addVariableLengthTypeField(fieldName, "Serializable",
                                             PdxFieldTypes::OBJECT);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
    return ptr;
  } else {
    return nullptr;
  }
}

std::vector<char16_t> PdxReaderWithTypeCollector::readCharArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::CHAR_ARRAY, "char[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "char[]",
                                           PdxFieldTypes::CHAR_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readCharArray():position = %d",
            position);
  std::vector<char16_t> array;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    array = PdxLocalReader::readCharArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return array;
}

std::vector<bool> PdxReaderWithTypeCollector::readBooleanArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::BOOLEAN_ARRAY, "boolean[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "boolean[]",
                                           PdxFieldTypes::BOOLEAN_ARRAY);
  LOG_DEBUG("NIL:293: PdxReaderWithTypeCollector::readBooleanArray Test-1");
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG(
      "NIL:293: PdxReaderWithTypeCollector::readBooleanArray(): Position =%d ",
      position);

  std::vector<bool> array;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    array = PdxLocalReader::readBooleanArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return array;
}

std::vector<int8_t> PdxReaderWithTypeCollector::readByteArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::BYTE_ARRAY, "byte[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "byte[]",
                                           PdxFieldTypes::BYTE_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readByteArray(): position = %d",
            position);
  std::vector<int8_t> array;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    array = PdxLocalReader::readByteArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return array;
}

std::vector<int16_t> PdxReaderWithTypeCollector::readShortArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::SHORT_ARRAY, "short[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "short[]",
                                           PdxFieldTypes::SHORT_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readShortArray():position = %d",
            position);
  std::vector<int16_t> shortArrptr;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    shortArrptr = PdxLocalReader::readShortArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return shortArrptr;
}

std::vector<int32_t> PdxReaderWithTypeCollector::readIntArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::INT_ARRAY, "int[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "int[]",
                                           PdxFieldTypes::INT_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readIntArray():position = %d",
            position);
  std::vector<int32_t> intArrayptr;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    intArrayptr = PdxLocalReader::readIntArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return intArrayptr;
}

std::vector<int64_t> PdxReaderWithTypeCollector::readLongArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::LONG_ARRAY, "long[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "long[]",
                                           PdxFieldTypes::LONG_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readLongArray():position = %d",
            position);
  std::vector<int64_t> longArrptr;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    longArrptr = PdxLocalReader::readLongArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return longArrptr;
}

std::vector<float> PdxReaderWithTypeCollector::readFloatArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::FLOAT_ARRAY, "float[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "float[]",
                                           PdxFieldTypes::FLOAT_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readFloatArray(): position = %d",
            position);
  std::vector<float> floatArrptr;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    floatArrptr = PdxLocalReader::readFloatArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return floatArrptr;
}

std::vector<double> PdxReaderWithTypeCollector::readDoubleArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::DOUBLE_ARRAY, "double[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "double[]",
                                           PdxFieldTypes::DOUBLE_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readDoubleArray():position = %d",
            position);
  std::vector<double> doubleArrptr;
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    doubleArrptr = PdxLocalReader::readDoubleArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
  }
  return doubleArrptr;
}

std::vector<std::string> PdxReaderWithTypeCollector::readStringArray(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::STRING_ARRAY, "String[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "String[]",
                                           PdxFieldTypes::STRING_ARRAY);
  auto position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                              m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readStringArray(): position = %d",
            position);

  std::vector<std::string> value;

  if (position > 0) {
    m_dataInput->advanceCursor(position);
    auto startLoc = m_dataInput->currentBufferPosition();
    value = PdxLocalReader::readStringArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(static_cast<int32_t>(strSize + position));
  }

  return value;
}

std::shared_ptr<CacheableObjectArray>
PdxReaderWithTypeCollector::readObjectArray(const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::OBJECT_ARRAY, "Object[]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "Object[]",
                                           PdxFieldTypes::OBJECT_ARRAY);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readObjectArray():position = %d",
            position);
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    auto retVal = PdxLocalReader::readObjectArray(fieldName);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
    return retVal;
  } else {
    return nullptr;
  }
}

int8_t** PdxReaderWithTypeCollector::readArrayOfByteArrays(
    const std::string& fieldName, int32_t& arrayLength,
    int32_t** elementLength) {
  checkType(fieldName, PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS, "byte[][]");
  m_newPdxType->addVariableLengthTypeField(fieldName, "byte[][]",
                                           PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readArrayOfByteArrays() position = %d",
            position);
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    const uint8_t* startLoc = m_dataInput->currentBufferPosition();
    int8_t** retVal = PdxLocalReader::readArrayOfByteArrays(
        fieldName, arrayLength, elementLength);
    auto strSize = m_dataInput->currentBufferPosition() - startLoc;
    m_dataInput->rewindCursor(strSize + position);
    startLoc = nullptr;
    return retVal;
  } else {
    return nullptr;
  }
}

std::shared_ptr<CacheableDate> PdxReaderWithTypeCollector::readDate(
    const std::string& fieldName) {
  checkType(fieldName, PdxFieldTypes::DATE, "Date");
  m_newPdxType->addFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE,
                                        PdxTypes::DATE_SIZE);
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);
  LOG_DEBUG("PdxReaderWithTypeCollector::readDate() position = %d", position);
  if (position != -1) {
    m_dataInput->advanceCursor(position);
    auto retVal = PdxLocalReader::readDate(fieldName);
    m_dataInput->rewindCursor(position + PdxTypes::DATE_SIZE);
    return retVal;
  } else {
    return nullptr;
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
