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
#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "SerializationRegistry.hpp"
#include "util/JavaModifiedUtf8.hpp"
#include "util/string.hpp"

namespace apache {
namespace geode {
namespace client {

DataInput::DataInput(const uint8_t* buffer, size_t len, const CacheImpl* cache,
                     Pool* pool)
    : buffer_(buffer),
      bufferHead_(buffer),
      bufferLength_(len),
      pool_(pool),
      cache_(cache) {}

std::shared_ptr<Serializable> DataInput::readObjectInternal(int8_t typeId) {
  return getSerializationRegistry().deserialize(*this, typeId);
}

const SerializationRegistry& DataInput::getSerializationRegistry() const {
  return *cache_->getSerializationRegistry();
}

Cache* DataInput::getCache() const { return cache_->getCache(); }

template <class _Traits, class _Allocator>
void DataInput::readJavaModifiedUtf8(
    std::basic_string<char, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE skip intermediate utf16 string
  std::u16string utf16;
  readJavaModifiedUtf8(utf16);
  value = to_utf8(utf16);
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void
DataInput::readJavaModifiedUtf8(std::string&);

template <class _Traits, class _Allocator>
void DataInput::readJavaModifiedUtf8(
    std::basic_string<char16_t, _Traits, _Allocator>& value) {
  uint16_t length = readInt16();
  _GEODE_CHECK_BUFFER_SIZE(length);
  value = internal::JavaModifiedUtf8::decode(
      reinterpret_cast<const char*>(buffer_), length);
  advanceCursor(length);
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void
DataInput::readJavaModifiedUtf8(std::u16string&);

template <class _Traits, class _Allocator>
void DataInput::readJavaModifiedUtf8(
    std::basic_string<char32_t, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE convert from UTF-16 to UCS-4 directly
  std::u16string utf16;
  readJavaModifiedUtf8(utf16);
  value = to_ucs4(utf16);
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void
DataInput::readJavaModifiedUtf8(std::u32string&);

template <class _Traits, class _Allocator>
void DataInput::readUtf16Huge(
    std::basic_string<char, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE skip intermediate utf16 string
  std::u16string utf16;
  readUtf16Huge(utf16);
  value = to_utf8(utf16);
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void DataInput::readUtf16Huge(
    std::string&);

template <class _Traits, class _Allocator>
void DataInput::readUtf16Huge(
    std::basic_string<char32_t, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE convert from UTF-16 to UCS-4 directly
  std::u16string utf16;
  readUtf16Huge(utf16);
  value = to_ucs4(utf16);
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void DataInput::readUtf16Huge(
    std::u32string&);

}  // namespace client
}  // namespace geode
}  // namespace apache
