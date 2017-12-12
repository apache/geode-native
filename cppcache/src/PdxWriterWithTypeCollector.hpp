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

  virtual std::shared_ptr<PdxWriter> writeString(
      const std::string& fieldName, const std::string& value) override;

  virtual std::shared_ptr<PdxWriter> writeObject(
      const std::string& fieldName,
      std::shared_ptr<Serializable> value) override;

  virtual std::shared_ptr<PdxWriter> writeBooleanArray(
      const std::string& fieldName, bool* array, int length) override;

  virtual std::shared_ptr<PdxWriter> writeCharArray(
      const std::string &fieldName, char16_t *array, int length) override;

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
      const std::string& fieldName,
      const std::vector<std::string>& array) override;

  virtual std::shared_ptr<PdxWriter> writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> array) override;

  virtual std::shared_ptr<PdxWriter> writeArrayOfByteArrays(
      const std::string& fieldName, int8_t* const* const byteArrays,
      int arrayLength, const int* elementLength) override;

  virtual std::shared_ptr<PdxWriter> markIdentityField(
      const std::string& fieldName) override;

  virtual std::shared_ptr<PdxWriter> writeUnreadFields(
      std::shared_ptr<PdxUnreadFields> unread) override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXWRITERWITHTYPECOLLECTOR_H_
