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

#include <geode/DataInput.hpp>

#include "EnumInfo.hpp"
#include "Utils.hpp"
#include "GeodeTypeIdsImpl.hpp"

namespace apache {
namespace geode {
namespace client {

EnumInfo::EnumInfo()
    : m_enumClassName(nullptr), m_enumName(nullptr), m_ordinal(-1) {}

EnumInfo::EnumInfo(const char *enumClassName, const char *enumName,
                   int32_t ordinal)
    : m_ordinal(ordinal) {
  m_enumClassName = CacheableString::create(enumClassName);
  m_enumName = CacheableString::create(enumName);
}

int32_t EnumInfo::hashcode() const {
  return ((m_enumClassName != nullptr ? m_enumClassName->hashcode() : 0) +
          (m_enumName != nullptr ? m_enumName->hashcode() : 0));
}

bool EnumInfo::operator==(const CacheableKey &other) const {
  if (auto otherEnum = dynamic_cast<const EnumInfo *>(&other)) {
    return m_ordinal == otherEnum->m_ordinal &&
           *m_enumClassName == *otherEnum->m_enumClassName &&
           *m_enumName == *otherEnum->m_enumName;
  }

  return false;
}

void EnumInfo::toData(apache::geode::client::DataOutput &output) const {
  output.writeObject(m_enumClassName);
  output.writeObject(m_enumName);
  output.writeInt(m_ordinal);
}

void EnumInfo::fromData(apache::geode::client::DataInput &input) {
  m_enumClassName =
      std::dynamic_pointer_cast<CacheableString>(input.readObject());
  m_enumName = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  m_ordinal = input.readInt32();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
