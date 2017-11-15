#pragma once

#ifndef GEODE_PDXWRITERWITHTYPECOLLECTOR_H_
#define GEODE_PDXWRITERWITHTYPECOLLECTOR_H_

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

#include "PdxLocalWriter.hpp"
//#include <map>

namespace apache {
namespace geode {
namespace client {

class PdxWriterWithTypeCollector : public PdxLocalWriter {
 private:
  std::vector<int32_t> m_offsets;
  void initialize();

 public:
  PdxWriterWithTypeCollector(DataOutput& output, std::string pdxType,
                             std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  virtual ~PdxWriterWithTypeCollector();

  std::shared_ptr<PdxType> getPdxLocalType() { return m_pdxType; }
  virtual void endObjectWriting() override;

  virtual void addOffset() override;

  virtual bool isFieldWritingStarted() override;

  virtual int32_t calculateLenWithOffsets() override;

  virtual void writeOffsets(int32_t len) override;

  /**
   *Write a 8-bit integer or byte to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 8-bit integer or byte to write
   */
  virtual std::shared_ptr<PdxWriter> writeChar(const std::string& fieldName,
                                               char value) override;

  virtual std::shared_ptr<PdxWriter> writeChar(const std::string& fieldName,
                                               char16_t value) override;

  /**
   *Write a boolean value to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The boolean value to write
   */
  virtual std::shared_ptr<PdxWriter> writeBoolean(const std::string& fieldName,
                                                  bool value) override;

  /**
   *Write a Byte value to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The Byte value to write
   */
  virtual std::shared_ptr<PdxWriter> writeByte(const std::string& fieldName,
                                               int8_t value) override;

  /**
   *Write a 16-bit integer or Short value to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 16-bit integer or short to write
   */
  virtual std::shared_ptr<PdxWriter> writeShort(const std::string& fieldName,
                                                int16_t value) override;

  /**
   *Write a 32-bit integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 32-bit integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeInt(const std::string& fieldName,
                                              int32_t value) override;

  /**
   *Write a long integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The long integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeLong(const std::string& fieldName,
                                               int64_t value) override;

  /**
   *Write a Float to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The float value to write
   */
  virtual std::shared_ptr<PdxWriter> writeFloat(const std::string& fieldName,
                                                float value) override;

  /**
   *Write a Double to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The double value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDouble(const std::string& fieldName,
                                                 double value) override;

  /**
   *Write a Date to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The date value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDate(
      const std::string& fieldName,
      std::shared_ptr<CacheableDate> date) override;

  /**
   *Write a string to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The string to write
   */
  virtual std::shared_ptr<PdxWriter> writeString(const std::string& fieldName,
                                                 const char* value) override;

  virtual std::shared_ptr<PdxWriter> writeWideString(
      const std::string& fieldName, const wchar_t* value) override;

  /**
   *Write a object to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The object to write
   */
  virtual std::shared_ptr<PdxWriter> writeObject(
      const std::string& fieldName,
      std::shared_ptr<Serializable> value) override;

  /**
   *Write a boolean array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The boolean array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeBooleanArray(
      const std::string& fieldName, bool* array, int length) override;

  /**
   *Write a Char array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The char array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeCharArray(
      const std::string& fieldName, char* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeWideCharArray(
      const std::string& fieldName, wchar_t* array, int length) override;

  /**
   *Write a Byte array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The byte array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeByteArray(
      const std::string& fieldName, int8_t* array, int length) override;

  /**
   *Write a 16-bit integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeShortArray(
      const std::string& fieldName, int16_t* array, int length) override;

  /**
   *Write a 32-bit integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The integer array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeIntArray(const std::string& fieldName,
                                                   int32_t* array,
                                                   int length) override;

  /**
   *Write a long integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The long integer array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeLongArray(
      const std::string& fieldName, int64_t* array, int length) override;

  /**
   *Write a Float array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The float array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeFloatArray(
      const std::string& fieldName, float* array, int length) override;

  /**
   *Write a double array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The double array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDoubleArray(
      const std::string& fieldName, double* array, int length) override;

  /**
   *Write a string array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The string array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeStringArray(
      const std::string& fieldName, char** array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeWideStringArray(
      const std::string& fieldName, wchar_t** array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> array) override;

  virtual std::shared_ptr<PdxWriter> writeArrayOfByteArrays(
      const std::string& fieldName, int8_t** byteArrays, int arrayLength,
      int* elementLength) override;

  virtual std::shared_ptr<PdxWriter> markIdentityField(
      const std::string& fieldName) override;

  virtual std::shared_ptr<PdxWriter> writeUnreadFields(
      std::shared_ptr<PdxUnreadFields> unread) override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXWRITERWITHTYPECOLLECTOR_H_
