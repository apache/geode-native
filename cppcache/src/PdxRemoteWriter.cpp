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
 * PdxRemoteWriter.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxRemoteWriter.hpp"

#include "PdxTypeRegistry.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

PdxRemoteWriter::PdxRemoteWriter(
    DataOutput& output, std::shared_ptr<PdxType> pdxType,
    std::shared_ptr<PdxRemotePreservedData> preservedData,
    std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : PdxLocalWriter(output, pdxType, pdxTypeRegistry),
      m_preserveDataIdx(0),
      m_currentDataIdx(-1),
      m_remoteTolocalMapLength(0) {
  m_preserveData = preservedData;
  if (m_pdxType != nullptr) {
    m_remoteTolocalMap = m_pdxType->getRemoteToLocalMap();
    m_remoteTolocalMapLength = m_pdxType->getTotalFields();
  }
  m_pdxClassName = pdxType->getPdxClassName();

  initialize();
}

PdxRemoteWriter::PdxRemoteWriter(
    DataOutput& output, std::string pdxClassName,
    std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : PdxLocalWriter(output, nullptr, pdxClassName, pdxTypeRegistry),
      m_preserveDataIdx(0),
      m_currentDataIdx(-1),
      m_remoteTolocalMapLength(0) {
  m_preserveData = nullptr;
  if (m_pdxType != nullptr) {
    m_remoteTolocalMapLength = m_pdxType->getTotalFields();
    m_remoteTolocalMap = m_pdxType->getRemoteToLocalMap();
  }
  initialize();
}

void PdxRemoteWriter::endObjectWriting() {
  writePreserveData();
  // write header
  PdxLocalWriter::writePdxHeader();
}

void PdxRemoteWriter::writePreserveData() {
  m_currentDataIdx++;  // it starts from -1
  LOG_DEBUG("PdxRemoteWriter::writePreserveData m_currentDataIdx = %d",
            m_currentDataIdx);
  LOG_DEBUG(
      "PdxRemoteWriter::writePreserveData m_remoteTolocalMap->Length = %d",
      m_remoteTolocalMapLength);

  if (m_preserveData != nullptr) {
    while (m_currentDataIdx < m_remoteTolocalMapLength) {
      if (m_remoteTolocalMap[m_currentDataIdx] ==
          -1)  // need to add preserve data with offset
      {
        PdxLocalWriter::addOffset();
        for (size_t i = 0;
             i < (m_preserveData->getPreservedData(m_preserveDataIdx)).size();
             i++) {
          m_dataOutput->write(
              (m_preserveData->getPreservedData(m_preserveDataIdx))[i]);
        }
        m_preserveDataIdx++;
        m_currentDataIdx++;
      } else if (m_remoteTolocalMap[m_currentDataIdx] ==
                 -2)  // need to add preserve data WITHOUT offset
      {
        for (size_t i = 0;
             i < (m_preserveData->getPreservedData(m_preserveDataIdx)).size();
             i++) {
          m_dataOutput->write(
              (m_preserveData->getPreservedData(m_preserveDataIdx))[i]);
        }
        m_preserveDataIdx++;
        m_currentDataIdx++;

      } else {
        break;  // continue writing local data..
      }
    }
  }
}

void PdxRemoteWriter::initialize() {
  // this is default case
  if (m_preserveData == nullptr) {
    m_pdxType = getPdxTypeRegistry()->getLocalPdxType(m_pdxClassName);
  }
}

bool PdxRemoteWriter::isFieldWritingStarted() {
  return m_currentDataIdx != -1;  // field writing NOT started. do we need
                                  // this??
}

PdxWriter& PdxRemoteWriter::writeUnreadFields(
    std::shared_ptr<PdxUnreadFields> unread) {
  PdxLocalWriter::writeUnreadFields(unread);
  m_remoteTolocalMap = m_pdxType->getRemoteToLocalMap();
  m_remoteTolocalMapLength = m_pdxType->getTotalFields();
  return *this;
}

PdxWriter& PdxRemoteWriter::writeChar(const std::string& fieldName,
                                      char16_t value) {
  writePreserveData();
  PdxLocalWriter::writeChar(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeBoolean(const std::string& fieldName,
                                         bool value) {
  writePreserveData();
  PdxLocalWriter::writeBoolean(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeByte(const std::string& fieldName,
                                      int8_t value) {
  writePreserveData();
  PdxLocalWriter::writeByte(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeShort(const std::string& fieldName,
                                       int16_t value) {
  writePreserveData();
  PdxLocalWriter::writeShort(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeInt(const std::string& fieldName,
                                     int32_t value) {
  writePreserveData();
  PdxLocalWriter::writeInt(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeLong(const std::string& fieldName,
                                      int64_t value) {
  writePreserveData();
  PdxLocalWriter::writeLong(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeFloat(const std::string& fieldName,
                                       float value) {
  writePreserveData();
  PdxLocalWriter::writeFloat(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeDouble(const std::string& fieldName,
                                        double value) {
  writePreserveData();
  PdxLocalWriter::writeDouble(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeDate(const std::string& fieldName,
                                      std::shared_ptr<CacheableDate> date) {
  writePreserveData();
  PdxLocalWriter::writeDate(fieldName, date);
  return *this;
}

PdxWriter& PdxRemoteWriter::writeString(const std::string& fieldName,
                                        const std::string& value) {
  writePreserveData();
  PdxLocalWriter::writeString(fieldName, value);
  return *this;
}

PdxWriter& PdxRemoteWriter::writeStringArray(
    const std::string& fieldName, const std::vector<std::string>& array) {
  writePreserveData();
  PdxLocalWriter::writeStringArray(fieldName, array);
  return *this;
}

PdxWriter& PdxRemoteWriter::writeObject(const std::string& fieldName,
                                        std::shared_ptr<Serializable> value) {
  writePreserveData();
  PdxLocalWriter::writeObject(fieldName, value);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeBooleanArray(const std::string& fieldName,
                                              const std::vector<bool>& array) {
  writePreserveData();
  PdxLocalWriter::writeBooleanArray(fieldName, array);
  return *this;
}

PdxWriter& PdxRemoteWriter::writeCharArray(const std::string& fieldName,
                                           const std::vector<char16_t>& array) {
  writePreserveData();
  PdxLocalWriter::writeCharArray(fieldName, array);
  return *this;
}

PdxWriter& PdxRemoteWriter::writeByteArray(const std::string& fieldName,
                                           const std::vector<int8_t>& array) {
  writePreserveData();
  PdxLocalWriter::writeByteArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeShortArray(const std::string& fieldName,
                                            const std::vector<int16_t>& array) {
  writePreserveData();
  PdxLocalWriter::writeShortArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeIntArray(const std::string& fieldName,
                                          const std::vector<int32_t>& array) {
  writePreserveData();
  PdxLocalWriter::writeIntArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeLongArray(const std::string& fieldName,
                                           const std::vector<int64_t>& array) {
  writePreserveData();
  PdxLocalWriter::writeLongArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeFloatArray(const std::string& fieldName,
                                            const std::vector<float>& array) {
  writePreserveData();
  PdxLocalWriter::writeFloatArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeDoubleArray(const std::string& fieldName,
                                             const std::vector<double>& array) {
  writePreserveData();
  PdxLocalWriter::writeDoubleArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeObjectArray(
    const std::string& fieldName, std::shared_ptr<CacheableObjectArray> array) {
  writePreserveData();
  PdxLocalWriter::writeObjectArray(fieldName, array);
  return *this;
}
PdxWriter& PdxRemoteWriter::writeArrayOfByteArrays(
    const std::string& fieldName, int8_t* const* const byteArrays,
    int arrayLength, const int* elementLength) {
  writePreserveData();
  PdxLocalWriter::writeArrayOfByteArrays(fieldName, byteArrays, arrayLength,
                                         elementLength);
  return *this;
}
std::shared_ptr<PdxTypeRegistry> PdxRemoteWriter::getPdxTypeRegistry() const {
  return m_pdxTypeRegistry;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
