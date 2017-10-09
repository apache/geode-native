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
 * PdxLocalReader.cpp
 * Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxLocalReader.hpp"
#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

PdxLocalReader::PdxLocalReader(PdxTypeRegistryPtr pdxTypeRegistry)
    : m_dataInput(nullptr),
      m_startBuffer(nullptr),
      m_startPosition(0),
      m_serializedLength(0),
      m_serializedLengthWithOffsets(0),
      m_offsetSize(0),
      m_offsetsBuffer(nullptr),
      m_isDataNeedToPreserve(false),
      m_localToRemoteMap(nullptr),
      m_remoteToLocalMap(nullptr),
      m_remoteToLocalMapSize(0),
      m_pdxTypeRegistry(pdxTypeRegistry) {}

PdxLocalReader::PdxLocalReader(DataInput& input, PdxTypePtr remoteType,
                               int32_t pdxLen,
                               PdxTypeRegistryPtr pdxTypeRegistry)
    : m_dataInput(&input),
      m_pdxType(remoteType),
      m_serializedLengthWithOffsets(pdxLen),
      m_localToRemoteMap(remoteType->getLocalToRemoteMap()),
      m_remoteToLocalMap(remoteType->getRemoteToLocalMap()),
      m_remoteToLocalMapSize(remoteType->getTotalFields()),
      m_pdxRemotePreserveData(std::make_shared<PdxRemotePreservedData>()),
      m_isDataNeedToPreserve(true),
      m_pdxTypeRegistry(pdxTypeRegistry) {
  initialize();
}

PdxLocalReader::~PdxLocalReader() {}

void PdxLocalReader::resettoPdxHead() {
  int32_t pdxHeadOffset = static_cast<int32_t>(
      m_dataInput->currentBufferPosition() - m_startBuffer);
  m_dataInput->rewindCursor(pdxHeadOffset);
}

void PdxLocalReader::initialize() {
  m_startBuffer = const_cast<uint8_t*>(m_dataInput->currentBufferPosition());
  m_startPosition = m_dataInput->getBytesRead();  // number of bytes read in
                                                  // c++;

  if (m_serializedLengthWithOffsets <= 0xff) {
    m_offsetSize = 1;
  } else if (m_serializedLengthWithOffsets <= 0xffff) {
    m_offsetSize = 2;
  } else {
    m_offsetSize = 4;
  }

  if (m_pdxType->getNumberOfVarLenFields() > 0) {
    m_serializedLength =
        m_serializedLengthWithOffsets -
        ((m_pdxType->getNumberOfVarLenFields() - 1) * m_offsetSize);
  } else {
    m_serializedLength = m_serializedLengthWithOffsets;
  }
  m_offsetsBuffer = m_startBuffer + m_serializedLength;
}

void PdxLocalReader::MoveStream() {
  // this will reset unmaged datainput as well
  m_dataInput->reset(m_startPosition + m_serializedLengthWithOffsets);
}

void PdxLocalReader::checkEmptyFieldName(const char* fieldName) {
  if (fieldName == nullptr) {
    throw IllegalStateException("Field name is null");
  }
}

char PdxLocalReader::readChar(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  uint16_t value = m_dataInput->readInt16();
  return (static_cast<char>(value));
}

wchar_t PdxLocalReader::readWideChar(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  uint16_t value = m_dataInput->readInt16();
  return static_cast<wchar_t>(value);
}

bool PdxLocalReader::readBoolean(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->readBoolean();
}

int8_t PdxLocalReader::readByte(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->read();
}

int16_t PdxLocalReader::readShort(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->readInt16();
}

int32_t PdxLocalReader::readInt(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->readInt32();
}

int64_t PdxLocalReader::readLong(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->readInt64();
}

float PdxLocalReader::readFloat(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->readFloat();
}

double PdxLocalReader::readDouble(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  return m_dataInput->readDouble();
}

char* PdxLocalReader::readString(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  char* str;
  m_dataInput->readString(&str);
  return str;
}

wchar_t* PdxLocalReader::readWideString(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  wchar_t* str;
  m_dataInput->readWideString(&str);
  return str;
}

SerializablePtr PdxLocalReader::readObject(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  SerializablePtr ptr;
  m_dataInput->readObject(ptr);
  if (ptr != nullptr) {
    return ptr;
  } else {
    return nullptr;
  }
}

char* PdxLocalReader::readCharArray(const char* fieldName,
                                    int32_t& length) {  // TODO:: need to return
                                                        // Length to user for
                                                        // all primitive arrays
  checkEmptyFieldName(fieldName);
  char* charArray = nullptr;
  m_dataInput->readCharArray(&charArray, length);
  return charArray;
}

wchar_t* PdxLocalReader::readWideCharArray(
    const char* fieldName,
    int32_t& length) {  // TODO:: need to return Length to user for all
                        // primitive arrays
  checkEmptyFieldName(fieldName);
  wchar_t* charArray = nullptr;
  m_dataInput->readWideCharArray(&charArray, length);
  return charArray;
}
bool* PdxLocalReader::readBooleanArray(const char* fieldName, int32_t& length) {
  checkEmptyFieldName(fieldName);
  bool* boolArray = nullptr;
  m_dataInput->readBooleanArray(&boolArray, length);
  return boolArray;
}

int8_t* PdxLocalReader::readByteArray(const char* fieldName, int32_t& length) {
  checkEmptyFieldName(fieldName);
  int8_t* byteArray = nullptr;
  m_dataInput->readByteArray(&byteArray, length);
  return byteArray;
}

int16_t* PdxLocalReader::readShortArray(const char* fieldName,
                                        int32_t& length) {
  checkEmptyFieldName(fieldName);
  int16_t* shortArray = nullptr;
  m_dataInput->readShortArray(&shortArray, length);
  return shortArray;
}

int32_t* PdxLocalReader::readIntArray(const char* fieldName, int32_t& length) {
  checkEmptyFieldName(fieldName);
  int32_t* intArray = nullptr;
  m_dataInput->readIntArray(&intArray, length);
  return intArray;
}

int64_t* PdxLocalReader::readLongArray(const char* fieldName, int32_t& length) {
  checkEmptyFieldName(fieldName);
  int64_t* longArray = nullptr;
  m_dataInput->readLongArray(&longArray, length);
  return longArray;
}

float* PdxLocalReader::readFloatArray(const char* fieldName, int32_t& length) {
  checkEmptyFieldName(fieldName);
  float* floatArray = nullptr;
  m_dataInput->readFloatArray(&floatArray, length);
  return floatArray;
}

double* PdxLocalReader::readDoubleArray(const char* fieldName,
                                        int32_t& length) {
  checkEmptyFieldName(fieldName);
  double* doubleArray = nullptr;
  m_dataInput->readDoubleArray(&doubleArray, length);
  return doubleArray;
}

char** PdxLocalReader::readStringArray(const char* fieldName, int32_t& length) {
  checkEmptyFieldName(fieldName);
  char** stringArray = nullptr;
  m_dataInput->readStringArray(&stringArray, length);
  return stringArray;
}

wchar_t** PdxLocalReader::readWideStringArray(const char* fieldName,
                                              int32_t& length) {
  checkEmptyFieldName(fieldName);
  wchar_t** stringArray = nullptr;
  m_dataInput->readWideStringArray(&stringArray, length);
  return stringArray;
}

CacheableObjectArrayPtr PdxLocalReader::readObjectArray(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  CacheableObjectArrayPtr coa = CacheableObjectArray::create();
  coa->fromData(*m_dataInput);
  LOGDEBUG("PdxLocalReader::readObjectArray coa->size() = %d", coa->size());
  if (coa->size() <= 0) {
    coa = nullptr;
  }
  return coa;
}

int8_t** PdxLocalReader::readArrayOfByteArrays(const char* fieldName,
                                               int32_t& arrayLength,
                                               int32_t** elementLength) {
  checkEmptyFieldName(fieldName);
  int8_t** arrofBytearr = nullptr;
  m_dataInput->readArrayOfByteArrays(&arrofBytearr, arrayLength, elementLength);
  return arrofBytearr;
}

CacheableDatePtr PdxLocalReader::readDate(const char* fieldName) {
  checkEmptyFieldName(fieldName);
  CacheableDatePtr cd = CacheableDate::create();
  cd->fromData(*m_dataInput);
  return cd;
}

PdxRemotePreservedDataPtr PdxLocalReader::getPreservedData(
    PdxTypePtr mergedVersion, PdxSerializablePtr pdxObject) {
  int nFieldExtra = m_pdxType->getNumberOfExtraFields();
  LOGDEBUG(
      "PdxLocalReader::getPreservedData::nFieldExtra = %d AND "
      "PdxTypeRegistry::getPdxIgnoreUnreadFields = %d ",
      nFieldExtra, m_pdxTypeRegistry->getPdxIgnoreUnreadFields());
  if (nFieldExtra > 0 &&
      m_pdxTypeRegistry->getPdxIgnoreUnreadFields() == false) {
    m_pdxRemotePreserveData->initialize(
        m_pdxType != nullptr ? m_pdxType->getTypeId() : 0,
        mergedVersion->getTypeId(), nFieldExtra, pdxObject);
    LOGDEBUG("PdxLocalReader::getPreservedData - 1");

    m_localToRemoteMap = m_pdxType->getLocalToRemoteMap();
    m_remoteToLocalMap = m_pdxType->getRemoteToLocalMap();

    int currentIdx = 0;
    std::vector<int8_t> pdVector;
    for (int i = 0; i < m_remoteToLocalMapSize; i++) {
      if (m_remoteToLocalMap[i] == -1 ||
          m_remoteToLocalMap[i] == -2)  // this field needs to preserve
      {
        int pos = m_pdxType->getFieldPosition(i, m_offsetsBuffer, m_offsetSize,
                                              m_serializedLength);
        int nFieldPos = 0;

        if (i == m_remoteToLocalMapSize - 1) {
          nFieldPos = m_serializedLength;
        } else {
          nFieldPos = m_pdxType->getFieldPosition(
              i + 1, m_offsetsBuffer, m_offsetSize, m_serializedLength);
        }

        resettoPdxHead();
        m_dataInput->advanceCursor(pos);

        for (int i = 0; i < (nFieldPos - pos); i++) {
          pdVector.push_back(m_dataInput->read());
        }
        resettoPdxHead();

        m_pdxRemotePreserveData->setPreservedData(pdVector);
        currentIdx++;
        pdVector.erase(pdVector.begin(), pdVector.end());
      } else {
        LOGDEBUG("PdxLocalReader::getPreservedData No need to preserve");
      }
    }

    if (m_isDataNeedToPreserve) {
      return m_pdxRemotePreserveData;
    } else {
      LOGDEBUG(
          "PdxLocalReader::GetPreservedData m_isDataNeedToPreserve is false");
    }
  }
  return nullptr;
}

bool PdxLocalReader::hasField(const char* fieldName) {
  return m_pdxType->getPdxField(fieldName) != nullptr;
}

bool PdxLocalReader::isIdentityField(const char* fieldName) {
  PdxFieldTypePtr pft = m_pdxType->getPdxField(fieldName);
  return (pft != nullptr) && (pft->getIdentityField());
}

void PdxLocalReader::readCollection(const char* fieldName,
                                    CacheableArrayListPtr& collection) {
  collection = m_dataInput->readObject<CacheableArrayList>();
}

PdxUnreadFieldsPtr PdxLocalReader::readUnreadFields() {
  LOGDEBUG("readUnreadFields:: %d ignore property %d", m_isDataNeedToPreserve,
           m_pdxTypeRegistry->getPdxIgnoreUnreadFields());
  if (m_pdxTypeRegistry->getPdxIgnoreUnreadFields() == true) return nullptr;
  m_isDataNeedToPreserve = false;
  return m_pdxRemotePreserveData;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
