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

#ifndef GEODE_PDXINSTANCEFACTORYIMPL_H_
#define GEODE_PDXINSTANCEFACTORYIMPL_H_

#include <map>

#include <geode/PdxInstanceFactory.hpp>
#include <geode/CacheableBuiltins.hpp>

#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "CachePerfStats.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<std::string, std::shared_ptr<Cacheable>> FieldVsValues;

/**
 * PdxInstanceFactory gives you a way to create PdxInstances.
 * Call the write methods to populate the field data and then call {@link
 * #create}
 * to produce an actual instance that contains the data.
 * To create a factory call {@link Cache#createPdxInstanceFactory}
 * A factory can only create a single instance. To create multiple instances
 * create
 * multiple factories or use {@link PdxInstance#createWriter} to create
 * subsequent instances.
 */
class _GEODE_EXPORT PdxInstanceFactoryImpl
    : public PdxInstanceFactory,
      public std::enable_shared_from_this<PdxInstanceFactoryImpl> {
 public:
  /**
   * @brief destructor
   */
  virtual ~PdxInstanceFactoryImpl();

  virtual std::unique_ptr<PdxInstance> create() override;

  virtual std::shared_ptr<PdxInstanceFactory> writeChar(
      const std::string& fieldName, char16_t value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeChar(
      const std::string& fieldName, char value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeBoolean(
      const std::string& fieldName, bool value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeByte(
      const std::string& fieldName, int8_t value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeShort(
      const std::string& fieldName, int16_t value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeInt(
      const std::string& fieldName, int32_t value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeLong(
      const std::string& fieldName, int64_t value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeFloat(
      const std::string& fieldName, float value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeDouble(
      const std::string& fieldName, double value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeDate(
      const std::string& fieldName,
      std::shared_ptr<CacheableDate> value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeString(
      const std::string& fieldName, const std::string& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeString(
      const std::string& fieldName, std::string&& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeObject(
      const std::string& fieldName, std::shared_ptr<Cacheable> value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeBooleanArray(
      const std::string& fieldName, const std::vector<bool>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeCharArray(
      const std::string& fieldName, const std::vector<char16_t>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeByteArray(
      const std::string& fieldName, const std::vector<int8_t>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeShortArray(
      const std::string& fieldName, const std::vector<int16_t>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeIntArray(
      const std::string& fieldName, const std::vector<int32_t>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeLongArray(
      const std::string& fieldName, const std::vector<int64_t>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeFloatArray(
      const std::string& fieldName, const std::vector<float>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeDoubleArray(
      const std::string& fieldName, const std::vector<double>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeStringArray(
      const std::string& fieldName,
      const std::vector<std::string>& value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> value) override;

  virtual std::shared_ptr<PdxInstanceFactory> writeArrayOfByteArrays(
      const std::string& fieldName, int8_t** value, int32_t arrayLength,
      int32_t* elementLength) override;

  virtual std::shared_ptr<PdxInstanceFactory> markIdentityField(
      const std::string& fieldName) override;

  PdxInstanceFactoryImpl(std::string className, CachePerfStats* cachePerfStats,
                         std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry,
                         const Cache* cache, bool enableTimeStatistics);

 private:
  bool m_created;
  std::shared_ptr<PdxType> m_pdxType;
  FieldVsValues m_FieldVsValues;
  CachePerfStats* m_cachePerfStats;
  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;
  const Cache* m_cache;
  bool m_enableTimeStatistics;
  void isFieldAdded(const std::string& fieldName);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXINSTANCEFACTORYIMPL_H_
