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
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2:
      return PdxLocalReader::readChar(fieldName);  // in same order
    case -1: {
      return '\0';  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      PdxLocalReader::resettoPdxHead();
      m_dataInput->advanceCursor(position);
      char retVal = PdxLocalReader::readChar(fieldName);
      PdxLocalReader::resettoPdxHead();
      return retVal;
    }
  }
}

bool PdxRemoteReader::readBoolean(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readBoolean(fieldName);  // in same order
    }
    case -1: {
      return 0;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        bool retVal = PdxLocalReader::readBoolean(fieldName);
        PdxLocalReader::resettoPdxHead();
        return retVal;
      } else {
        return 0;  // null value
      }
    }
  }
}

int8_t PdxRemoteReader::readByte(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readByte(fieldName);  // in same order
    }
    case -1: {
      return 0;
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        int8_t retValue;
        retValue = PdxLocalReader::readByte(fieldName);
        PdxLocalReader::resettoPdxHead();
        return retValue;
      } else {
        return 0;  // null value
      }
    }
  }
}

int16_t PdxRemoteReader::readShort(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readShort(fieldName);  // in same order
    }
    case -1: {
      return 0;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        int16_t value;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        value = PdxLocalReader::readShort(fieldName);
        PdxLocalReader::resettoPdxHead();
        return value;
      } else {
        return 0;  // null value
      }
    }
  }
}

int32_t PdxRemoteReader::readInt(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readInt(fieldName);  // in same order
    }
    case -1: {
      return 0;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        int32_t value;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        value = PdxLocalReader::readInt(fieldName);
        PdxLocalReader::resettoPdxHead();
        return value;
      } else {
        return 0;  // null value
      }
    }
  }
}

int64_t PdxRemoteReader::readLong(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readLong(fieldName);  // in same order
    }
    case -1: {
      return 0;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        int64_t value;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        value = PdxLocalReader::readLong(fieldName);
        PdxLocalReader::resettoPdxHead();
        return value;
      } else {
        return 0;  // null value
      }
    }
  }
}

float PdxRemoteReader::readFloat(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readFloat(fieldName);  // in same order
    }
    case -1: {
      return 0.0f;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        float value;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        value = PdxLocalReader::readFloat(fieldName);
        PdxLocalReader::resettoPdxHead();
        return value;
      } else {
        return 0.0f;  // null value
      }
    }
  }
}

double PdxRemoteReader::readDouble(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readDouble(fieldName);  // in same order
    }
    case -1: {
      return 0.0;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        double value;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        value = PdxLocalReader::readDouble(fieldName);
        PdxLocalReader::resettoPdxHead();
        return value;
      } else {
        return 0.0;  // null value
      }
    }
  }
}

std::string PdxRemoteReader::readString(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readString(fieldName);
    }
    case -1: {
      return std::string();
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        auto retVal = PdxLocalReader::readString(fieldName);
        PdxLocalReader::resettoPdxHead();
        return retVal;
      } else {
        static std::string emptyString;
        return emptyString;
      }
    }
  }
}

std::shared_ptr<Serializable> PdxRemoteReader::readObject(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readObject(fieldName);  // in same order
    }
    case -1: {
      return nullptr;
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::shared_ptr<Serializable> ptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        ptr = PdxLocalReader::readObject(fieldName);
        PdxLocalReader::resettoPdxHead();
        return ptr;
      } else {
        return nullptr;  // null value
      }
    }
  }
}

std::vector<char16_t> PdxRemoteReader::readCharArray(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2:
      return PdxLocalReader::readCharArray(fieldName);  // in same order
    case -1: {
      return std::vector<char16_t>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      PdxLocalReader::resettoPdxHead();
      m_dataInput->advanceCursor(position);
      auto retVal = PdxLocalReader::readCharArray(fieldName);
      PdxLocalReader::resettoPdxHead();
      return retVal;
    }
  }
}

std::vector<bool> PdxRemoteReader::readBooleanArray(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2:
      return PdxLocalReader::readBooleanArray(fieldName);  // in same order
    case -1: {
      return std::vector<bool>{}; // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      PdxLocalReader::resettoPdxHead();
      m_dataInput->advanceCursor(position);
      std::vector<bool> retVal = PdxLocalReader::readBooleanArray(fieldName);
      PdxLocalReader::resettoPdxHead();
      return retVal;
    }
  }
}

std::vector<int8_t> PdxRemoteReader::readByteArray(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readByteArray(fieldName);  // in same order
    }
    case -1: {
      return std::vector<int8_t>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::vector<int8_t> byteArrptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        byteArrptr = PdxLocalReader::readByteArray(fieldName);
        PdxLocalReader::resettoPdxHead();
        return byteArrptr;
      } else {
        return std::vector<int8_t>{};
      }
    }
  }
}

std::vector<int16_t> PdxRemoteReader::readShortArray(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readShortArray(fieldName);  // in same
                                                                 // order
    }
    case -1: {
      return std::vector<int16_t>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::vector<int16_t> shortArrptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        shortArrptr = PdxLocalReader::readShortArray(fieldName);
        PdxLocalReader::resettoPdxHead();
        return shortArrptr;
      } else {
        return std::vector<int16_t>{};
      }
    }
  }
}

std::vector<int32_t> PdxRemoteReader::readIntArray(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readIntArray(fieldName);  // in same order
    }
    case -1: {
      return std::vector<int32_t>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::vector<int32_t> intArrptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        intArrptr = PdxLocalReader::readIntArray(fieldName);
        PdxLocalReader::resettoPdxHead();
        return intArrptr;
      } else {
        return std::vector<int32_t>{};
      }
    }
  }
}

std::vector<int64_t> PdxRemoteReader::readLongArray(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readLongArray(fieldName);  // in same order
    }
    case -1: {
      return std::vector<int64_t>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::vector<int64_t> longArrptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        longArrptr = PdxLocalReader::readLongArray(fieldName);
        PdxLocalReader::resettoPdxHead();
        return longArrptr;
      } else {
        return std::vector<int64_t>{};
      }
    }
  }
}

std::vector<float> PdxRemoteReader::readFloatArray(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readFloatArray(fieldName);  // in same
                                                                 // order
    }
    case -1: {
      return std::vector<float>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::vector<float> floatArrptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        floatArrptr =
            PdxLocalReader::readFloatArray(fieldName);  // in same order
        PdxLocalReader::resettoPdxHead();
        return floatArrptr;
      } else {
        return std::vector<float>{};
      }
    }
  }
}

std::vector<double> PdxRemoteReader::readDoubleArray(const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readDoubleArray(fieldName);  // in same order
    }
    case -1: {
      return std::vector<double>{};
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        std::vector<double> doubleArrptr;
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        doubleArrptr = PdxLocalReader::readDoubleArray(
            fieldName);  // in same order
        PdxLocalReader::resettoPdxHead();
        return doubleArrptr;
      } else {
        return std::vector<double>{};
      }
    }
  }
}

std::vector<std::string> PdxRemoteReader::readStringArray(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      return PdxLocalReader::readStringArray(fieldName);
    }
    case -1: {
      return std::vector<std::string>();
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        auto strArray = PdxLocalReader::readStringArray(fieldName);
        PdxLocalReader::resettoPdxHead();
        return strArray;
      } else {
        return std::vector<std::string>();
      }
    }
  }
}

std::shared_ptr<CacheableObjectArray> PdxRemoteReader::readObjectArray(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2:
      return PdxLocalReader::readObjectArray(fieldName);  // in same order
    case -1: {
      return nullptr;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      PdxLocalReader::resettoPdxHead();
      m_dataInput->advanceCursor(position);
      auto retVal = PdxLocalReader::readObjectArray(fieldName);
      PdxLocalReader::resettoPdxHead();
      return retVal;
    }
  }
}

int8_t** PdxRemoteReader::readArrayOfByteArrays(const std::string& fieldName,
                                                int32_t& arrayLength,
                                                int32_t** elementLength) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2:
      return PdxLocalReader::readArrayOfByteArrays(
          fieldName, arrayLength, elementLength);  // in same order
    case -1:
      return nullptr;  // null value
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      PdxLocalReader::resettoPdxHead();
      m_dataInput->advanceCursor(position);
      int8_t** retVal = PdxLocalReader::readArrayOfByteArrays(
          fieldName, arrayLength, elementLength);
      PdxLocalReader::resettoPdxHead();
      return retVal;
    }
  }
}
std::shared_ptr<CacheableDate> PdxRemoteReader::readDate(
    const std::string& fieldName) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2:
      return PdxLocalReader::readDate(fieldName);  // in same order
    case -1:
      return nullptr;
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      PdxLocalReader::resettoPdxHead();
      m_dataInput->advanceCursor(position);
      auto retVal = PdxLocalReader::readDate(fieldName);
      PdxLocalReader::resettoPdxHead();
      return retVal;
    }
  }
}

void PdxRemoteReader::readCollection(
    const std::string& fieldName,
    std::shared_ptr<CacheableArrayList>& collection) {
  int choice = m_localToRemoteMap[m_currentIndex++];

  switch (choice) {
    case -2: {
      PdxLocalReader::readCollection(fieldName, collection);  // in same order
      break;
    }
    case -1: {
      collection = nullptr;
      break;  // null value
    }
    default: {
      // sequence id read field and then update
      int position = m_pdxType->getFieldPosition(
          choice, m_offsetsBuffer, m_offsetSize, m_serializedLength);
      if (position != -1) {
        PdxLocalReader::resettoPdxHead();
        m_dataInput->advanceCursor(position);
        PdxLocalReader::readCollection(fieldName, collection);
        PdxLocalReader::resettoPdxHead();
      } else {
        collection = nullptr;
      }
      break;
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
