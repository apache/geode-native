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

#include "CacheRegionHelper.hpp"
#include "SerializationRegistry.hpp"
#include "CacheImpl.hpp"
#include "util/string.hpp"

namespace apache {
namespace geode {
namespace client {

std::shared_ptr<Serializable> DataInput::readObjectInternal(int8_t typeId) {
  return getSerializationRegistry().deserialize(*this, typeId);
}

const SerializationRegistry& DataInput::getSerializationRegistry() const {
  return *CacheRegionHelper::getCacheImpl(m_cache)->getSerializationRegistry();
}

const Cache* DataInput::getCache() { return m_cache; }

void DataInput::readJavaModifiedUtf8(std::string& value) {
  // OPTIMIZE transcode Java Modified UTF-8/CESU-8 to standard UTF-8
  std::u16string utf16;
  readJavaModifiedUtf8(utf16);
  value = to_utf8(utf16);
}

void DataInput::readJavaModifiedUtf8(std::u16string& value) {
  uint16_t length = readInt16();
  checkBufferSize(length);
  uint16_t decodedLen = static_cast<uint16_t>(getDecodedLength(m_buf, length));
  value.reserve(decodedLen);
  for (uint16_t i = 0; i < decodedLen; i++) {
    value.push_back(decodeJavaModifiedUtf8Char());
  }
}

inline char16_t DataInput::decodeJavaModifiedUtf8Char() {
  char16_t c;
  // get next byte unsigned
  int32_t b = *m_buf++ & 0xff;
  int32_t k = b >> 5;
  // classify based on the high order 3 bits
  switch (k) {
    case 6: {
      // two byte encoding
      // 110yyyyy 10xxxxxx
      // use low order 6 bits
      int32_t y = b & 0x1f;
      // use low order 6 bits of the next byte
      // It should have high order bits 10, which we don't check.
      int32_t x = *m_buf++ & 0x3f;
      // 00000yyy yyxxxxxx
      c = (y << 6 | x);
      break;
    }
    case 7: {
      // three byte encoding
      // 1110zzzz 10yyyyyy 10xxxxxx
      // use low order 4 bits
      int32_t z = b & 0x0f;
      // use low order 6 bits of the next byte
      // It should have high order bits 10, which we don't check.
      int32_t y = *m_buf++ & 0x3f;
      // use low order 6 bits of the next byte
      // It should have high order bits 10, which we don't check.
      int32_t x = *m_buf++ & 0x3f;
      // zzzzyyyy yyxxxxxx
      c = (z << 12 | y << 6 | x);
      break;
    }
    default:
      // one byte encoding
      // 0xxxxxxx
      // use just low order 7 bits
      // 00000000 0xxxxxxx
      c = (b & 0x7f);
      break;
  }
  return c;
}

void DataInput::readUtf16Huge(std::string& value) {
  // OPTIMIZE skip intermediate utf16 string
  std::u16string utf16;
  readUtf16Huge(utf16);
  value = to_utf8(utf16);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
