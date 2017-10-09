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

PdxLocalWriter::PdxLocalWriter(DataOutput& output, PdxTypePtr pdxType,
                               PdxTypeRegistryPtr pdxTypeRegistry)
    : PdxLocalWriter(output, pdxType,
                     pdxType ? pdxType->getPdxClassName() : nullptr,
                     pdxTypeRegistry)

{}

PdxLocalWriter::PdxLocalWriter(DataOutput& dataOutput, PdxTypePtr pdxType,
                               const char* pdxClassName,
                               PdxTypeRegistryPtr pdxTypeRegistry)
    : m_dataOutput(&dataOutput),
      m_pdxType(pdxType),
      m_pdxClassName(pdxClassName),
      m_startPosition(nullptr),
      m_startPositionOffset(0),
      m_domainClassName(nullptr),
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

PdxWriterPtr PdxLocalWriter::writeUnreadFields(PdxUnreadFieldsPtr unread) {
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
  return shared_from_this();
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

PdxWriterPtr PdxLocalWriter::writeChar(const char* fieldName, char value) {
  m_dataOutput->writeChar(static_cast<uint16_t>(value));
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeChar(const char* fieldName,
                                           char16_t value) {
  m_dataOutput->writeChar(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeBoolean(const char* fieldName, bool value) {
  m_dataOutput->writeBoolean(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeByte(const char* fieldName, int8_t value) {
  m_dataOutput->write(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeShort(const char* fieldName, int16_t value) {
  m_dataOutput->writeInt(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeInt(const char* fieldName, int32_t value) {
  m_dataOutput->writeInt(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeLong(const char* fieldName, int64_t value) {
  m_dataOutput->writeInt(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeFloat(const char* fieldName, float value) {
  m_dataOutput->writeFloat(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeDouble(const char* fieldName, double value) {
  m_dataOutput->writeDouble(value);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeDate(const char* fieldName,
                                       CacheableDatePtr date) {
  // m_dataOutput->writeObject(date.get());
  if (date != nullptr) {
    date->toData(*m_dataOutput);
  } else {
    m_dataOutput->writeInt(static_cast<uint64_t>(-1L));
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeString(const char* fieldName,
                                         const char* value) {
  addOffset();
  if (value == nullptr) {
    m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableNullString));
  } else {
    int32_t len = DataOutput::getEncodedLength(value);
    if (len > 0xffff) {
      // write HugUTF
      m_dataOutput->write(
          static_cast<int8_t>(GeodeTypeIds::CacheableStringHuge));
      m_dataOutput->writeUTFHuge(value);
    } else {
      // Write normal UTF
      m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableString));
      m_dataOutput->writeUTF(value);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeWideString(const char* fieldName,
                                             const wchar_t* value) {
  addOffset();
  if (value == nullptr) {
    m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableNullString));
  } else {
    int32_t len = DataOutput::getEncodedLength(value);
    if (len > 0xffff) {
      // write HugUTF
      m_dataOutput->write(
          static_cast<int8_t>(GeodeTypeIds::CacheableStringHuge));
      m_dataOutput->writeUTFHuge(value);
    } else {
      // Write normal UTF
      m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableString));
      m_dataOutput->writeUTF(value);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeStringwithoutOffset(const char* value) {
  if (value == nullptr) {
    m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableNullString));
  } else {
    int32_t len = DataOutput::getEncodedLength(value);
    if (len > 0xffff) {
      // write HugUTF
      m_dataOutput->write(
          static_cast<int8_t>(GeodeTypeIds::CacheableStringHuge));
      m_dataOutput->writeUTFHuge(value);
    } else {
      // Write normal UTF
      m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableString));
      m_dataOutput->writeUTF(value);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeWideStringwithoutOffset(
    const wchar_t* value) {
  if (value == nullptr) {
    m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableNullString));
  } else {
    int32_t len = DataOutput::getEncodedLength(value);
    if (len > 0xffff) {
      // write HugUTF
      m_dataOutput->write(
          static_cast<int8_t>(GeodeTypeIds::CacheableStringHuge));
      m_dataOutput->writeUTFHuge(value);
    } else {
      // Write normal UTF
      m_dataOutput->write(static_cast<int8_t>(GeodeTypeIds::CacheableString));
      m_dataOutput->writeUTF(value);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeStringArray(const char* fieldName,
                                              char** array, int length) {
  addOffset();
  if (array == nullptr) {
    m_dataOutput->write(static_cast<int8_t>(-1));
    // WriteByte(-1);
  } else {
    m_dataOutput->writeArrayLen(length);
    for (int i = 0; i < length; i++) {
      writeStringwithoutOffset(array[i]);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeWideStringArray(const char* fieldName,
                                                  wchar_t** array, int length) {
  addOffset();
  if (array == nullptr) {
    m_dataOutput->write(static_cast<int8_t>(-1));
  } else {
    m_dataOutput->writeArrayLen(length);
    for (int i = 0; i < length; i++) {
      writeWideStringwithoutOffset(array[i]);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeObject(const char* fieldName,
                                         SerializablePtr value) {
  addOffset();
  CacheableEnumPtr enumValPtr = nullptr;
  CacheableObjectArrayPtr objArrPtr = nullptr;
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

      m_dataOutput->write(
          static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString));
      m_dataOutput->writeASCII(actualObjPtr->getClassName());

      for (; iter != objArrPtr->end(); ++iter) {
        m_dataOutput->writeObject(*iter);
      }
    } else {
      m_dataOutput->writeObject(value);
    }
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeBooleanArray(const char* fieldName,
                                               bool* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeCharArray(const char* fieldName, char* array,
                                            int length) {
  addOffset();
  writePdxCharArray(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeWideCharArray(const char* fieldName,
                                                wchar_t* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeByteArray(const char* fieldName,
                                            int8_t* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeShortArray(const char* fieldName,
                                             int16_t* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeIntArray(const char* fieldName,
                                           int32_t* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeLongArray(const char* fieldName,
                                            int64_t* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeFloatArray(const char* fieldName,
                                             float* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeDoubleArray(const char* fieldName,
                                              double* array, int length) {
  addOffset();
  writeObject(array, length);
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeObjectArray(const char* fieldName,
                                              CacheableObjectArrayPtr array) {
  addOffset();
  if (array != nullptr) {
    array->toData(*m_dataOutput);
  } else {
    m_dataOutput->write(static_cast<int8_t>(-1));
  }
  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::writeArrayOfByteArrays(const char* fieldName,
                                                    int8_t** byteArrays,
                                                    int arrayLength,
                                                    int* elementLength) {
  addOffset();
  if (byteArrays != nullptr) {
    m_dataOutput->writeArrayLen(arrayLength);
    for (int i = 0; i < arrayLength; i++) {
      m_dataOutput->writeBytes(byteArrays[i], elementLength[i]);
    }
  } else {
    m_dataOutput->write(static_cast<int8_t>(-1));
  }

  return shared_from_this();
}

PdxWriterPtr PdxLocalWriter::markIdentityField(const char* fieldName) {
  return shared_from_this();
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

PdxTypeRegistryPtr PdxLocalWriter::getPdxTypeRegistry() const {
  return m_pdxTypeRegistry;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
