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
 * PdxRemoteReader.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxRemoteReader.hpp"

#include "PdxTypes.hpp"

namespace apache {
namespace geode {
namespace client {

PdxRemoteReader::~PdxRemoteReader() {
  // TODO Auto-generated destructor stub
}

char16_t PdxRemoteReader::readChar(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return '\0';
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readChar(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

bool PdxRemoteReader::readBoolean(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return false;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  bool result = PdxLocalReader::readBoolean(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

int8_t PdxRemoteReader::readByte(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return 0;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readByte(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

int16_t PdxRemoteReader::readShort(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return 0;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readShort(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

int32_t PdxRemoteReader::readInt(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return 0;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readInt(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

int64_t PdxRemoteReader::readLong(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return 0;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readLong(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

float PdxRemoteReader::readFloat(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return 0.f;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readFloat(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

double PdxRemoteReader::readDouble(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return 0.;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readDouble(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::string PdxRemoteReader::readString(const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readString(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::shared_ptr<Serializable> PdxRemoteReader::readObject(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return nullptr;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readObject(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<char16_t> PdxRemoteReader::readCharArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readCharArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<bool> PdxRemoteReader::readBooleanArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readBooleanArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<int8_t> PdxRemoteReader::readByteArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readByteArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<int16_t> PdxRemoteReader::readShortArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readShortArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<int32_t> PdxRemoteReader::readIntArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readIntArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<int64_t> PdxRemoteReader::readLongArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readLongArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<float> PdxRemoteReader::readFloatArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readFloatArray(fieldName);  // in same order
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<double> PdxRemoteReader::readDoubleArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readDoubleArray(fieldName);  // in same order
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::vector<std::string> PdxRemoteReader::readStringArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readStringArray(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

std::shared_ptr<CacheableObjectArray> PdxRemoteReader::readObjectArray(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readObjectArray(fieldName);
  PdxLocalReader::resettoPdxHead();
  return result;
}

int8_t** PdxRemoteReader::readArrayOfByteArrays(const std::string& fieldName,
                                                int32_t& arrayLength,
                                                int32_t** elementLength) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return nullptr;
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readArrayOfByteArrays(fieldName, arrayLength,
                                                      elementLength);

  PdxLocalReader::resettoPdxHead();
  return result;
}

std::shared_ptr<CacheableDate> PdxRemoteReader::readDate(
    const std::string& fieldName) {
  int position = m_pdxType->getFieldPosition(fieldName, m_offsetsBuffer,
                                             m_offsetSize, m_serializedLength);

  if (position == -1) {
    return {};
  }

  PdxLocalReader::resettoPdxHead();
  m_dataInput->advanceCursor(position);

  auto result = PdxLocalReader::readDate(fieldName);
  PdxLocalReader::resettoPdxHead();

  return result;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
