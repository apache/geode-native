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

#ifndef GEODE_ENUMINFO_H_
#define GEODE_ENUMINFO_H_

#include <geode/CacheableKey.hpp>
#include <geode/CacheableString.hpp>

#include "geode/internal/DataSerializableFixedId.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DataSerializableFixedId_t;
using internal::DSFid;

class APACHE_GEODE_EXPORT EnumInfo
    : public DataSerializableFixedId_t<DSFid::EnumInfo>,
      public CacheableKey {
 private:
  std::shared_ptr<CacheableString> m_enumClassName;
  std::shared_ptr<CacheableString> m_enumName;
  int32_t m_ordinal;

 public:
  ~EnumInfo() override = default;
  EnumInfo();
  EnumInfo(const char* enumClassName, const char* enumName, int32_t m_ordinal);

  static std::shared_ptr<Serializable> createDeserializable();

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  size_t objectSize() const override;

  std::string toString() const override;

  bool operator==(const CacheableKey& other) const override;

  int32_t hashcode() const override;

  std::shared_ptr<CacheableString> getEnumClassName() const;

  std::shared_ptr<CacheableString> getEnumName() const;

  int32_t getEnumOrdinal() const;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_ENUMINFO_H_
