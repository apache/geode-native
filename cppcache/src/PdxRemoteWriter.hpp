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

#ifndef GEODE_PDXREMOTEWRITER_H_
#define GEODE_PDXREMOTEWRITER_H_

#include "PdxLocalWriter.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxRemoteWriter : public PdxLocalWriter {
 private:
  int32_t* m_remoteTolocalMap;
  int32_t m_preserveDataIdx;
  int32_t m_currentDataIdx;

  int32_t m_remoteTolocalMapLength;

  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;

  void initialize();
  void writePreserveData();

  std::shared_ptr<PdxTypeRegistry> getPdxTypeRegistry() const;

 public:
  virtual ~PdxRemoteWriter();

  PdxRemoteWriter(DataOutput& output, std::shared_ptr<PdxType> pdxType,
                  std::shared_ptr<PdxRemotePreservedData> preservedData,
                  std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  PdxRemoteWriter(DataOutput& output, const char* pdxClassName,
                  std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  virtual void endObjectWriting();

  virtual bool isFieldWritingStarted();

  virtual std::shared_ptr<PdxWriter> writeUnreadFields(std::shared_ptr<PdxUnreadFields> unread);

  /**
   *Write a wide char to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The wide char value to write
   */
  virtual std::shared_ptr<PdxWriter> writeChar(const char* fieldName, char value);

  virtual std::shared_ptr<PdxWriter> writeChar(const char* fieldName, char16_t value);

  /**
   *Write a boolean value to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The boolean value to write
   */
  virtual std::shared_ptr<PdxWriter> writeBoolean(const char* fieldName, bool value);

  /**
   *Write a 8-bit integer or byte to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 8-bit integer or byte to write
   */
  virtual std::shared_ptr<PdxWriter> writeByte(const char* fieldName, int8_t value);

  /**
   *Write a 16-bit integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 16-bit integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeShort(const char* fieldName, int16_t value);

  /**
   *Write a 32-bit integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The 32-bit integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeInt(const char* fieldName, int32_t value);

  /**
   *Write a long integer to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The long integer to write
   */
  virtual std::shared_ptr<PdxWriter> writeLong(const char* fieldName, int64_t value);

  /**
   *Write a Float to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The float value to write
   */
  virtual std::shared_ptr<PdxWriter> writeFloat(const char* fieldName, float value);

  /**
   *Write a Double to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The double value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDouble(const char* fieldName, double value);

  /**
   *Write a Date to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The date value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDate(const char* fieldName, std::shared_ptr<CacheableDate> date);

  /**
   *Write a string to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The string to write
   */
  virtual std::shared_ptr<PdxWriter> writeString(const char* fieldName, const char* value);

  virtual std::shared_ptr<PdxWriter> writeWideString(const char* fieldName,
                                       const wchar_t* value);
  /**
   *Write a object to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The object to write
   */
  virtual std::shared_ptr<PdxWriter> writeObject(const char* fieldName,
                                   std::shared_ptr<Serializable> value);

  /**
   *Write a boolean array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The boolean array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeBooleanArray(const char* fieldName, bool* array,
                                         int length);

  /**
   *Write a Char array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The char array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeCharArray(const char* fieldName, char* array,
                                      int length);

  virtual std::shared_ptr<PdxWriter> writeWideCharArray(const char* fieldName, wchar_t* array,
                                          int length);

  /**
   *Write a Byte array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The byte array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeByteArray(const char* fieldName, int8_t* array,
                                      int length);

  /**
   *Write a 16-bit integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeShortArray(const char* fieldName, int16_t* array,
                                       int length);

  /**
   *Write a 32-bit integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The integer array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeIntArray(const char* fieldName, int32_t* array,
                                     int length);

  /**
   *Write a long integer array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The long integer array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeLongArray(const char* fieldName, int64_t* array,
                                      int length);

  /**
   *Write a Float array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The float array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeFloatArray(const char* fieldName, float* array,
                                       int length);

  /**
   *Write a double array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The double array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeDoubleArray(const char* fieldName, double* array,
                                        int length);

  /**
   *Write a string array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The string array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeStringArray(const char* fieldName, char** array,
                                        int length);

  virtual std::shared_ptr<PdxWriter> writeWideStringArray(const char* fieldName,
                                            wchar_t** array, int length);

  /**
   *Write a object array to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param value The object array value to write
   */
  virtual std::shared_ptr<PdxWriter> writeObjectArray(const char* fieldName,
                                        std::shared_ptr<CacheableObjectArray> array);

  /**
   *Write a array of byte arrays to the PdxWriter.
   *@param fieldName The name of the field associated with the value.
   *@param array The arrayOfbytearray value to write
   */
  virtual std::shared_ptr<PdxWriter> writeArrayOfByteArrays(const char* fieldName,
                                              int8_t** array, int arrayLength,
                                              int* elementLength);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREMOTEWRITER_H_
