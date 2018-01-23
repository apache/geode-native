#pragma once

#ifndef GEODE_PDXREADERWITHTYPECOLLECTOR_H_
#define GEODE_PDXREADERWITHTYPECOLLECTOR_H_

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

#include "PdxLocalReader.hpp"
#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxReaderWithTypeCollector : public PdxLocalReader {
 private:
  std::shared_ptr<PdxType> m_newPdxType;

  void checkType(const std::string& fieldName, int8_t typeId,
                 const std::string& fieldType);

 public:
  PdxReaderWithTypeCollector(DataInput& dataInput,
                             std::shared_ptr<PdxType> pdxType, int pdxlen,
                             std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  virtual ~PdxReaderWithTypeCollector();

  std::shared_ptr<PdxType> getLocalType() const { return m_newPdxType; }

  virtual char16_t readChar(const std::string& fieldName) override;

  virtual bool readBoolean(const std::string& fieldName) override;

  virtual int8_t readByte(const std::string& fieldName) override;

  virtual int16_t readShort(const std::string& fieldName) override;

  virtual int32_t readInt(const std::string& fieldName) override;

  virtual int64_t readLong(const std::string& fieldName) override;

  virtual float readFloat(const std::string& fieldName) override;

  virtual double readDouble(const std::string& fieldName) override;

  virtual std::string readString(const std::string& fieldName) override;

  virtual std::shared_ptr<Serializable> readObject(
      const std::string& fieldName) override;

  virtual std::vector<char16_t> readCharArray(
      const std::string& fieldName) override;

  virtual std::vector<bool> readBooleanArray(
      const std::string& fieldName) override;

  virtual std::vector<int8_t> readByteArray(
      const std::string& fieldName) override;

  virtual std::vector<int16_t> readShortArray(
      const std::string& fieldName) override;

  virtual std::vector<int32_t> readIntArray(
      const std::string& fieldName) override;

  virtual std::vector<int64_t> readLongArray(
      const std::string& fieldName) override;

  virtual std::vector<float> readFloatArray(
      const std::string& fieldName) override;

  virtual std::vector<double> readDoubleArray(
      const std::string& fieldName) override;

  virtual std::vector<std::string> readStringArray(
      const std::string& fieldName) override;

  virtual std::shared_ptr<CacheableObjectArray> readObjectArray(
      const std::string& fieldName) override;

  virtual int8_t** readArrayOfByteArrays(const std::string& fieldName,
                                         int32_t& arrayLength,
                                         int32_t** elementLength) override;

  virtual std::shared_ptr<CacheableDate> readDate(
      const std::string& fieldName) override;

  virtual void readCollection(
      const std::string& fieldName,
      std::shared_ptr<CacheableArrayList>& collection) override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREADERWITHTYPECOLLECTOR_H_
