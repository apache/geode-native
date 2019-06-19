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

#include <vector>

#include <geode/DataOutput.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "SerializationRegistry.hpp"
#include "util/JavaModifiedUtf8.hpp"
#include "util/Log.hpp"
#include "util/string.hpp"

namespace apache {
namespace geode {
namespace client {

std::recursive_mutex globalBigBufferMutex;
size_t DataOutput::m_highWaterMark = 50 * 1024 * 1024;
size_t DataOutput::m_lowWaterMark = 8192;

/** This represents a allocation in this thread local pool. */
class BufferDesc {
 public:
  uint8_t* m_buf;
  size_t m_size;

  BufferDesc(uint8_t* buf, size_t size) : m_buf(buf), m_size(size) {}

  BufferDesc() : m_buf(nullptr), m_size(0) {}

  ~BufferDesc() {}

  BufferDesc& operator=(const BufferDesc& other) {
    if (this != &other) {
      m_buf = other.m_buf;
      m_size = other.m_size;
    }
    return *this;
  }

  BufferDesc(const BufferDesc& other)
      : m_buf(other.m_buf), m_size(other.m_size) {}
};

/** Thread local pool of buffers for DataOutput objects. */
class TSSDataOutput {
 private:
  std::vector<BufferDesc> m_buffers;

 public:
  TSSDataOutput();
  ~TSSDataOutput();

  uint8_t* getBuffer(size_t* size) {
    if (!m_buffers.empty()) {
      BufferDesc desc = m_buffers.back();
      m_buffers.pop_back();
      *size = desc.m_size;
      return desc.m_buf;
    } else {
      uint8_t* buf;
      *size = 8192;
      buf = static_cast<uint8_t*>(std::malloc(8192 * sizeof(uint8_t)));
      if (buf == nullptr) {
        throw OutOfMemoryException("Out of Memory while resizing buffer");
      }
      return buf;
    }
  }

  void poolBuffer(uint8_t* buf, size_t size) {
    BufferDesc desc(buf, size);
    m_buffers.push_back(desc);
  }

  static thread_local TSSDataOutput threadLocalBufferPool;
};

TSSDataOutput::TSSDataOutput() : m_buffers() {
  m_buffers.reserve(10);
  LOGDEBUG("DATAOUTPUT poolsize is %d", m_buffers.size());
}

TSSDataOutput::~TSSDataOutput() {
  while (!m_buffers.empty()) {
    BufferDesc desc = m_buffers.back();
    m_buffers.pop_back();
    std::free(desc.m_buf);
  }
}

thread_local TSSDataOutput TSSDataOutput::threadLocalBufferPool;

DataOutput::DataOutput(const CacheImpl* cache, Pool* pool)
    : m_size(0), m_haveBigBuffer(false), m_cache(cache), m_pool(pool) {
  m_bytes.reset(DataOutput::checkoutBuffer(&m_size));
  m_buf = m_bytes.get();
}

uint8_t* DataOutput::checkoutBuffer(size_t* size) {
  return TSSDataOutput::threadLocalBufferPool.getBuffer(size);
}

void DataOutput::checkinBuffer(uint8_t* buffer, size_t size) {
  TSSDataOutput::threadLocalBufferPool.poolBuffer(buffer, size);
}

void DataOutput::writeObjectInternal(const std::shared_ptr<Serializable>& ptr,
                                     bool isDelta) {
  getSerializationRegistry().serialize(ptr, *this, isDelta);
}

void DataOutput::acquireLock() { globalBigBufferMutex.lock(); }

void DataOutput::releaseLock() { globalBigBufferMutex.unlock(); }

const SerializationRegistry& DataOutput::getSerializationRegistry() const {
  return *m_cache->getSerializationRegistry();
}

Cache* DataOutput::getCache() const { return m_cache->getCache(); }

template <class _Traits, class _Allocator>
void DataOutput::writeJavaModifiedUtf8(
    const std::basic_string<char, _Traits, _Allocator>& value) {
  /*
   * OPTIMIZE convert from UTF-8 to CESU-8/Java Modified UTF-8 directly
   * http://www.unicode.org/reports/tr26/
   */
  if (value.empty()) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    writeJavaModifiedUtf8(to_utf16(value));
  }
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void
DataOutput::writeJavaModifiedUtf8(const std::string&);

template <class _Traits, class _Allocator>
void DataOutput::writeJavaModifiedUtf8(
    const std::basic_string<char32_t, _Traits, _Allocator>& value) {
  /*
   * OPTIMIZE convert from UCS-4 to CESU-8/Java Modified UTF-8 directly
   * http://www.unicode.org/reports/tr26/
   */
  if (value.empty()) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    writeJavaModifiedUtf8(to_utf16(value));
  }
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void
DataOutput::writeJavaModifiedUtf8(const std::u32string&);

void DataOutput::writeJavaModifiedUtf8(const char32_t* data, size_t len) {
  // TODO string optimize from UCS-4 to jmutf8
  if (0 == len) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    writeJavaModifiedUtf8(to_utf16(data, len));
  }
}

size_t DataOutput::getJavaModifiedUtf8EncodedLength(const char16_t* data,
                                                    size_t length) {
  return internal::JavaModifiedUtf8::encodedLength(data, length);
}

template <class _Traits, class _Allocator>
void DataOutput::writeUtf16Huge(
    const std::basic_string<char, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE convert from UTF-8 to UTF-16 directly
  if (value.empty()) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    writeUtf16Huge(to_utf16(value));
  }
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void DataOutput::writeUtf16Huge(
    const std::string&);

template <class _Traits, class _Allocator>
void DataOutput::writeUtf16Huge(
    const std::basic_string<char32_t, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE convert from UCS-4 to UTF-16 directly
  if (value.empty()) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    writeUtf16Huge(to_utf16(value));
  }
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void DataOutput::writeUtf16Huge(
    const std::u32string&);

void DataOutput::writeUtf16Huge(const char32_t* data, size_t len) {
  // TODO string optimize from UCS-4 to UTF-16
  if (0 == len) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    writeUtf16Huge(to_utf16(data, len));
  }
}

template <class _Traits, class _Allocator>
void DataOutput::writeUtf16(
    const std::basic_string<char, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE convert from UTF-8 to UTF-16 directly
  if (!value.empty()) {
    writeUtf16(to_utf16(value));
  }
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void DataOutput::writeUtf16(
    const std::string&);

template <class _Traits, class _Allocator>
void DataOutput::writeUtf16(
    const std::basic_string<char32_t, _Traits, _Allocator>& value) {
  // TODO string OPTIMIZE convert from UCS-4 to UTF-16 directly
  if (!value.empty()) {
    writeUtf16(to_utf16(value));
  }
}
template APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT void DataOutput::writeUtf16(
    const std::u32string&);

void DataOutput::writeUtf16(const char32_t* data, size_t len) {
  // TODO string optimize from UCS-4 to UTF-16
  if (len > 0) {
    writeUtf16(to_utf16(data, len));
  }
}

void DataOutput::write(uint8_t value) {
  ensureCapacity(1);
  writeNoCheck(value);
}

void DataOutput::write(int8_t value) { write(static_cast<uint8_t>(value)); }

void DataOutput::writeBoolean(bool value) {
  write(static_cast<uint8_t>(value));
}

void DataOutput::writeBytes(const uint8_t* bytes, int32_t len) {
  if (len >= 0) {
    ensureCapacity(len + 5);
    writeArrayLen(bytes == nullptr ? 0 : len);  // length of bytes...
    if (len > 0 && bytes != nullptr) {
      std::memcpy(m_buf, bytes, len);
      m_buf += len;
    }
  } else {
    write(static_cast<int8_t>(-1));
  }
}

void DataOutput::writeBytes(const int8_t* bytes, int32_t len) {
  writeBytes(reinterpret_cast<const uint8_t*>(bytes), len);
}

void DataOutput::writeBytesOnly(const uint8_t* bytes, size_t len) {
  ensureCapacity(len);
  std::memcpy(m_buf, bytes, len);
  m_buf += len;
}

void DataOutput::writeBytesOnly(const int8_t* bytes, size_t len) {
  writeBytesOnly(reinterpret_cast<const uint8_t*>(bytes), len);
}

void DataOutput::writeInt(uint16_t value) {
  ensureCapacity(2);
  *(m_buf++) = static_cast<uint8_t>(value >> 8);
  *(m_buf++) = static_cast<uint8_t>(value);
}

void DataOutput::writeChar(uint16_t value) {
  ensureCapacity(2);
  *(m_buf++) = static_cast<uint8_t>(value >> 8);
  *(m_buf++) = static_cast<uint8_t>(value);
}

void DataOutput::writeInt(uint32_t value) {
  ensureCapacity(4);
  *(m_buf++) = static_cast<uint8_t>(value >> 24);
  *(m_buf++) = static_cast<uint8_t>(value >> 16);
  *(m_buf++) = static_cast<uint8_t>(value >> 8);
  *(m_buf++) = static_cast<uint8_t>(value);
}

void DataOutput::writeInt(uint64_t value) {
  ensureCapacity(8);
  *(m_buf++) = static_cast<uint8_t>(value >> 56);
  *(m_buf++) = static_cast<uint8_t>(value >> 48);
  *(m_buf++) = static_cast<uint8_t>(value >> 40);
  *(m_buf++) = static_cast<uint8_t>(value >> 32);
  *(m_buf++) = static_cast<uint8_t>(value >> 24);
  *(m_buf++) = static_cast<uint8_t>(value >> 16);
  *(m_buf++) = static_cast<uint8_t>(value >> 8);
  *(m_buf++) = static_cast<uint8_t>(value);
}

void DataOutput::writeInt(int16_t value) {
  writeInt(static_cast<uint16_t>(value));
}

void DataOutput::writeInt(int32_t value) {
  writeInt(static_cast<uint32_t>(value));
}

void DataOutput::writeInt(int64_t value) {
  writeInt(static_cast<uint64_t>(value));
}

void DataOutput::writeArrayLen(int32_t len) {
  if (len == -1) {
    write(static_cast<int8_t>(-1));
  } else if (len <= 252) {  // 252 is java's ((byte)-4 && 0xFF)
    write(static_cast<uint8_t>(len));
  } else if (len <= 0xFFFF) {
    write(static_cast<int8_t>(-2));
    writeInt(static_cast<uint16_t>(len));
  } else {
    write(static_cast<int8_t>(-3));
    writeInt(len);
  }
}

void DataOutput::writeFloat(float value) {
  union float_uint32_t {
    float f;
    uint32_t u;
  } v;
  v.f = value;
  writeInt(v.u);
}

void DataOutput::writeDouble(double value) {
  union double_uint64_t {
    double d;
    uint64_t ll;
  } v;
  v.d = value;
  writeInt(v.ll);
}

const uint8_t* DataOutput::getCursor() { return m_buf; }

void DataOutput::advanceCursor(size_t offset) {
  ensureCapacity(offset);
  m_buf += offset;
}

void DataOutput::rewindCursor(size_t offset) { m_buf -= offset; }

void DataOutput::updateValueAtPos(size_t offset, uint8_t value) {
  m_bytes.get()[offset] = value;
}

uint8_t DataOutput::getValueAtPos(size_t offset) {
  return m_bytes.get()[offset];
}

const uint8_t* DataOutput::getBuffer() const { return m_bytes.get(); }

size_t DataOutput::getRemainingBufferLength() const {
  return m_size - getBufferLength();
}

const uint8_t* DataOutput::getBuffer(size_t* rsize) const {
  *rsize = m_buf - m_bytes.get();
  return m_bytes.get();
}

uint8_t* DataOutput::getBufferCopy() {
  size_t size = m_buf - m_bytes.get();
  auto result = static_cast<uint8_t*>(std::malloc(size * sizeof(uint8_t)));
  if (result == nullptr) {
    throw OutOfMemoryException("Out of Memory while resizing buffer");
  }
  std::memcpy(result, m_bytes.get(), size);
  return result;
}

size_t DataOutput::getBufferLength() const { return m_buf - m_bytes.get(); }

void DataOutput::reset() {
  if (m_haveBigBuffer) {
    // create smaller buffer
    m_bytes.reset(
        static_cast<uint8_t*>(std::malloc(m_lowWaterMark * sizeof(uint8_t))));
    if (m_bytes == nullptr) {
      throw OutOfMemoryException("Out of Memory while resizing buffer");
    }
    m_size = m_lowWaterMark;
    // reset the flag
    m_haveBigBuffer = false;
    // release the lock
    releaseLock();
  }
  m_buf = m_bytes.get();
}

void DataOutput::ensureCapacity(size_t size) {
  size_t offset = m_buf - m_bytes.get();
  if ((m_size - offset) < size) {
    size_t newSize = m_size * 2 + (8192 * (size / 8192));
    if (newSize >= m_highWaterMark && !m_haveBigBuffer) {
      // acquire the lock
      acquireLock();
      // set flag
      m_haveBigBuffer = true;
    }
    m_size = newSize;

    auto bytes = m_bytes.release();
    auto tmp =
        static_cast<uint8_t*>(std::realloc(bytes, m_size * sizeof(uint8_t)));
    if (tmp == nullptr) {
      throw OutOfMemoryException("Out of Memory while resizing buffer");
    }
    m_bytes.reset(tmp);
    m_buf = m_bytes.get() + offset;
  }
}

uint8_t* DataOutput::getBufferCopyFrom(const uint8_t* from, size_t length) {
  uint8_t* result;
  _GEODE_NEW(result, uint8_t[length]);
  std::memcpy(result, from, length);

  return result;
}

void DataOutput::safeDelete(uint8_t* src) { _GEODE_SAFE_DELETE(src); }

DataOutput::~DataOutput() {
  reset();
  if (m_bytes) {
    DataOutput::checkinBuffer(m_bytes.release(), m_size);
  }
}

void DataOutput::writeAscii(const std::string& value) {
  uint16_t len = static_cast<uint16_t>(
      std::min<size_t>(value.length(), std::numeric_limits<uint16_t>::max()));
  writeInt(len);
  for (size_t i = 0; i < len; i++) {
    // blindly assumes ascii so mask off only 7 bits
    write(static_cast<int8_t>(value.data()[i] & 0x7F));
  }
}

void DataOutput::writeAsciiHuge(const std::string& value) {
  uint32_t len = static_cast<uint32_t>(
      std::min<size_t>(value.length(), std::numeric_limits<uint32_t>::max()));
  writeInt(static_cast<uint32_t>(len));
  for (size_t i = 0; i < len; i++) {
    // blindly assumes ascii so mask off only 7 bits
    write(static_cast<int8_t>(value.data()[i] & 0x7F));
  }
}

void DataOutput::writeJavaModifiedUtf8(const char16_t* data, size_t len) {
  if (0 == len) {
    writeInt(static_cast<uint16_t>(0));
  } else {
    auto encodedLen = static_cast<uint16_t>(
        std::min<size_t>(getJavaModifiedUtf8EncodedLength(data, len),
                         std::numeric_limits<uint16_t>::max()));
    writeInt(encodedLen);
    ensureCapacity(encodedLen);
    const auto end = m_buf + encodedLen;
    while (m_buf < end) {
      encodeJavaModifiedUtf8(*data++);
    }
    if (m_buf > end) m_buf = end;
  }
}

void DataOutput::writeUtf16Huge(const char16_t* data, size_t length) {
  uint32_t len = static_cast<uint32_t>(
      std::min<size_t>(length, std::numeric_limits<uint32_t>::max()));
  writeInt(len);
  writeUtf16(data, length);
}

void DataOutput::writeUtf16(const char16_t* data, size_t length) {
  ensureCapacity(length * 2);
  for (; length > 0; length--, data++) {
    writeNoCheck(static_cast<uint8_t>(*data >> 8));
    writeNoCheck(static_cast<uint8_t>(*data));
  }
}

void DataOutput::getEncodedLength(const char val, int32_t& encodedLen) {
  if ((val == 0) || (val & 0x80)) {
    // two byte.
    encodedLen += 2;
  } else {
    // one byte.
    encodedLen++;
  }
}

void DataOutput::getEncodedLength(const wchar_t val, int32_t& encodedLen) {
  if (val == 0) {
    encodedLen += 2;
  } else if (val < 0x80)  // ASCII character
  {
    encodedLen++;
  } else if (val < 0x800) {
    encodedLen += 2;
  } else {
    encodedLen += 3;
  }
}

void DataOutput::encodeChar(const char value) {
  uint8_t tmp = static_cast<uint8_t>(value);
  if ((tmp == 0) || (tmp & 0x80)) {
    // two byte.
    *(m_buf++) = static_cast<uint8_t>(0xc0 | ((tmp & 0xc0) >> 6));
    *(m_buf++) = static_cast<uint8_t>(0x80 | (tmp & 0x3f));
  } else {
    // one byte.
    *(m_buf++) = tmp;
  }
}

void DataOutput::encodeChar(const wchar_t value) {
  uint16_t c = static_cast<uint16_t>(value);
  if (c == 0) {
    *(m_buf++) = 0xc0;
    *(m_buf++) = 0x80;
  } else if (c < 0x80) {  // ASCII character
    *(m_buf++) = static_cast<uint8_t>(c);
  } else if (c < 0x800) {
    *(m_buf++) = static_cast<uint8_t>(0xC0 | c >> 6);
    *(m_buf++) = static_cast<uint8_t>(0x80 | (c & 0x3F));
  } else {
    *(m_buf++) = static_cast<uint8_t>(0xE0 | c >> 12);
    *(m_buf++) = static_cast<uint8_t>(0x80 | ((c >> 6) & 0x3F));
    *(m_buf++) = static_cast<uint8_t>(0x80 | (c & 0x3F));
  }
}

void DataOutput::encodeJavaModifiedUtf8(const char16_t c) {
  if (c == 0) {
    // NUL
    *(m_buf++) = 0xc0;
    *(m_buf++) = 0x80;
  } else if (c < 0x80) {
    // ASCII character
    *(m_buf++) = static_cast<uint8_t>(c);
  } else if (c < 0x800) {
    *(m_buf++) = static_cast<uint8_t>(0xC0 | c >> 6);
    *(m_buf++) = static_cast<uint8_t>(0x80 | (c & 0x3F));
  } else {
    *(m_buf++) = static_cast<uint8_t>(0xE0 | c >> 12);
    *(m_buf++) = static_cast<uint8_t>(0x80 | ((c >> 6) & 0x3F));
    *(m_buf++) = static_cast<uint8_t>(0x80 | (c & 0x3F));
  }
}

void DataOutput::writeNoCheck(uint8_t value) { *(m_buf++) = value; }

void DataOutput::writeNoCheck(int8_t value) {
  writeNoCheck(static_cast<uint8_t>(value));
}

Pool* DataOutput::getPool() const { return m_pool; }

}  // namespace client
}  // namespace geode
}  // namespace apache
