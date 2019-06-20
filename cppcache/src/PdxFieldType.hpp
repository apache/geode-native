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

class APACHE_GEODE_EXPORT PdxFieldType
    : public internal::DataSerializableInternal {
 private:
  std::string m_fieldName;
  std::string m_className;
  PdxFieldTypes m_typeId;
  uint32_t m_sequenceId;

  bool m_isVariableLengthType;
  bool m_isIdentityField;
  int32_t m_fixedSize;
  int32_t m_varLenFieldIdx;

  int32_t m_vlOffsetIndex;
  int32_t m_relativeOffset;

  int32_t getFixedTypeSize() const;

 public:
  PdxFieldType(std::string fieldName, std::string className,
               PdxFieldTypes typeId, int32_t sequenceId,
               bool isVariableLengthType, int32_t fixedSize,
               int32_t varLenFieldIdx);

  PdxFieldType();

  const std::string& getFieldName();

  const std::string& getClassName();

  PdxFieldTypes getTypeId();

  uint8_t getSequenceId();

  bool IsVariableLengthType();

  bool getIdentityField() const;

  int32_t getVarLenFieldIdx() const;

  void setVarLenOffsetIndex(int32_t value);

  void setRelativeOffset(int32_t value);

  int32_t getFixedSize() const;
  void setIdentityField(bool identityField);

  // TODO:add more getters for the remaining members.

  void toData(DataOutput& output) const override;

  virtual void fromData(DataInput& input) override;

  virtual size_t objectSize() const override;

  std::string toString() const override;

  ~PdxFieldType() override = default;

  bool equals(std::shared_ptr<PdxFieldType> otherObj);

  int32_t getVarLenOffsetIndex() const;

  int32_t getRelativeOffset() const;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXFIELDTYPE_H_
