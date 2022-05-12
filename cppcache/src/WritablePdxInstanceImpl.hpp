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

#ifndef GEODE_WRITABLEPDXINSTANCEIMPL_H_
#define GEODE_WRITABLEPDXINSTANCEIMPL_H_

#include <mutex>

#include <geode/WritablePdxInstance.hpp>

#include "PdxInstanceImpl.hpp"

namespace apache {
namespace geode {
namespace client {

class CachePerfStats;
class DataOutput;
class PdxType;
class PdxWriterImpl;

class WritablePdxInstanceImpl : virtual public WritablePdxInstance,
                                virtual public PdxInstanceImpl {
 public:
  WritablePdxInstanceImpl(Fields fields,
                          FieldsBuffer buffer,
                          std::shared_ptr<PdxType> pdxType,
                          const CacheImpl& cacheImpl);

  WritablePdxInstanceImpl(const PdxInstanceImpl& other) = delete;

  void operator=(const WritablePdxInstanceImpl& other) = delete;

  ~WritablePdxInstanceImpl() noexcept override;

  virtual size_t objectSize() const override;

  virtual void setField(const std::string& fieldName, bool value) override;

  virtual void setField(const std::string& fieldName,
                        signed char value) override;

  virtual void setField(const std::string& fieldName,
                        unsigned char value) override;

  virtual void setField(const std::string& fieldName, int16_t value) override;

  virtual void setField(const std::string& fieldName, int32_t value) override;

  virtual void setField(const std::string& fieldName, int64_t value) override;

  virtual void setField(const std::string& fieldName, float value) override;

  virtual void setField(const std::string& fieldName, double value) override;

  virtual void setField(const std::string& fieldName, char16_t value) override;

  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<CacheableDate> value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<bool>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int8_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int16_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int32_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int64_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<float>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<double>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::string& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<char16_t>& value) override;

  virtual void setField(const std::string& fieldName, std::string* value,
                        int32_t length) override;

  virtual void setField(const std::string& fieldName, int8_t** value,
                        int32_t arrayLength, int32_t* elementLength) override;

  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<Cacheable> value) override;

  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<CacheableObjectArray> value) override;

 private:
  void updateFieldValue(std::shared_ptr<PdxFieldType> field,
                        std::shared_ptr<Cacheable> value);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_WRITABLEPDXINSTANCEIMPL_H_
