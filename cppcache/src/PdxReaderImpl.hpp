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

#ifndef GEODE_PDXREADERIMPL_H_
#define GEODE_PDXREADERIMPL_H_

#include <geode/PdxReader.hpp>

namespace apache {
namespace geode {
namespace client {

class CacheableDate;
class CacheableObjectArray;
class DataInput;
class PdxType;
class PdxFieldType;

class PdxReaderImpl : public PdxReader {
 public:
  PdxReaderImpl();

  PdxReaderImpl(DataInput &input, std::shared_ptr<PdxType> remoteType,
                int32_t pdxLen);

  virtual ~PdxReaderImpl() override;

  void moveInputToEnd();

  virtual char16_t readChar(const std::string &name) override;

  virtual bool readBoolean(const std::string &name) override;

  virtual int8_t readByte(const std::string &name) override;

  virtual int16_t readShort(const std::string &name) override;

  virtual int32_t readInt(const std::string &name) override;

  virtual int64_t readLong(const std::string &name) override;

  virtual float readFloat(const std::string &name) override;

  virtual double readDouble(const std::string &name) override;

  virtual std::shared_ptr<CacheableDate> readDate(
      const std::string &name) override;

  virtual std::string readString(const std::string &name) override;

  virtual std::shared_ptr<Serializable> readObject(
      const std::string &name) override;

  virtual std::vector<char16_t> readCharArray(const std::string &name) override;

  virtual std::vector<bool> readBooleanArray(const std::string &name) override;

  virtual std::vector<int8_t> readByteArray(const std::string &name) override;

  virtual std::vector<int16_t> readShortArray(const std::string &name) override;

  virtual std::vector<int32_t> readIntArray(const std::string &name) override;

  virtual std::vector<int64_t> readLongArray(const std::string &name) override;

  virtual std::vector<float> readFloatArray(const std::string &name) override;

  virtual std::vector<double> readDoubleArray(const std::string &name) override;

  virtual std::vector<std::string> readStringArray(
      const std::string &name) override;

  virtual std::shared_ptr<CacheableObjectArray> readObjectArray(
      const std::string &name) override;

  virtual int8_t **readArrayOfByteArrays(const std::string &name,
                                         int32_t &arrayLength,
                                         int32_t **elementLength) override;

  virtual bool hasField(const std::string &name) override;

  virtual bool isIdentityField(const std::string &name) override;

  virtual std::shared_ptr<PdxUnreadFields> readUnreadFields() override {
    return std::shared_ptr<PdxUnreadFields>{};
  }

  std::shared_ptr<PdxSerializer> getPdxSerializer() const override;

  char16_t readChar(std::shared_ptr<PdxFieldType> field);

  bool readBoolean(std::shared_ptr<PdxFieldType> field);

  int8_t readByte(std::shared_ptr<PdxFieldType> field);

  int16_t readShort(std::shared_ptr<PdxFieldType> field);

  int32_t readInt(std::shared_ptr<PdxFieldType> field);

  int64_t readLong(std::shared_ptr<PdxFieldType> field);

  float readFloat(std::shared_ptr<PdxFieldType> field);

  double readDouble(std::shared_ptr<PdxFieldType> field);

  std::shared_ptr<CacheableDate> readDate(std::shared_ptr<PdxFieldType> field);

  std::string readString(std::shared_ptr<PdxFieldType> field);

  std::shared_ptr<Serializable> readObject(std::shared_ptr<PdxFieldType> field);

  std::vector<char16_t> readCharArray(std::shared_ptr<PdxFieldType> field);

  std::vector<bool> readBooleanArray(std::shared_ptr<PdxFieldType> field);

  std::vector<int8_t> readByteArray(std::shared_ptr<PdxFieldType> field);

  std::vector<int16_t> readShortArray(std::shared_ptr<PdxFieldType> field);

  std::vector<int32_t> readIntArray(std::shared_ptr<PdxFieldType> field);

  std::vector<int64_t> readLongArray(std::shared_ptr<PdxFieldType> field);

  std::vector<float> readFloatArray(std::shared_ptr<PdxFieldType> field);

  std::vector<double> readDoubleArray(std::shared_ptr<PdxFieldType> field);

  std::vector<std::string> readStringArray(std::shared_ptr<PdxFieldType> field);

  std::shared_ptr<CacheableObjectArray> readObjectArray(
      std::shared_ptr<PdxFieldType> field);

  int8_t **readArrayOfByteArrays(std::shared_ptr<PdxFieldType> field,
                                 int32_t &arrayLength, int32_t **elementLength);

  std::vector<uint8_t> getRawFieldData(int32_t idx) const;

 private:
  void initialize();
  void moveInputToField(std::shared_ptr<PdxFieldType> field);

 protected:
  std::shared_ptr<PdxType> pdxType_;

 private:
  DataInput *dataInput_;
  int32_t startPosition_;
  int32_t length_;
  int32_t lengthWithOffsets_;
  int32_t offsetSize_;
  uint8_t *offsets_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREADERIMPL_H_
