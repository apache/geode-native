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

#include <geode/TypeRegistry.hpp>

#include "PdxTypeRegistry.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

PdxLocalReader::PdxLocalReader(std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
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

PdxLocalReader::PdxLocalReader(DataInput& input,
                               std::shared_ptr<PdxType> remoteType,
                               int32_t pdxLen,
                               std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : m_dataInput(&input),
      m_pdxType(remoteType),
      m_serializedLengthWithOffsets(pdxLen),
      m_isDataNeedToPreserve(true),
      m_pdxRemotePreserveData(std::make_shared<PdxRemotePreservedData>()),
      m_localToRemoteMap(remoteType->getLocalToRemoteMap()),
      m_remoteToLocalMap(remoteType->getRemoteToLocalMap()),
      m_remoteToLocalMapSize(remoteType->getTotalFields()),
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
  m_startPosition = static_cast<int32_t>(m_dataInput->getBytesRead());

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

void PdxLocalReader::moveStream() {
  // this will reset unmaged datainput as well
  m_dataInput->reset(m_startPosition + m_serializedLengthWithOffsets);
}

char16_t PdxLocalReader::readChar(const std::string&) {
  char16_t value = m_dataInput->readInt16();
  return value;
}

bool PdxLocalReader::readBoolean(const std::string&) {
  return m_dataInput->readBoolean();
}

int8_t PdxLocalReader::readByte(const std::string&) {
  return m_dataInput->read();
}

int16_t PdxLocalReader::readShort(const std::string&) {
  return m_dataInput->readInt16();
}

int32_t PdxLocalReader::readInt(const std::string&) {
  return m_dataInput->readInt32();
}

int64_t PdxLocalReader::readLong(const std::string&) {
  return m_dataInput->readInt64();
}

float PdxLocalReader::readFloat(const std::string&) {
  return m_dataInput->readFloat();
}

double PdxLocalReader::readDouble(const std::string&) {
  return m_dataInput->readDouble();
}

std::string PdxLocalReader::readString(const std::string&) {
  return m_dataInput->readString();
}

std::shared_ptr<Serializable> PdxLocalReader::readObject(const std::string&) {
  std::shared_ptr<Serializable> ptr;
  m_dataInput->readObject(ptr);
  if (ptr != nullptr) {
    return ptr;
  } else {
    return nullptr;
  }
}

std::vector<char16_t> PdxLocalReader::readCharArray(const std::string&) {
  return m_dataInput->readCharArray();
}

std::vector<bool> PdxLocalReader::readBooleanArray(const std::string&) {
  return m_dataInput->readBooleanArray();
}

std::vector<int8_t> PdxLocalReader::readByteArray(const std::string&) {
  return m_dataInput->readByteArray();
}

std::vector<int16_t> PdxLocalReader::readShortArray(const std::string&) {
  return m_dataInput->readShortArray();
}

std::vector<int32_t> PdxLocalReader::readIntArray(const std::string&) {
  return m_dataInput->readIntArray();
}

std::vector<int64_t> PdxLocalReader::readLongArray(const std::string&) {
  return m_dataInput->readLongArray();
}

std::vector<float> PdxLocalReader::readFloatArray(const std::string&) {
  return m_dataInput->readFloatArray();
}

std::vector<double> PdxLocalReader::readDoubleArray(const std::string&) {
  return m_dataInput->readDoubleArray();
}

std::vector<std::string> PdxLocalReader::readStringArray(const std::string&) {
  return m_dataInput->readStringArray();
}

std::shared_ptr<CacheableObjectArray> PdxLocalReader::readObjectArray(
    const std::string&) {
  auto coa = CacheableObjectArray::create();
  coa->fromData(*m_dataInput);
  LOG_DEBUG("PdxLocalReader::readObjectArray coa->size() = %zu", coa->size());
  if (coa->size() <= 0) {
    coa = nullptr;
  }
  return coa;
}

int8_t** PdxLocalReader::readArrayOfByteArrays(const std::string&,
                                               int32_t& arrayLength,
                                               int32_t** elementLength) {
  int8_t** arrofBytearr = nullptr;
  m_dataInput->readArrayOfByteArrays(&arrofBytearr, arrayLength, elementLength);
  return arrofBytearr;
}

std::shared_ptr<CacheableDate> PdxLocalReader::readDate(const std::string&) {
  auto cd = CacheableDate::create();
  cd->fromData(*m_dataInput);
  return cd;
}

std::shared_ptr<PdxRemotePreservedData> PdxLocalReader::getPreservedData(
    std::shared_ptr<PdxType> mergedVersion,
    std::shared_ptr<PdxSerializable> pdxObject) {
  int nFieldExtra = m_pdxType->getNumberOfExtraFields();
  LOG_DEBUG(
      "PdxLocalReader::getPreservedData::nFieldExtra = %d AND "
      "PdxTypeRegistry::getPdxIgnoreUnreadFields = %d ",
      nFieldExtra, m_pdxTypeRegistry->getPdxIgnoreUnreadFields());
  if (nFieldExtra > 0 &&
      m_pdxTypeRegistry->getPdxIgnoreUnreadFields() == false) {
    m_pdxRemotePreserveData->initialize(
        m_pdxType != nullptr ? m_pdxType->getTypeId() : 0,
        mergedVersion->getTypeId(), pdxObject);
    LOG_DEBUG("PdxLocalReader::getPreservedData - 1");

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

        for (int j = 0; j < (nFieldPos - pos); j++) {
          pdVector.push_back(m_dataInput->read());
        }
        resettoPdxHead();

        m_pdxRemotePreserveData->setPreservedData(pdVector);
        currentIdx++;
        pdVector.erase(pdVector.begin(), pdVector.end());
      } else {
        LOG_DEBUG("PdxLocalReader::getPreservedData No need to preserve");
      }
    }

    if (m_isDataNeedToPreserve) {
      return m_pdxRemotePreserveData;
    } else {
      LOG_DEBUG(
          "PdxLocalReader::GetPreservedData m_isDataNeedToPreserve is false");
    }
  }
  return nullptr;
}

bool PdxLocalReader::hasField(const std::string& fieldName) {
  return m_pdxType->getPdxField(fieldName) != nullptr;
}

bool PdxLocalReader::isIdentityField(const std::string& fieldName) {
  auto pft = m_pdxType->getPdxField(fieldName);
  return (pft != nullptr) && (pft->getIdentityField());
}

std::shared_ptr<PdxUnreadFields> PdxLocalReader::readUnreadFields() {
  LOG_DEBUG("readUnreadFields:: %d ignore property %d", m_isDataNeedToPreserve,
            m_pdxTypeRegistry->getPdxIgnoreUnreadFields());
  if (m_pdxTypeRegistry->getPdxIgnoreUnreadFields() == true) return nullptr;
  m_isDataNeedToPreserve = false;
  return m_pdxRemotePreserveData;
}

std::shared_ptr<PdxSerializer> PdxLocalReader::getPdxSerializer() const {
  return m_dataInput->getCache()->getTypeRegistry().getPdxSerializer();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
