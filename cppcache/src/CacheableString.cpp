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

#include <codecvt>
#include <cstdlib>
#include <cwchar>
#include <locale>

#include <geode/CacheableString.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>

#include "DataOutputInternal.hpp"
#include "SerializationRegistry.hpp"
#include "Utils.hpp"
#include "internal/string.hpp"

namespace apache {
namespace geode {
namespace client {

void CacheableString::toData(DataOutput& output) const {
  if (m_type == DSCode::CacheableASCIIString) {
    output.writeAscii(m_str);
  } else if (m_type == DSCode::CacheableString) {
    output.writeJavaModifiedUtf8(m_str);
  } else if (m_type == DSCode::CacheableASCIIStringHuge) {
    output.writeAsciiHuge(m_str);
  } else if (m_type == DSCode::CacheableStringHuge) {
    output.writeUtf16Huge(m_str);
  }
}

void CacheableString::fromData(DataInput& input) {
  if (m_type == DSCode::CacheableASCIIString) {
    input.readAscii(m_str);
  } else if (m_type == DSCode::CacheableString) {
    input.readJavaModifiedUtf8(m_str);
  } else if (m_type == DSCode::CacheableASCIIStringHuge) {
    input.readAsciiHuge(m_str);
  } else if (m_type == DSCode::CacheableStringHuge) {
    input.readUtf16Huge(m_str);
  }
}

std::shared_ptr<Serializable> CacheableString::createDeserializable() {
  return std::make_shared<CacheableString>(DSCode::CacheableASCIIString);
}

std::shared_ptr<Serializable> CacheableString::createDeserializableHuge() {
  return std::make_shared<CacheableString>(DSCode::CacheableASCIIStringHuge);
}

std::shared_ptr<Serializable> CacheableString::createUTFDeserializable() {
  return std::make_shared<CacheableString>(DSCode::CacheableString);
}

std::shared_ptr<Serializable> CacheableString::createUTFDeserializableHuge() {
  return std::make_shared<CacheableString>(DSCode::CacheableStringHuge);
}

std::shared_ptr<CacheableString> CacheableString::create(
    const std::u16string& value) {
  return std::make_shared<CacheableString>(to_utf8(value));
}

std::shared_ptr<CacheableString> CacheableString::create(
    std::u16string&& value) {
  return std::make_shared<CacheableString>(to_utf8(value));
}

std::shared_ptr<CacheableString> CacheableString::create(
    const std::u32string& value) {
  return std::make_shared<CacheableString>(to_utf8(value));
}

std::shared_ptr<CacheableString> CacheableString::create(
    std::u32string&& value) {
  return std::make_shared<CacheableString>(to_utf8(value));
}

bool CacheableString::operator==(const CacheableKey& other) const {
  if (auto otherString = dynamic_cast<const CacheableString*>(&other)) {
    return m_str == otherString->m_str;
  }

  return false;
}

int32_t CacheableString::hashcode() const {
  if (m_hashcode == 0) {
    m_hashcode = internal::geode_hash<std::string>{}(m_str);
  }
  return m_hashcode;
}

bool CacheableString::isAscii(const std::string& str) {
  for (auto&& c : str) {
    if (c & 0x80) {
      return false;
    }
  }
  return true;
}

size_t CacheableString::objectSize() const {
  auto size = sizeof(CacheableString) +
              sizeof(std::string::value_type) * m_str.capacity();
  return size;
}

std::string CacheableString::toString() const { return m_str; }

}  // namespace client
}  // namespace geode
}  // namespace apache
