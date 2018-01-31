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
  virtual ~PdxRemoteWriter() override;

  PdxRemoteWriter() = default;

  PdxRemoteWriter(DataOutput& output, std::shared_ptr<PdxType> pdxType,
                  std::shared_ptr<PdxRemotePreservedData> preservedData,
                  std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  PdxRemoteWriter(DataOutput& output, std::string pdxClassName,
                  std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  PdxRemoteWriter(PdxRemoteWriter&& move) = default;

  virtual void endObjectWriting() override;

  virtual bool isFieldWritingStarted() override;

  virtual PdxWriter& writeUnreadFields(
      std::shared_ptr<PdxUnreadFields> unread) override;


  virtual PdxWriter& writeChar(const std::string& fieldName,
                               char16_t value) override;

  virtual PdxWriter& writeBoolean(const std::string& fieldName,
                                                  bool value) override;

  virtual PdxWriter& writeByte(const std::string& fieldName,
                                               int8_t value) override;

  virtual PdxWriter& writeShort(const std::string& fieldName,
                                                int16_t value) override;

  virtual PdxWriter& writeInt(const std::string& fieldName,
                                              int32_t value) override;

  virtual PdxWriter& writeLong(const std::string& fieldName,
                                               int64_t value) override;

  virtual PdxWriter& writeFloat(const std::string& fieldName,
                                                float value) override;

  virtual PdxWriter& writeDouble(const std::string& fieldName,
                                                 double value) override;

  virtual PdxWriter& writeDate(
      const std::string& fieldName,
      std::shared_ptr<CacheableDate> date) override;

  virtual PdxWriter& writeString(
      const std::string& fieldName, const std::string& value) override;

  virtual PdxWriter& writeObject(
      const std::string& fieldName,
      std::shared_ptr<Serializable> value) override;

  virtual PdxWriter& writeBooleanArray(
      const std::string& fieldName,
      const std::vector<bool>& array) override;

  virtual PdxWriter& writeCharArray(
      const std::string& fieldName,
      const std::vector<char16_t>& array) override;

  virtual PdxWriter& writeByteArray(
      const std::string& fieldName,
      const std::vector<int8_t>& array) override;

  virtual PdxWriter& writeShortArray(
      const std::string& fieldName,
      const std::vector<int16_t>& array) override;

  virtual PdxWriter& writeIntArray(
      const std::string& fieldName,
      const std::vector<int32_t>& array) override;

  virtual PdxWriter& writeLongArray(
      const std::string& fieldName,
      const std::vector<int64_t>& array) override;

  virtual PdxWriter& writeFloatArray(
      const std::string& fieldName,
      const std::vector<float>& array) override;

  virtual PdxWriter& writeDoubleArray(
      const std::string& fieldName,
      const std::vector<double>& array) override;

  virtual PdxWriter& writeStringArray(
      const std::string& fieldName,
      const std::vector<std::string>& array) override;

  virtual PdxWriter& writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> array) override;

  virtual PdxWriter& writeArrayOfByteArrays(
      const std::string& fieldName, int8_t* const* const array, int arrayLength,
      const int* elementLength) override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREMOTEWRITER_H_
