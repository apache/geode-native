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

#ifndef GEODE_PDXFIELDTYPE_H_
#define GEODE_PDXFIELDTYPE_H_

#include <string>

#include <geode/CacheableString.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/PdxFieldTypes.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/DataSerializableInternal.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

class PdxFieldType : public internal::DataSerializableInternal {
 public:
  static constexpr int32_t npos{-1};

 public:
  PdxFieldType(std::string fieldName, PdxFieldTypes typeId, int32_t sequenceId,
               int32_t varId);

  PdxFieldType();

  ~PdxFieldType() override = default;

  const std::string& getName() const { return name_; }

  PdxFieldTypes getType() const { return type_; }

  bool isVariable() const { return isVariable_; }

  bool isIdentity() const { return isIdentity_; }

  int32_t getRelativeOffset() const { return relOffset_; }

  uint8_t getIndex() const { return sequenceId_; }

  int32_t getVarId() const { return varId_; }

  int32_t getFixedSize() const { return fixedSize_; }

  int32_t getVarLenOffsetIndex() const { return varOffsetId_; }

  void setIdentity(bool isIdentity) { isIdentity_ = isIdentity; }

  void setVarOffsetId(int32_t value) { varOffsetId_ = value; }

  void setRelativeOffset(int32_t value) { relOffset_ = value; }

  void toData(DataOutput& output) const override;

  virtual void fromData(DataInput& input) override;

  virtual size_t objectSize() const override;

  std::string toString() const override;

  bool equals(std::shared_ptr<PdxFieldType> other);

  bool operator==(const PdxFieldType& other) const;

  bool operator!=(const PdxFieldType& other) const;

 private:
  std::string name_;
  PdxFieldTypes type_;

  bool isVariable_;
  bool isIdentity_;
  int32_t fixedSize_;

  int32_t relOffset_;
  int32_t sequenceId_;
  int32_t varId_;
  int32_t varOffsetId_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXFIELDTYPE_H_
