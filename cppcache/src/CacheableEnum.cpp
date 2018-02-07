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

#include <geode/CacheableEnum.hpp>
#include <geode/internal/functional.hpp>

#include "Utils.hpp"
#include "PdxHelper.hpp"
#include "GeodeTypeIdsImpl.hpp"
#include "EnumInfo.hpp"
#include "CacheRegionHelper.hpp"

namespace apache {
namespace geode {
namespace client {

CacheableEnum::CacheableEnum()
    : m_enumClassName(nullptr),
      m_enumName(nullptr),
      m_ordinal(-1),
      m_hashcode(0) {}

CacheableEnum::CacheableEnum(std::string enumClassName, std::string enumName,
                             int32_t ordinal)
    : m_enumClassName(std::move(enumClassName)),
      m_enumName(std::move(enumName)),
      m_ordinal(ordinal),
      m_hashcode(0) {}

void CacheableEnum::toData(apache::geode::client::DataOutput& output) const {
  int enumVal = PdxHelper::getEnumValue(
      m_enumClassName.c_str(), m_enumName.c_str(), m_ordinal,
      CacheRegionHelper::getCacheImpl(output.getCache())->getPdxTypeRegistry());
  output.write(static_cast<int8_t>(GeodeTypeIds::CacheableEnum));
  output.write(int8_t(enumVal >> 24));
  output.writeArrayLen(enumVal & 0xFFFFFF);
}

void CacheableEnum::fromData(apache::geode::client::DataInput& input) {
  auto dsId = input.read();
  int32_t arrLen = input.readArrayLen();
  int enumId = (dsId << 24) | (arrLen & 0xFFFFFF);
  auto enumVal = PdxHelper::getEnum(
      enumId,
      CacheRegionHelper::getCacheImpl(input.getCache())->getPdxTypeRegistry());

  m_enumClassName = enumVal->getEnumClassName()->toString();
  m_enumName = enumVal->getEnumName()->toString();
  m_ordinal = enumVal->getEnumOrdinal();

  calculateHashcode();
}

void CacheableEnum::calculateHashcode() {
  m_hashcode = 1;
  const int32_t prime = 31;
  m_hashcode = prime * m_hashcode + geode_hash<std::string>{}(m_enumClassName);
  m_hashcode = prime * m_hashcode + geode_hash<std::string>{}(m_enumName);
}

bool CacheableEnum::operator==(const CacheableKey& other) const {
  if (other.typeId() != typeId()) {
    return false;
  }
  auto&& otherEnum = dynamic_cast<const CacheableEnum*>(&other);
  if (otherEnum == nullptr) {
    return false;
  }
  if (m_ordinal != otherEnum->m_ordinal) {
    return false;
  }
  if (m_enumClassName != otherEnum->m_enumClassName) {
    return false;
  }
  if (m_enumName != otherEnum->m_enumName) {
    return false;
  }
  return true;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
