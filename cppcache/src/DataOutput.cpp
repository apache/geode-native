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

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "SerializationRegistry.hpp"
#include "util/JavaModifiedUtf8.hpp"
#include "util/Log.hpp"
#include "util/string.hpp"

namespace apache {
namespace geode {
namespace client {

static std::recursive_mutex globalBigBufferMutex;
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
  LOG_DEBUG("DATAOUTPUT poolsize is %zu", m_buffers.size());
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

}  // namespace client
}  // namespace geode
}  // namespace apache
