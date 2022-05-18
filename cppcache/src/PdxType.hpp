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

#ifndef GEODE_PDXTYPE_H_
#define GEODE_PDXTYPE_H_

#include <list>
#include <map>
#include <string>
#include <vector>

#include <geode/CacheableBuiltins.hpp>
#include <geode/PdxFieldTypes.hpp>
#include <geode/Serializable.hpp>

#include "PdxFieldType.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxType;

class PdxType : public internal::DataSerializableInternal,
                public std::enable_shared_from_this<PdxType> {
 private:
  using Fields = std::vector<std::shared_ptr<PdxFieldType>>;
  using IdentityFields = std::vector<std::shared_ptr<PdxFieldType>>;
  using FieldMap = std::map<std::string, std::shared_ptr<PdxFieldType>>;

 public:
  PdxType(const PdxType&) = delete;
  PdxType& operator=(const PdxType&) = delete;
  explicit PdxType(const std::string& className, bool expectDomainClass = true);

  ~PdxType() noexcept override;

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  static std::shared_ptr<Serializable> createDeserializable();

  size_t objectSize() const override;

  int32_t getTypeId() const { return typeId_; }

  void setTypeId(int32_t typeId) { typeId_ = typeId; }

  int32_t getOffsetsCount() const;

  int32_t getFieldsCount() const {
    return static_cast<int32_t>(fields_.size());
  }

  const std::string& getPdxClassName() const { return className_; }

  bool isDomainClass() const { return isDomainClass_; }

  std::shared_ptr<PdxFieldType> getField(int32_t idx) const;

  std::shared_ptr<PdxFieldType> getField(const std::string& name) const;

  const Fields& getFields() const { return fields_; }

  const FieldMap& getFieldMap() const { return fieldMap_; }

  std::shared_ptr<PdxFieldType> addField(const std::string& fieldName,
                                         PdxFieldTypes typeId);

  IdentityFields getIdentityFields() const;

  void initialize();

  int32_t getFieldPosition(std::shared_ptr<PdxFieldType> field,
                           uint8_t* offsets, int32_t offsetSize,
                           int32_t length);

  bool operator==(const PdxType& other) const;

 protected:

 private:
  int32_t getFixedFieldPos(std::shared_ptr<PdxFieldType> field,
                           uint8_t* offsets, int32_t offsetSize,
                           int32_t length);

  int32_t getVarFieldPos(std::shared_ptr<PdxFieldType> field,
                         uint8_t* offsets, int32_t offsetSize);

 private:
  bool initialized_;

  Fields fields_;
  std::string className_;
  bool isDomainClass_;

  int32_t typeId_;
  int32_t lastVarFieldId_;

  FieldMap fieldMap_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::PdxType> {
  typedef apache::geode::client::PdxType argument_type;
  typedef size_t result_type;
  result_type operator()(const argument_type& val) const {
    std::hash<std::string> strHash;
    auto result = strHash(val.getPdxClassName());

    for (const auto& field : val.getFields()) {
      result = result ^ (static_cast<uint32_t>(field->getType()) << 1);
      result = result ^ (strHash(field->getName()) << 1);
    }

    return result;
  }
};

}  // namespace std

#endif  // GEODE_PDXTYPE_H_
