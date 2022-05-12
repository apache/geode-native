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

#ifndef GEODE_PDXHELPER_H_
#define GEODE_PDXHELPER_H_

#include <geode/DataOutput.hpp>

#include "CacheImpl.hpp"
#include "EnumInfo.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxInstanceImpl;
class PdxType;

class PdxHelper {
 public:
  static constexpr auto HeaderSize = sizeof(uint32_t) + sizeof(uint32_t);

  // Serialize
  static std::shared_ptr<PdxType> serializePdxInstance(
      DataOutput& output, const std::shared_ptr<PdxInstanceImpl>& instance);

  static std::shared_ptr<PdxType> serializePdxSerializable(
      DataOutput& output, const std::shared_ptr<PdxSerializable>& pdxObject);

  static void serializePdx(DataOutput& output,
                           const std::shared_ptr<PdxSerializable>& pdxObject);

  // Deserialize
  static std::shared_ptr<PdxSerializable> deserializePdx(DataInput& input);

  static std::shared_ptr<PdxSerializable> deserializePdx(
      DataInput& input, std::shared_ptr<PdxType> pdxType, int32_t length);

  static std::shared_ptr<PdxSerializable> deserializePdxInstance(
      DataInput& input, std::shared_ptr<PdxType> pdxType, int32_t length);

  static std::shared_ptr<PdxSerializable> deserializePdxSerializable(
      DataInput& input, std::shared_ptr<PdxType> pdxType, int32_t length);

  static size_t getOffsetSize(size_t length);

  static int32_t readInt16(uint8_t* offsetPosition);

  static int32_t readUInt16(uint8_t* offsetPosition);

  static int32_t readByte(uint8_t* offsetPosition);

  static int32_t readInt32(uint8_t* offsetPosition);

  static void writeByte(uint8_t* offsetPosition, int32_t value);

  static int32_t readInt(uint8_t* offsetPosition, int size);

  static int32_t getEnumValue(const char* enumClassName, const char* enumName,
                              int hashcode,
                              std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);

  static std::shared_ptr<EnumInfo> getEnum(
      int enumId, std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXHELPER_H_
