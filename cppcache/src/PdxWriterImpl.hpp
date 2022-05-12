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

#ifndef GEODE_PDXWRITERIMPL_H_
#define GEODE_PDXWRITERIMPL_H_

#include <vector>

#include <geode/PdxFieldTypes.hpp>
#include <geode/PdxWriter.hpp>

namespace apache {
namespace geode {
namespace client {

class PdxFieldType;
class PdxType;
class PdxUnreadData;
class PdxTypeRegistry;

class PdxWriterImpl : public PdxWriter {
 public:
  PdxWriterImpl() = default;

  explicit PdxWriterImpl(DataOutput& output);

  PdxWriterImpl(std::shared_ptr<PdxType> pdxType, DataOutput& output);

  PdxWriterImpl(PdxWriterImpl&& move) = default;

  ~PdxWriterImpl() override = default;

  void initialize();

  virtual PdxWriter& writeChar(const std::string& name,
                               char16_t value) override;

  virtual PdxWriter& writeBoolean(const std::string& name, bool value) override;

  virtual PdxWriter& writeByte(const std::string& name, int8_t value) override;

  virtual PdxWriter& writeShort(const std::string& name,
                                int16_t value) override;

  virtual PdxWriter& writeInt(const std::string& name, int32_t value) override;

  virtual PdxWriter& writeLong(const std::string& name, int64_t value) override;

  virtual PdxWriter& writeFloat(const std::string& name, float value) override;

  virtual PdxWriter& writeDouble(const std::string& name,
                                 double value) override;

  virtual PdxWriter& writeDate(const std::string& name,
                               std::shared_ptr<CacheableDate> date) override;

  virtual PdxWriter& writeString(const std::string& name,
                                 const std::string& value) override;

  virtual PdxWriter& writeObject(const std::string& name,
                                 std::shared_ptr<Serializable> value) override;

  virtual PdxWriter& writeBooleanArray(const std::string& name,
                                       const std::vector<bool>& array) override;

  virtual PdxWriter& writeCharArray(
      const std::string& fieldName,
      const std::vector<char16_t>& array) override;

  virtual PdxWriter& writeByteArray(const std::string& name,
                                    const std::vector<int8_t>& array) override;

  virtual PdxWriter& writeShortArray(
      const std::string& fieldName, const std::vector<int16_t>& array) override;

  virtual PdxWriter& writeIntArray(const std::string& fieldName,
                                   const std::vector<int32_t>& array) override;

  virtual PdxWriter& writeLongArray(const std::string& fieldName,
                                    const std::vector<int64_t>& array) override;

  virtual PdxWriter& writeFloatArray(const std::string& fieldName,
                                     const std::vector<float>& array) override;

  virtual PdxWriter& writeDoubleArray(
      const std::string& fieldName, const std::vector<double>& array) override;

  virtual PdxWriter& writeStringArray(
      const std::string& fieldName,
      const std::vector<std::string>& array) override;

  virtual PdxWriter& writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> array) override;

  virtual PdxWriter& writeArrayOfByteArrays(const std::string& fieldName,
                                            int8_t* const* const array,
                                            int arrayLength,
                                            const int* elementLength) override;

  virtual PdxWriter& markIdentityField(const std::string& fieldName) override;

  virtual PdxWriter& writeUnreadFields(
      std::shared_ptr<PdxUnreadFields> unread) override;

  virtual std::shared_ptr<PdxSerializer> getPdxSerializer() const override;

  void writeChar(char16_t value);

  void writeBoolean(bool value);

  void writeByte(int8_t value);

  void writeShort(int16_t value);

  void writeInt(int32_t value);

  void writeLong(int64_t value);

  void writeFloat(float value);

  void writeDouble(double value);

  void writeDate(std::shared_ptr<CacheableDate> date);

  void writeString(const std::string& value);

  void writeObject(std::shared_ptr<Serializable> value);

  void writeBooleanArray(const std::vector<bool>& array);

  void writeCharArray(const std::vector<char16_t>& array);

  void writeByteArray(const std::vector<int8_t>& array);

  void writeShortArray(const std::vector<int16_t>& array);

  void writeIntArray(const std::vector<int32_t>& array);

  void writeLongArray(const std::vector<int64_t>& array);

  void writeFloatArray(const std::vector<float>& array);

  void writeDoubleArray(const std::vector<double>& array);

  void writeStringArray(const std::vector<std::string>& array);

  void writeObjectArray(std::shared_ptr<CacheableObjectArray> array);

  void writeArrayOfByteArrays(int8_t* const* const array, int arrayLength,
                              const int* elementLength);

  void writeRawField(std::shared_ptr<PdxFieldType> field,
                     const std::vector<int8_t>& data);

  void setUnreadData(std::shared_ptr<PdxUnreadData> data);


  void completeSerialization();

  std::vector<int8_t> getFieldsBuffer() const;

 private:
  void writeOffsets(int32_t len);

  std::shared_ptr<PdxFieldType> addPdxField(const std::string& name,
                                            PdxFieldTypes type);

  void writeObject(bool value) { dataOutput_->writeBoolean(value); }

  void writeObject(char16_t value) {
    dataOutput_->writeInt(static_cast<uint16_t>(value));
  }

  void writeObject(int8_t value) { dataOutput_->write(value); }

  void writeObject(int16_t value) { dataOutput_->writeInt(value); }

  void writeObject(int32_t value) { dataOutput_->writeInt(value); }

  void writeObject(int64_t value) { dataOutput_->writeInt(value); }

  void writeObject(float value) { dataOutput_->writeFloat(value); }

  void writeObject(double value) { dataOutput_->writeDouble(value); }

  template <typename T>
  void writeObject(T* array, int arrayLen) {
    if (array != nullptr) {
      dataOutput_->writeArrayLen(arrayLen);
      if (arrayLen > 0) {
        auto ptr = array;
        int i = 0;
        for (i = 0; i < arrayLen; i++) {
          writeObject(*ptr++);
        }
      }
    } else {
      dataOutput_->write(static_cast<uint8_t>(0xff));
    }
  }

  template <typename T>
  void writeArrayObject(const std::vector<T>& array) {
    dataOutput_->writeArrayLen(static_cast<int32_t>(array.size()));
    for (auto&& obj : array) {
      writeObject(obj);
    }
  }

  void addOffset();

  int32_t getTotalLength() const;

  bool hasWrittenFields() const;

 private:
  DataOutput* dataOutput_;
  std::shared_ptr<PdxType> pdxType_;
  std::shared_ptr<PdxUnreadData> unreadData_;

  int32_t startPosition_;
  std::vector<int32_t> offsets_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXWRITERIMPL_H_
