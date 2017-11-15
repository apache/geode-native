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

#pragma once

#ifndef GEODE_PDXLOCALWRITER_H_
#define GEODE_PDXLOCALWRITER_H_

#include <vector>

#include <geode/PdxWriter.hpp>
#include <geode/DataOutput.hpp>
#include <geode/CacheableObjectArray.hpp>

#include "PdxType.hpp"
#include "PdxRemotePreservedData.hpp"
#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxLocalWriter : public PdxWriter,
                       public std::enable_shared_from_this<PdxLocalWriter> {
 protected:
  DataOutput* m_dataOutput;
  std::shared_ptr<PdxType> m_pdxType;
  const uint8_t* m_startPosition;
  int32_t m_startPositionOffset;
  std::string m_domainClassName;
  std::vector<int32_t> m_offsets;
  int32_t m_currentOffsetIndex;

  std::shared_ptr<PdxRemotePreservedData> m_preserveData;
  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;
  std::string m_pdxClassName;

  std::shared_ptr<PdxWriter> writeStringwithoutOffset(const char* value);

  std::shared_ptr<PdxWriter> writeWideStringwithoutOffset(const wchar_t* value);

 public:
  PdxLocalWriter(DataOutput& output, std::shared_ptr<PdxType> pdxType,
                 std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  PdxLocalWriter(DataOutput& output, std::shared_ptr<PdxType> pdxType,
                 std::string pdxDomainType,
                 std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  virtual ~PdxLocalWriter();

  void initialize();

  virtual void addOffset();

  virtual void endObjectWriting();

  void writePdxHeader();

  virtual void writeOffsets(int32_t len);

  virtual int32_t calculateLenWithOffsets();

  virtual bool isFieldWritingStarted();

  inline void writeObject(bool value) { m_dataOutput->writeBoolean(value); }

  inline void writeObject(wchar_t value) {
    m_dataOutput->writeInt(static_cast<uint16_t>(value));
  }

  inline void writePdxChar(char value) {
    m_dataOutput->writeInt(static_cast<uint16_t>(value));
  }

  inline void writePdxCharArray(char* objArray, int arrayLen) {
    if (objArray != nullptr) {
      m_dataOutput->writeArrayLen(arrayLen);
      if (arrayLen > 0) {
        char* ptr = objArray;
        int i = 0;
        for (i = 0; i < arrayLen; i++) {
          writePdxChar(*ptr++);
        }
      }
    } else {
      m_dataOutput->write(static_cast<uint8_t>(0xff));
    }
  }

  inline void writeObject(int8_t value) { m_dataOutput->write(value); }

  inline void writeObject(int16_t value) { m_dataOutput->writeInt(value); }

  inline void writeObject(int32_t value) { m_dataOutput->writeInt(value); }

  inline void writeObject(int64_t value) { m_dataOutput->writeInt(value); }

  inline void writeObject(float value) { m_dataOutput->writeFloat(value); }

  inline void writeObject(double value) { m_dataOutput->writeDouble(value); }

  template <typename mType>
  void writeObject(mType* objArray, int arrayLen) {
    if (objArray != nullptr) {
      m_dataOutput->writeArrayLen(arrayLen);
      if (arrayLen > 0) {
        mType* ptr = objArray;
        int i = 0;
        for (i = 0; i < arrayLen; i++) {
          writeObject(*ptr++);
        }
      }
    } else {
      m_dataOutput->write(static_cast<uint8_t>(0xff));
    }
  }

  virtual std::shared_ptr<PdxWriter> writeChar(const std::string& fieldName,
                                               char value) override;

  virtual std::shared_ptr<PdxWriter> writeChar(const std::string& fieldName,
                                               char16_t value) override;

  virtual std::shared_ptr<PdxWriter> writeBoolean(const std::string& fieldName,
                                                  bool value) override;

  virtual std::shared_ptr<PdxWriter> writeByte(const std::string& fieldName,
                                               int8_t value) override;

  virtual std::shared_ptr<PdxWriter> writeShort(const std::string& fieldName,
                                                int16_t value) override;

  virtual std::shared_ptr<PdxWriter> writeInt(const std::string& fieldName,
                                              int32_t value) override;

  virtual std::shared_ptr<PdxWriter> writeLong(const std::string& fieldName,
                                               int64_t value) override;

  virtual std::shared_ptr<PdxWriter> writeFloat(const std::string& fieldName,
                                                float value) override;

  virtual std::shared_ptr<PdxWriter> writeDouble(const std::string& fieldName,
                                                 double value) override;

  virtual std::shared_ptr<PdxWriter> writeDate(
      const std::string& fieldName,
      std::shared_ptr<CacheableDate> date) override;

  virtual std::shared_ptr<PdxWriter> writeString(const std::string& fieldName,
                                                 const char* value) override;

  virtual std::shared_ptr<PdxWriter> writeWideString(
      const std::string& fieldName, const wchar_t* value) override;
  virtual std::shared_ptr<PdxWriter> writeObject(
      const std::string& fieldName,
      std::shared_ptr<Serializable> value) override;

  virtual std::shared_ptr<PdxWriter> writeBooleanArray(
      const std::string& fieldName, bool* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeCharArray(
      const std::string& fieldName, char* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeWideCharArray(
      const std::string& fieldName, wchar_t* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeByteArray(
      const std::string& fieldName, int8_t* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeShortArray(
      const std::string& fieldName, int16_t* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeIntArray(const std::string& fieldName,
                                                   int32_t* array,
                                                   int length) override;

  virtual std::shared_ptr<PdxWriter> writeLongArray(
      const std::string& fieldName, int64_t* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeFloatArray(
      const std::string& fieldName, float* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeDoubleArray(
      const std::string& fieldName, double* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeStringArray(
      const std::string& fieldName, char** array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeWideStringArray(
      const std::string& fieldName, wchar_t** array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> array) override;

  virtual std::shared_ptr<PdxWriter> writeArrayOfByteArrays(
      const std::string& fieldName, int8_t** array, int arrayLength,
      int* elementLength) override;

  virtual std::shared_ptr<PdxWriter> markIdentityField(
      const std::string& fieldName) override;

  virtual std::shared_ptr<PdxWriter> writeUnreadFields(
      std::shared_ptr<PdxUnreadFields> unread) override;

  // this is used to get pdx stream when WriteablePdxStream udpadates the field
  // It should be called after pdx stream has been written to output
  uint8_t* getPdxStream(int& pdxLen);

  void writeByte(int8_t byte);

  inline int32_t getStartPositionOffset() { return m_startPositionOffset; }

 private:
  std::shared_ptr<PdxTypeRegistry> getPdxTypeRegistry() const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXLOCALWRITER_H_
