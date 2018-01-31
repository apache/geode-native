#pragma once

#ifndef GEODE_PDXLOCALREADER_H_
#define GEODE_PDXLOCALREADER_H_

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

#include <geode/PdxReader.hpp>
#include "PdxType.hpp"
#include <geode/DataInput.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/CacheableDate.hpp>
#include "PdxRemotePreservedData.hpp"
namespace apache {
namespace geode {
namespace client {

class PdxLocalReader : public PdxReader {
 protected:
  DataInput* m_dataInput;
  std::shared_ptr<PdxType> m_pdxType;
  uint8_t* m_startBuffer;
  int32_t m_startPosition;
  int32_t m_serializedLength;
  int32_t m_serializedLengthWithOffsets;
  int32_t m_offsetSize;
  uint8_t* m_offsetsBuffer;
  bool m_isDataNeedToPreserve;
  std::shared_ptr<PdxRemotePreservedData> m_pdxRemotePreserveData;
  int32_t* m_localToRemoteMap;
  int32_t* m_remoteToLocalMap;
  int32_t m_remoteToLocalMapSize;

  void initialize();
  void resettoPdxHead();

 public:
  PdxLocalReader(std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  PdxLocalReader(DataInput& input, std::shared_ptr<PdxType> remoteType,
                 int32_t pdxLen,
                 std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  virtual ~PdxLocalReader();

  void moveStream();

  virtual std::shared_ptr<PdxRemotePreservedData> getPreservedData(
      std::shared_ptr<PdxType> mergedVersion,
      std::shared_ptr<PdxSerializable> pdxObject);

  virtual char16_t readChar(const std::string &fieldName) override;

  virtual bool readBoolean(const std::string &fieldName) override;

  virtual int8_t readByte(const std::string &fieldName) override;

  virtual int16_t readShort(const std::string &fieldName) override;

  virtual int32_t readInt(const std::string &fieldName) override;

  virtual int64_t readLong(const std::string &fieldName) override;

  virtual float readFloat(const std::string &fieldName) override;

  virtual double readDouble(const std::string &fieldName) override;

  virtual std::string readString(const std::string &fieldName) override;

  virtual std::shared_ptr<Serializable> readObject(
      const std::string &fieldName) override;

  virtual std::vector<char16_t> readCharArray(
      const std::string &fieldName) override;

  virtual std::vector<bool> readBooleanArray(
      const std::string &fieldName) override;

  virtual std::vector<int8_t> readByteArray(
      const std::string &fieldName) override;

  virtual std::vector<int16_t> readShortArray(
      const std::string &fieldName) override;

  virtual std::vector<int32_t> readIntArray(
      const std::string &fieldName) override;

  virtual std::vector<int64_t> readLongArray(
      const std::string &fieldName) override;

  virtual std::vector<float> readFloatArray(
      const std::string &fieldName) override;

  virtual std::vector<double> readDoubleArray(
      const std::string &fieldName) override;

  virtual std::vector<std::string> readStringArray(
      const std::string &fieldName) override;

  virtual std::shared_ptr<CacheableObjectArray> readObjectArray(
      const std::string &fieldName) override;

  virtual int8_t **readArrayOfByteArrays(const std::string &fieldName,
                                         int32_t &arrayLength,
                                         int32_t **elementLength) override;

  virtual std::shared_ptr<CacheableDate> readDate(
      const std::string &fieldName) override;

  virtual bool hasField(const std::string &fieldName) override;

  virtual bool isIdentityField(const std::string &fieldName) override;

  virtual void readCollection(const std::string &fieldName,
                              std::shared_ptr<CacheableArrayList> &collection);

  virtual std::shared_ptr<PdxUnreadFields> readUnreadFields() override;

 protected:
  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXLOCALREADER_H_
