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
 * PdxLocalWriter.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxLocalWriter.hpp"
#include "PdxHelper.hpp"
#include "PdxTypeRegistry.hpp"
#include <geode/CacheableEnum.hpp>
#include "GeodeTypeIdsImpl.hpp"

namespace apache {
namespace geode {
namespace client {

PdxLocalWriter::PdxLocalWriter(DataOutput& output,
                               std::shared_ptr<PdxType> pdxType,
                               std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : PdxLocalWriter(output, pdxType, pdxType ? pdxType->getPdxClassName() : "",
                     pdxTypeRegistry)

{}

PdxLocalWriter::PdxLocalWriter(DataOutput& dataOutput,
                               std::shared_ptr<PdxType> pdxType,
                               std::string pdxClassName,
                               std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry)
    : m_dataOutput(&dataOutput),
      m_pdxType(pdxType),
      m_pdxClassName(pdxClassName),
      m_startPosition(nullptr),
      m_startPositionOffset(0),
      m_domainClassName(""),
      m_currentOffsetIndex(0),
      m_pdxTypeRegistry(pdxTypeRegistry) {
  initialize();
}

PdxLocalWriter::~PdxLocalWriter() {}

void PdxLocalWriter::initialize() {
  if (m_pdxType != nullptr) {
    m_currentOffsetIndex = 0;
  }

  // start position, this should start of dataoutput buffer and then use
  // bufferlen
  m_startPosition = m_dataOutput->getBuffer();

  // data has been write
  m_startPositionOffset = m_dataOutput->getBufferLength();

  // Advance cursor to write pdx header
  m_dataOutput->advanceCursor(PdxHelper::PdxHeader);
}

void PdxLocalWriter::addOffset() {
  // bufferLen gives lenght which has been written to DataOutput
  // m_startPositionOffset: from where pdx header length starts
  int bufferLen = m_dataOutput->getBufferLength() - m_startPositionOffset;

  int offset = bufferLen - PdxHelper::PdxHeader;

  m_offsets.push_back(offset);
}

void PdxLocalWriter::endObjectWriting() {
  // Write header for pdx.
  writePdxHeader();
}

void PdxLocalWriter::writePdxHeader() {
  int32_t len = calculateLenWithOffsets();
  int32_t typeId = m_pdxType->getTypeId();

  const uint8_t* starpos = m_dataOutput->getBuffer() + m_startPositionOffset;
  PdxHelper::writeInt32(const_cast<uint8_t*>(starpos), len);
  PdxHelper::writeInt32(const_cast<uint8_t*>(starpos + 4), typeId);

  writeOffsets(len);
}

void PdxLocalWriter::writeOffsets(int32_t len) {
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
PdxWriter& PdxLocalWriter::writeUnreadFields(
    std::shared_ptr<PdxUnreadFields> unread) {
  if (isFieldWritingStarted()) {
    throw IllegalStateException(
        "WriteUnreadFields must be called before any other fields are "
        "written.");
  }

  if (unread != nullptr) {
    m_preserveData = std::dynamic_pointer_cast<PdxRemotePreservedData>(unread);
    if (m_preserveData != nullptr) {
      m_pdxType =
          getPdxTypeRegistry()->getPdxType(m_preserveData->getMergedTypeId());
      if (m_pdxType == nullptr) {
        // its local type
        // this needs to fix for IPdxTypemapper
        m_pdxType = getPdxTypeRegistry()->getLocalPdxType(m_pdxClassName);
      }
    } else {
      throw IllegalStateException(
          "PdxLocalWriter::writeUnreadFields: m_preserveData should not be "
          "nullptr");
    }
  }
  return *this;
}

int32_t PdxLocalWriter::calculateLenWithOffsets() {
  int bufferLen = m_dataOutput->getBufferLength() - m_startPositionOffset;
  int32_t totalOffsets = 0;
  if (m_pdxType->getNumberOfVarLenFields() > 0) {
    totalOffsets = m_pdxType->getNumberOfVarLenFields() -
                   1;  // for first var len no need to append offset
  }
  int32_t totalLen = bufferLen - PdxHelper::PdxHeader + totalOffsets;

  if (totalLen <= 0xff) {  // 1 byte
    return totalLen;
  } else if (totalLen + totalOffsets <= 0xffff) {  // 2 byte
    return totalLen + totalOffsets;
  } else {  // 4 byte
    return totalLen + totalOffsets * 3;
  }
}

bool PdxLocalWriter::isFieldWritingStarted() { return true; }

PdxWriter& PdxLocalWriter::writeChar(
    const std::string& fieldName, char16_t value) {
  m_dataOutput->writeChar(value);
  return *this;
}
PdxWriter& PdxLocalWriter::writeBoolean(const std::string& fieldName,
                                        bool value) {
  m_dataOutput->writeBoolean(value);
  return *this;
 }
 PdxWriter& PdxLocalWriter::writeByte(const std::string& fieldName,
                                      int8_t value) {
   m_dataOutput->write(value);
   return *this;
 }
 PdxWriter& PdxLocalWriter::writeShort(const std::string& fieldName,
                                       int16_t value) {
   m_dataOutput->writeInt(value);
   return *this;
}
PdxWriter& PdxLocalWriter::writeInt(const std::string& fieldName,
                                    int32_t value) {
  m_dataOutput->writeInt(value);
  return *this;
 }
 PdxWriter& PdxLocalWriter::writeLong(const std::string& fieldName,
                                      int64_t value) {
   m_dataOutput->writeInt(value);
   return *this;
 }
 PdxWriter& PdxLocalWriter::writeFloat(const std::string& fieldName,
                                       float value) {
   m_dataOutput->writeFloat(value);
   return *this;
 }
 PdxWriter& PdxLocalWriter::writeDouble(const std::string& fieldName,
                                        double value) {
   m_dataOutput->writeDouble(value);
   return *this;
 }
 PdxWriter& PdxLocalWriter::writeDate(const std::string& fieldName,
                                      std::shared_ptr<CacheableDate> date) {
   // m_dataOutput->writeObject(date.get());
   if (date != nullptr) {
     date->toData(*m_dataOutput);
   } else {
     m_dataOutput->writeInt(static_cast<uint64_t>(-1L));
   }
   return *this;
}

PdxWriter& PdxLocalWriter::writeString(
    const std::string& fieldName, const std::string& value) {
  addOffset();
  m_dataOutput->writeString(value);
  return *this;
}

PdxWriter& PdxLocalWriter::writeStringArray(
    const std::string& fieldName, const std::vector<std::string>& array) {
  addOffset();
  m_dataOutput->writeArrayLen(array.size());
  for (auto&& entry : array) {
    m_dataOutput->writeString(entry);
  }
  return *this;
}

PdxWriter& PdxLocalWriter::writeObject(
    const std::string& fieldName, std::shared_ptr<Serializable> value) {
  addOffset();
  std::shared_ptr<CacheableEnum> enumValPtr = nullptr;
  std::shared_ptr<CacheableObjectArray> objArrPtr = nullptr;
  /*if (value != nullptr) {
    try {
      enumValPtr = std::dynamic_pointer_cast<CacheableEnum>(value);
    }
    catch (const ClassCastException&) {
      //ignore
    }
  }*/

  if (value != nullptr &&
      value->typeId() == static_cast<int8_t>(GeodeTypeIds::CacheableEnum)) {
    enumValPtr = std::dynamic_pointer_cast<CacheableEnum>(value);
  }

  if (enumValPtr != nullptr) {
    enumValPtr->toData(*m_dataOutput);
  } else {
    if (value != nullptr &&
        value->typeId() == GeodeTypeIds::CacheableObjectArray) {
      objArrPtr = std::dynamic_pointer_cast<CacheableObjectArray>(value);
      m_dataOutput->write(
          static_cast<int8_t>(GeodeTypeIds::CacheableObjectArray));
      m_dataOutput->writeArrayLen(static_cast<int32_t>(objArrPtr->size()));
      m_dataOutput->write(static_cast<int8_t>(GeodeTypeIdsImpl::Class));

      auto iter = objArrPtr->begin();
      const auto actualObjPtr =
          std::dynamic_pointer_cast<PdxSerializable>(*iter);

      m_dataOutput->writeString(actualObjPtr->getClassName());

      for (; iter != objArrPtr->end(); ++iter) {
        m_dataOutput->writeObject(*iter);
      }
    } else {
      m_dataOutput->writeObject(value);
    }
  }
  return *this;
}
PdxWriter& PdxLocalWriter::writeBooleanArray(const std::string& fieldName,
                                             const std::vector<bool>& array) {
  addOffset();
  writeArrayObject(array);
  return *this;
}

PdxWriter& PdxLocalWriter::writeCharArray(
    const std::string& fieldName, const std::vector<char16_t>& array) {
  addOffset();
  writeArrayObject(array);
  return *this;
}

PdxWriter& PdxLocalWriter::writeByteArray(
    const std::string& fieldName, const std::vector<int8_t>& array) {
  addOffset();
  writeArrayObject(array);
  return *this;
}
PdxWriter& PdxLocalWriter::writeShortArray(const std::string& fieldName,
                                           const std::vector<int16_t>& array) {
  addOffset();
  writeArrayObject(array);
  return *this;
}
PdxWriter& PdxLocalWriter::writeIntArray(const std::string& fieldName,
                                         const std::vector<int32_t>& array) {
  addOffset();
  writeArrayObject(array);
  return *this;
}
PdxWriter& PdxLocalWriter::writeLongArray(const std::string& fieldName,
                                          const std::vector<int64_t>& array) {
  addOffset();
  writeArrayObject(array);
  return *this;
 }
 PdxWriter& PdxLocalWriter::writeFloatArray(const std::string& fieldName,
                                            const std::vector<float>& array) {
   addOffset();
   writeArrayObject(array);
   return *this;
 }
 PdxWriter& PdxLocalWriter::writeDoubleArray(const std::string& fieldName,
                                             const std::vector<double>& array) {
   addOffset();
   writeArrayObject(array);
   return *this;
 }
 PdxWriter& PdxLocalWriter::writeObjectArray(
     const std::string& fieldName,
     std::shared_ptr<CacheableObjectArray> array) {
   addOffset();
   if (array != nullptr) {
     array->toData(*m_dataOutput);
   } else {
     m_dataOutput->write(static_cast<int8_t>(-1));
   }
   return *this;
}
PdxWriter& PdxLocalWriter::writeArrayOfByteArrays(
    const std::string& fieldName, int8_t* const* const byteArrays,
    int arrayLength, const int* elementLength) {
  addOffset();
  if (byteArrays != nullptr) {
    m_dataOutput->writeArrayLen(arrayLength);
    for (int i = 0; i < arrayLength; i++) {
      m_dataOutput->writeBytes(byteArrays[i], elementLength[i]);
    }
  } else {
    m_dataOutput->write(static_cast<int8_t>(-1));
  }

  return *this;
}
PdxWriter& PdxLocalWriter::markIdentityField(const std::string& fieldName) {
  return *this;
}

uint8_t* PdxLocalWriter::getPdxStream(int& pdxLen) {
  uint8_t* stPos =
      const_cast<uint8_t*>(m_dataOutput->getBuffer()) + m_startPositionOffset;
  int len = PdxHelper::readInt32 /*readByte*/ (stPos);
  pdxLen = len;
  // ignore len and typeid
  return m_dataOutput->getBufferCopyFrom(stPos + 8, len);
}

void PdxLocalWriter::writeByte(int8_t byte) { m_dataOutput->write(byte); }
std::shared_ptr<PdxTypeRegistry> PdxLocalWriter::getPdxTypeRegistry() const {
  return m_pdxTypeRegistry;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
