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
    : m_buf(buffer),
      m_bufHead(buffer),
      m_bufLength(len),
      m_pool(pool),
      m_cache(cache) {}

std::shared_ptr<Serializable> DataInput::readObjectInternal(int8_t typeId) {
  return getSerializationRegistry().deserialize(*this, typeId);
}

const SerializationRegistry& DataInput::getSerializationRegistry() const {
  return *m_cache->getSerializationRegistry();
}

Cache* DataInput::getCache() const { return m_cache->getCache(); }

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
      reinterpret_cast<const char*>(m_buf), length);
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

int8_t DataInput::read() {
  _GEODE_CHECK_BUFFER_SIZE(1);
  return readNoCheck();
}

/**
 * Read a boolean value from the <code>DataInput</code>.
 *
 * @param value output parameter to hold the boolean read from stream
 */
bool DataInput::readBoolean() {
  _GEODE_CHECK_BUFFER_SIZE(1);
  return *(m_buf++) == 1 ? true : false;
}

/**
 * Read the given number of unsigned bytes from the <code>DataInput</code>.
 * @remarks This method is complimentary to
 *   <code>DataOutput::writeBytesOnly</code> and, unlike
 *   <code>readBytes</code>, does not expect the length of array
 *   in the stream.
 *
 * @param buffer array to hold the bytes read from stream
 * @param len number of unsigned bytes to be read
 */
void DataInput::readBytesOnly(uint8_t* buffer, size_t len) {
  if (len > 0) {
    _GEODE_CHECK_BUFFER_SIZE(len);
    std::memcpy(buffer, m_buf, len);
    m_buf += len;
  }
}

/**
 * Read the given number of signed bytes from the <code>DataInput</code>.
 * @remarks This method is complimentary to
 *   <code>DataOutput::writeBytesOnly</code> and, unlike
 *   <code>readBytes</code>, does not expect the length of array
 *   in the stream.
 *
 * @param buffer array to hold the bytes read from stream
 * @param len number of signed bytes to be read
 */
void DataInput::readBytesOnly(int8_t* buffer, size_t len) {
  if (len > 0) {
    _GEODE_CHECK_BUFFER_SIZE(len);
    std::memcpy(buffer, m_buf, len);
    m_buf += len;
  }
}

/**
 * Read an array of unsigned bytes from the <code>DataInput</code>
 * expecting to find the length of array in the stream at the start.
 * @remarks This method is complimentary to
 *   <code>DataOutput::writeBytes</code>.
 *
 * @param bytes output array to hold the bytes read from stream; the array
 *   is allocated by this method
 * @param len output parameter to hold the length of array read from stream
 */
void DataInput::readBytes(uint8_t** bytes, int32_t* len) {
  auto length = readArrayLength();
  *len = length;
  uint8_t* buffer = nullptr;
  if (length > 0) {
    _GEODE_CHECK_BUFFER_SIZE(length);
    _GEODE_NEW(buffer, uint8_t[length]);
    std::memcpy(buffer, m_buf, length);
    m_buf += length;
  }
  *bytes = buffer;
}

void DataInput::readBytes(int8_t** bytes, int32_t* len) {
  auto length = readArrayLength();
  *len = length;
  int8_t* buffer = nullptr;
  if (length > 0) {
    _GEODE_CHECK_BUFFER_SIZE(length);
    _GEODE_NEW(buffer, int8_t[length]);
    std::memcpy(buffer, m_buf, length);
    m_buf += length;
  }
  *bytes = buffer;
}

int16_t DataInput::readInt16() {
  _GEODE_CHECK_BUFFER_SIZE(2);
  return readInt16NoCheck();
}

int32_t DataInput::readInt32() {
  _GEODE_CHECK_BUFFER_SIZE(4);
  int32_t tmp = *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  return tmp;
}

int64_t DataInput::readInt64() {
  _GEODE_CHECK_BUFFER_SIZE(8);
  int64_t tmp;
  tmp = *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  tmp = (tmp << 8) | *(m_buf++);
  return tmp;
}

int32_t DataInput::readArrayLength() {
  const uint8_t code = read();
  if (code == 0xFF) {
    return -1;
  } else {
    int32_t result = code;
    if (result > 252) {  // 252 is java's ((byte)-4 && 0xFF)
      if (code == 0xFE) {
        uint16_t val = readInt16();
        result = val;
      } else if (code == 0xFD) {
        uint32_t val = readInt32();
        result = val;
      } else {
        throw IllegalStateException("unexpected array length code");
      }
    }
    return result;
  }
}

int64_t DataInput::readUnsignedVL() {
  int32_t shift = 0;
  int64_t result = 0;
  while (shift < 64) {
    const auto b = read();
    result |= static_cast<int64_t>(b & 0x7F) << shift;
    if ((b & 0x80) == 0) {
      return result;
    }
    shift += 7;
  }
  throw IllegalStateException("Malformed variable length integer");
}

float DataInput::readFloat() {
  _GEODE_CHECK_BUFFER_SIZE(4);
  union float_uint32_t {
    float f;
    uint32_t u;
  } v;
  v.u = readInt32();
  return v.f;
}

double DataInput::readDouble() {
  _GEODE_CHECK_BUFFER_SIZE(8);
  union double_uint64_t {
    double d;
    uint64_t ll;
  } v;
  v.ll = readInt64();
  return v.d;
}

bool DataInput::readNativeBool() {
  read();  // ignore type id
  return readBoolean();
}

int32_t DataInput::readNativeInt32() {
  read();  // ignore type id
  return readInt32();
}

std::shared_ptr<Serializable> DataInput::readDirectObject(int8_t typeId) {
  return readObjectInternal(typeId);
}

std::shared_ptr<Serializable> DataInput::readObject() {
  return readObjectInternal();
}

void DataInput::readObject(std::shared_ptr<Serializable>& ptr) {
  ptr = readObjectInternal();
}

void DataInput::readObject(char16_t* value) { *value = readInt16(); }

void DataInput::readObject(bool* value) { *value = readBoolean(); }

void DataInput::readObject(int8_t* value) { *value = read(); }

void DataInput::readObject(int16_t* value) { *value = readInt16(); }

void DataInput::readObject(int32_t* value) { *value = readInt32(); }

void DataInput::readObject(int64_t* value) { *value = readInt64(); }

void DataInput::readObject(float* value) { *value = readFloat(); }

void DataInput::readObject(double* value) { *value = readDouble(); }

std::vector<char16_t> DataInput::readCharArray() {
  return readArray<char16_t>();
}

std::vector<bool> DataInput::readBooleanArray() { return readArray<bool>(); }

std::vector<int8_t> DataInput::readByteArray() { return readArray<int8_t>(); }

std::vector<int16_t> DataInput::readShortArray() {
  return readArray<int16_t>();
}

std::vector<int32_t> DataInput::readIntArray() { return readArray<int32_t>(); }

std::vector<int64_t> DataInput::readLongArray() { return readArray<int64_t>(); }

std::vector<float> DataInput::readFloatArray() { return readArray<float>(); }

std::vector<double> DataInput::readDoubleArray() { return readArray<double>(); }

std::vector<std::string> DataInput::readStringArray() {
  std::vector<std::string> value;

  auto arrLen = readArrayLength();
  if (arrLen > 0) {
    value.reserve(arrLen);
    for (int i = 0; i < arrLen; i++) {
      value.push_back(readString());
    }
  }

  return value;
}

void DataInput::readArrayOfByteArrays(int8_t*** arrayofBytearr,
                                      int32_t& arrayLength,
                                      int32_t** elementLength) {
  auto arrLen = readArrayLength();
  arrayLength = arrLen;

  if (arrLen == -1) {
    *arrayofBytearr = nullptr;
    return;
  } else {
    int8_t** tmpArray;
    int32_t* tmpLengtharr;
    _GEODE_NEW(tmpArray, int8_t * [arrLen]);
    _GEODE_NEW(tmpLengtharr, int32_t[arrLen]);
    for (int i = 0; i < arrLen; i++) {
      readBytes(&tmpArray[i], &tmpLengtharr[i]);
    }
    *arrayofBytearr = tmpArray;
    *elementLength = tmpLengtharr;
  }
}

const uint8_t* DataInput::currentBufferPosition() const { return m_buf; }

size_t DataInput::getBytesRead() const { return m_buf - m_bufHead; }

size_t DataInput::getBytesRemaining() const {
  return (m_bufLength - getBytesRead());
}

void DataInput::advanceCursor(size_t offset) { m_buf += offset; }

void DataInput::rewindCursor(size_t offset) { m_buf -= offset; }

void DataInput::reset() { m_buf = m_bufHead; }

void DataInput::setBuffer() {
  m_buf = currentBufferPosition();
  m_bufLength = getBytesRemaining();
}

void DataInput::resetPdx(size_t offset) { m_buf = m_bufHead + offset; }

size_t DataInput::getPdxBytes() const { return m_bufLength; }

uint8_t* DataInput::getBufferCopy(const uint8_t* from, size_t length) {
  uint8_t* result;
  _GEODE_NEW(result, uint8_t[length]);
  std::memcpy(result, from, length);

  return result;
}

void DataInput::reset(size_t offset) { m_buf = m_bufHead + offset; }

uint8_t* DataInput::getBufferCopyFrom(const uint8_t* from, size_t length) {
  uint8_t* result;
  _GEODE_NEW(result, uint8_t[length]);
  std::memcpy(result, from, length);

  return result;
}

char DataInput::readPdxChar() { return static_cast<char>(readInt16()); }

void DataInput::_checkBufferSize(size_t size, int32_t line) {
  if ((m_bufLength - (m_buf - m_bufHead)) < size) {
    throw OutOfRangeException(
        "DataInput: attempt to read beyond buffer at line " +
        std::to_string(line) + ": available buffer size " +
        std::to_string(m_bufLength - (m_buf - m_bufHead)) +
        ", attempted read of size " + std::to_string(size));
  }
}

int8_t DataInput::readNoCheck() { return *(m_buf++); }

int16_t DataInput::readInt16NoCheck() {
  int16_t tmp = *(m_buf++);
  tmp = static_cast<int16_t>((tmp << 8) | *(m_buf++));
  return tmp;
}

Pool* DataInput::getPool() const { return m_pool; }

}  // namespace client
}  // namespace geode
}  // namespace apache
