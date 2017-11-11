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
  const char* m_domainClassName;
  std::vector<int32_t> m_offsets;
  int32_t m_currentOffsetIndex;

  std::shared_ptr<PdxRemotePreservedData> m_preserveData;
  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;
  const char* m_pdxClassName;

  std::shared_ptr<PdxWriter> writeStringwithoutOffset(const char* value);

  std::shared_ptr<PdxWriter> writeWideStringwithoutOffset(const wchar_t* value);

 public:
  PdxLocalWriter(DataOutput& output, std::shared_ptr<PdxType> pdxType,
                 std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  PdxLocalWriter(DataOutput& output, std::shared_ptr<PdxType> pdxType,
                 const char* pdxDomainType,
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

  /**
   *Write a wide char to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The wide char value to write
   */
  virtual std::shared_ptr<PdxWriter> writeChar(const char* fieldName,
                                               char value);

  virtual std::shared_ptr<PdxWriter> writeChar(const char* fieldName,
                                               char16_t value);

  /**
   *Write a boolean value to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The boolean value to write
   */
  virtual std::shared_ptr<PdxWriter> writeBoolean(const char* fieldName,
                                                  bool value);

  /**
   *Write a 8-bit integer or byte to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 8-bit integer or byte to write
   */
  virtual std::shared_ptr<PdxWriter> writeByte(const char* fieldName,
                                               int8_t value);

  /**
   *Write a 16-bit integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 16-bit integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeShort(const char* fieldName,
                                                int16_t value);

  /**
   *Write a 32-bit integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 32-bit integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeInt(const char* fieldName,
                                              int32_t value);

  /**
   *Write a long integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The long integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeLong(const char* fieldName,
                                               int64_t value);

  /**
   *Write a Float to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The float value to write
   */
  virtual std::shared_ptr<PdxWriter> writeFloat(const char* fieldName,
                                                float value);

  /**
   *Write a Double to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The double value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDouble(const char* fieldName,
                                                 double value);

  /**
   *Write a Date to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The date value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDate(
      const char* fieldName, std::shared_ptr<CacheableDate> date);

  /**
   *Write a string to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The string to write
   */
  virtual std::shared_ptr<PdxWriter> writeString(const char* fieldName,
                                                 const char* value);

  virtual std::shared_ptr<PdxWriter> writeWideString(const char* fieldName,
                                                     const wchar_t* value);
  /**
   *Write a object to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The object to write
   */
  virtual std::shared_ptr<PdxWriter> writeObject(
      const char* fieldName, std::shared_ptr<Serializable> value);

  /**
   *Write a boolean array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The boolean array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeBooleanArray(const char* fieldName,
                                                       bool* array, int length);

  /**
   *Write a Char array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The char array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeCharArray(const char* fieldName,
                                                    char* array, int length);

  virtual std::shared_ptr<PdxWriter> writeWideCharArray(const char* fieldName,
                                                        wchar_t* array,
                                                        int length);
  /**
   *Write a Byte array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The byte array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeByteArray(const char* fieldName,
                                                    int8_t* array, int length);

  /**
   *Write a 16-bit integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeShortArray(const char* fieldName,
                                                     int16_t* array,
                                                     int length);

  /**
   *Write a 32-bit integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The integer array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeIntArray(const char* fieldName,
                                                   int32_t* array, int length);

  /**
   *Write a long integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The long integer array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeLongArray(const char* fieldName,
                                                    int64_t* array, int length);

  /**
   *Write a Float array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The float array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeFloatArray(const char* fieldName,
                                                     float* array, int length);

  /**
   *Write a double array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The double array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDoubleArray(const char* fieldName,
                                                      double* array,
                                                      int length);

  /**
   *Write a string array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The string array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeStringArray(const char* fieldName,
                                                      char** array, int length);

  virtual std::shared_ptr<PdxWriter> writeWideStringArray(const char* fieldName,
                                                          wchar_t** array,
                                                          int length);
  /**
   *Write a object array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The object array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeObjectArray(
      const char* fieldName, std::shared_ptr<CacheableObjectArray> array);

  /**
   *Write a array of byte arrays to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param array The arrayOfbytearray value to write
   */
  virtual std::shared_ptr<PdxWriter> writeArrayOfByteArrays(
      const char* fieldName, int8_t** array, int arrayLength,
      int* elementLength);

  /**
   *Indicate that the given field name should be included in hashCode and equals
   *checks
   *of this object on a server that is using {@link
   *CacheFactory#setPdxReadSerialized(boolean)} or when a client executes a
   *query on a server.
   *The fields that are marked as identity fields are used to generate the
   *hashCode and
   *equals methods of {@link PdxInstance}. Because of this, the identity fields
   *should themselves
   *either be primatives, or implement hashCode and equals.
   *
   *If no fields are set as identity fields, then all fields will be used in
   *hashCode and equal checks.
   *
   *The identity fields should make marked after they are written using a write*
   *method.
   *
   *@param fieldName The name of the field that should be used in the as part of
   *the identity.
   *@eturns this std::shared_ptr<PdxWriter>
   */
  virtual std::shared_ptr<PdxWriter> markIdentityField(const char* fieldName);

  virtual std::shared_ptr<PdxWriter> writeUnreadFields(
      std::shared_ptr<PdxUnreadFields> unread);

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
