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

#ifndef GEODE_DATAOUTPUT_H_
#define GEODE_DATAOUTPUT_H_

#include <cstring>
#include <string>
#include <cstdlib>
#include <algorithm>

#include "geode_globals.hpp"
#include "ExceptionTypes.hpp"
#include "Serializable.hpp"
#include "CacheableString.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {
class SerializationRegistry;
class DataOutputInternal;
class CacheImpl;

/**
 * Provide operations for writing primitive data values, byte arrays,
 * strings, <code>Serializable</code> objects to a byte stream.
 * This class is intentionally not thread safe.
 */
class CPPCACHE_EXPORT DataOutput {
 public:
  /**
   * Write an unsigned byte to the <code>DataOutput</code>.
   *
   * @param value the unsigned byte to be written
   */
  inline void write(uint8_t value) {
    ensureCapacity(1);
    writeNoCheck(value);
  }

  /**
   * Write a signed byte to the <code>DataOutput</code>.
   *
   * @param value the signed byte to be written
   */
  inline void write(int8_t value) { write(static_cast<uint8_t>(value)); }

  /**
   * Write a boolean value to the <code>DataOutput</code>.
   *
   * @param value the boolean value to be written
   */
  inline void writeBoolean(bool value) { write(static_cast<uint8_t>(value)); }

  /**
   * Write an array of unsigned bytes to the <code>DataOutput</code>.
   *
   * @param value the array of unsigned bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  inline void writeBytes(const uint8_t* bytes, int32_t len) {
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

  /**
   * Write an array of signed bytes to the <code>DataOutput</code>.
   *
   * @param value the array of signed bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  inline void writeBytes(const int8_t* bytes, int32_t len) {
    writeBytes(reinterpret_cast<const uint8_t*>(bytes), len);
  }

  /**
   * Write an array of unsigned bytes without its length to the
   * <code>DataOutput</code>.
   * @remarks The difference between this and <code>writeBytes</code> is that
   *   this does write the length of bytes so the corresponding
   *   <code>DataInput::readBytesOnly</code> (unlike
   *   <code>DataInput::readBytes</code>) needs the length argument explicitly.
   *
   * @param value the array of unsigned bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  inline void writeBytesOnly(const uint8_t* bytes, uint32_t len) {
    ensureCapacity(len);
    std::memcpy(m_buf, bytes, len);
    m_buf += len;
  }

  /**
   * Write an array of signed bytes without its length to the
   * <code>DataOutput</code>.
   * @remarks The difference between this and <code>writeBytes</code> is that
   *   this does write the length of bytes so the corresponding
   *   <code>DataInput::readBytesOnly</code> (unlike
   *   <code>DataInput::readBytes</code>) needs the length argument explicitly.
   *
   * @param value the array of signed bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  inline void writeBytesOnly(const int8_t* bytes, uint32_t len) {
    writeBytesOnly(reinterpret_cast<const uint8_t*>(bytes), len);
  }

  /**
   * Write a 16-bit unsigned integer value to the <code>DataOutput</code>.
   *
   * @param value the 16-bit unsigned integer value to be written
   */
  inline void writeInt(uint16_t value) {
    ensureCapacity(2);
    *(m_buf++) = static_cast<uint8_t>(value >> 8);
    *(m_buf++) = static_cast<uint8_t>(value);
  }

  /**
   * Write a 16-bit Char (wchar_t) value to the <code>DataOutput</code>.
   *
   * @param value the 16-bit wchar_t value to be written
   */
  inline void writeChar(uint16_t value) {
    ensureCapacity(2);
    *(m_buf++) = static_cast<uint8_t>(value >> 8);
    *(m_buf++) = static_cast<uint8_t>(value);
  }

  /**
   * Write a 32-bit unsigned integer value to the <code>DataOutput</code>.
   *
   * @param value the 32-bit unsigned integer value to be written
   */
  inline void writeInt(uint32_t value) {
    ensureCapacity(4);
    *(m_buf++) = static_cast<uint8_t>(value >> 24);
    *(m_buf++) = static_cast<uint8_t>(value >> 16);
    *(m_buf++) = static_cast<uint8_t>(value >> 8);
    *(m_buf++) = static_cast<uint8_t>(value);
  }

  /**
   * Write a 64-bit unsigned integer value to the <code>DataOutput</code>.
   *
   * @param value the 64-bit unsigned integer value to be written
   */
  inline void writeInt(uint64_t value) {
    ensureCapacity(8);
    // the defines are not reliable and can be changed by compiler options.
    // Hence using sizeof() test instead.
    //#if defined(_LP64) || ( defined(__WORDSIZE) && __WORDSIZE == 64 ) ||
    //( defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64 )
    if (sizeof(long) == 8) {
      *(m_buf++) = static_cast<uint8_t>(value >> 56);
      *(m_buf++) = static_cast<uint8_t>(value >> 48);
      *(m_buf++) = static_cast<uint8_t>(value >> 40);
      *(m_buf++) = static_cast<uint8_t>(value >> 32);
      *(m_buf++) = static_cast<uint8_t>(value >> 24);
      *(m_buf++) = static_cast<uint8_t>(value >> 16);
      *(m_buf++) = static_cast<uint8_t>(value >> 8);
      *(m_buf++) = static_cast<uint8_t>(value);
    } else {
      uint32_t hword = static_cast<uint32_t>(value >> 32);
      *(m_buf++) = static_cast<uint8_t>(hword >> 24);
      *(m_buf++) = static_cast<uint8_t>(hword >> 16);
      *(m_buf++) = static_cast<uint8_t>(hword >> 8);
      *(m_buf++) = static_cast<uint8_t>(hword);

      hword = static_cast<uint32_t>(value);
      *(m_buf++) = static_cast<uint8_t>(hword >> 24);
      *(m_buf++) = static_cast<uint8_t>(hword >> 16);
      *(m_buf++) = static_cast<uint8_t>(hword >> 8);
      *(m_buf++) = static_cast<uint8_t>(hword);
    }
  }

  /**
   * Write a 16-bit signed integer value to the <code>DataOutput</code>.
   *
   * @param value the 16-bit signed integer value to be written
   */
  inline void writeInt(int16_t value) {
    writeInt(static_cast<uint16_t>(value));
  }

  /**
   * Write a 32-bit signed integer value to the <code>DataOutput</code>.
   *
   * @param value the 32-bit signed integer value to be written
   */
  inline void writeInt(int32_t value) {
    writeInt(static_cast<uint32_t>(value));
  }

  /**
   * Write a 64-bit signed integer value to the <code>DataOutput</code>.
   *
   * @param value the 64-bit signed integer value to be written
   */
  inline void writeInt(int64_t value) {
    writeInt(static_cast<uint64_t>(value));
  }

  /**
   * Write a 32-bit signed integer array length value to the
   * <code>DataOutput</code> in a manner compatible with java server's
   * <code>DataSerializer.writeArrayLength</code>.
   *
   * @param value the 32-bit signed integer array length to be written
   */
  inline void writeArrayLen(int32_t len) {
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

  /**
   * Write a float value to the <code>DataOutput</code>.
   *
   * @param value the float value to be written
   */
  inline void writeFloat(float value) {
    union float_uint32_t {
      float f;
      uint32_t u;
    } v;
    v.f = value;
    writeInt(v.u);
  }

  /**
   * Write a double precision real number to the <code>DataOutput</code>.
   *
   * @param value the double precision real number to be written
   */
  inline void writeDouble(double value) {
    union double_uint64_t {
      double d;
      uint64_t ll;
    } v;
    v.d = value;
    writeInt(v.ll);
  }

  template <class CharT, class... Tail>
  inline void writeString(const std::basic_string<CharT, Tail...>& value) {
    // without scanning string, making worst case choices.
    if (value.length() / 3 <= std::numeric_limits<uint16_t>::max()) {
      write(static_cast<uint8_t>(GeodeTypeIds::CacheableString));
      writeJavaModifiedUtf8(value);
    } else {
      write(static_cast<uint8_t>(GeodeTypeIds::CacheableStringHuge));
      writeUtf16Huge(value);
    }
  }

  /**
   * Writes the given ASCII string supporting maximum length of 64K
   * (i.e. unsigned 16-bit integer).
   * @remarks The string will be truncated if greater than the maximum
   *   permissible length of 64K. Use <code>writeBytes</code> or
   *   <code>writeASCIIHuge</code> to write ASCII strings of length larger
   *   than this.
   *
   * @param value the C string to be written
   * @param length the number of characters from start of string to be
   *   written; the default value of 0 implies the complete string
   */
  inline void writeASCII(const char* value, uint32_t length = 0) {
    if (value != nullptr) {
      if (length == 0) {
        length = static_cast<uint32_t>(strlen(value));
      }
      uint16_t len = static_cast<uint16_t>(length > 0xFFFF ? 0xFFFF : length);
      writeInt(len);
      writeBytesOnly((int8_t*)value, len);  // K64
    } else {
      writeInt(static_cast<uint16_t>(0));
    }
  }

  inline void writeNativeString(const char* value) {
    // create cacheable string
    // write typeid id.
    // call todata
    auto csPtr = CacheableString::create(value);
    write(csPtr->typeId());
    csPtr->toData(*this);
  }

  /**
   * Writes the given ASCII string supporting upto maximum 32-bit
   * integer value.
   * @remarks Use this to write large ASCII strings. The other
   *   <code>writeASCII</code> method will truncate strings greater than
   *   64K in size.
   *
   * @param value the wide-character string to be written
   * @param length the number of characters from start of string to be
   *   written; the default value of 0 implies the complete string
   */
  inline void writeASCIIHuge(const char* value, uint32_t length = 0) {
    if (value != nullptr) {
      if (length == 0) {
        length = static_cast<uint32_t>(strlen(value));
      }
      writeInt(length);
      writeBytesOnly((int8_t*)value, length);
    } else {
      writeInt(static_cast<uint32_t>(0));
    }
  }

  /**
   * Writes the given given string using java modified UTF-8 encoding
   * supporting maximum encoded length of 64K (i.e. unsigned 16-bit integer).
   * @remarks The string will be truncated if greater than the maximum
   *   permissible length of 64K. Use <code>writeUTFHuge</code> to write
   *   strings of length larger than this.
   *
   * @param value the C string to be written
   * @param length the number of characters from start of string to be
   *   written; the default value of 0 implies the complete string
   */
  inline void writeUTF(const char* value, uint32_t length = 0) {
    if (value != nullptr) {
      int32_t len = getEncodedLength(value, length);
      uint16_t encodedLen = static_cast<uint16_t>(len > 0xFFFF ? 0xFFFF : len);
      writeInt(encodedLen);
      ensureCapacity(encodedLen);
      uint8_t* end = m_buf + encodedLen;
      while (m_buf < end) {
        encodeChar(*value++);
      }
      if (m_buf > end) m_buf = end;
    } else {
      writeInt(static_cast<uint16_t>(0));
    }
  }

  /**
   * Writes the given string using UTF-16 encoding.
   * @remarks Use this to write large strings. The other
   *   <code>writeUTF</code> method will truncate strings greater than
   *   64K in size.
   *
   * @param value the C string to be written
   * @param length the number of characters from start of string to be
   *   written; the default value of 0 implies the complete string
   *   assuming a null terminated string; do not use this unless sure
   *   that the UTF string does not contain any null characters
   */
  inline void writeUTFHuge(const char* value, uint32_t length = 0) {
    if (value != nullptr) {
      if (length == 0) {
        length = static_cast<uint32_t>(strlen(value));
      }
      writeInt(length);
      ensureCapacity(length * 2);
      for (uint32_t pos = 0; pos < length; pos++) {
        writeNoCheck(static_cast<int8_t>(0));
        writeNoCheck(static_cast<int8_t>(value[pos]));
      }
    } else {
      writeInt(static_cast<uint32_t>(0));
    }
  }

  /**
   * Writes the given given string using java modified UTF-8 encoding
   * supporting maximum encoded length of 64K (i.e. unsigned 16-bit integer).
   * @remarks The string will be truncated if greater than the maximum
   *   permissible length of 64K. Use <code>writeUTFHuge</code> to write
   *   strings of length larger than this.
   *
   * @param value the wide-character string to be written
   * @param length the number of characters from start of string to be
   *   written; the default value of 0 implies the complete string
   */
  inline void writeUTF(const wchar_t* value, uint32_t length = 0) {
    if (value != nullptr) {
      int32_t len = getEncodedLength(value, length);
      uint16_t encodedLen = static_cast<uint16_t>(len > 0xFFFF ? 0xFFFF : len);
      writeInt(encodedLen);
      ensureCapacity(encodedLen);
      uint8_t* end = m_buf + encodedLen;
      while (m_buf < end) {
        encodeChar(*value++);
      }
      if (m_buf > end) m_buf = end;
    } else {
      writeInt(static_cast<uint16_t>(0));
    }
  }

  /**
   * Writes the given string using UTF-16 encoding.
   * @remarks Use this to write large strings. The other
   *   <code>writeUTF</code> method will truncate strings greater than
   *   64K in size.
   *
   * @param value the wide-character string to be written
   * @param length the number of characters from start of string to be
   *   written; the default value of 0 implies the complete string
   */
  inline void writeUTFHuge(const wchar_t* value, uint32_t length = 0) {
    if (value != nullptr) {
      if (length == 0) {
        length = static_cast<uint32_t>(wcslen(value));
      }
      writeInt(length);
      ensureCapacity(length * 2);
      for (uint32_t pos = 0; pos < length; pos++) {
        uint16_t item = static_cast<uint16_t>(value[pos]);
        writeNoCheck(static_cast<uint8_t>((item & 0xFF00) >> 8));
        writeNoCheck(static_cast<uint8_t>(item & 0xFF));
      }
    } else {
      writeInt(static_cast<uint32_t>(0));
    }
  }

  /**
   * Get the length required to represent a given ASCII character string in
   * java modified UTF-8 format.
   *
   * @param value The C string.
   * @param length The length of the string; or zero to use the full string.
   * @return The length required for representation in java modified
   *         UTF-8 format.
   * @see DataInput::getDecodedLength
   */
  inline static int32_t getEncodedLength(const char* value, int32_t length = 0,
                                         uint32_t* valLength = nullptr) {
    if (value == nullptr) return 0;
    char currentChar;
    int32_t encodedLen = 0;
    const char* start = value;
    if (length == 0) {
      while ((currentChar = *value) != '\0') {
        getEncodedLength(currentChar, encodedLen);
        value++;
      }
    } else {
      const char* end = value + length;
      while (value < end) {
        currentChar = *value;
        getEncodedLength(currentChar, encodedLen);
        value++;
      }
    }
    if (valLength != nullptr) {
      *valLength = static_cast<uint32_t>(value - start);
    }
    return encodedLen;
  }

  /**
   * Get the length required to represent a given wide-character string in
   * java modified UTF-8 format.
   *
   * @param value The wide-character string.
   * @param length The length of the string.
   * @return The length required for representation in java modified
   *         UTF-8 format.
   * @see DataInput::getDecodedLength
   */
  inline static int32_t getEncodedLength(const wchar_t* value,
                                         int32_t length = 0,
                                         uint32_t* valLength = nullptr) {
    if (value == nullptr) return 0;
    wchar_t currentChar;
    int32_t encodedLen = 0;
    const wchar_t* start = value;
    if (length == 0) {
      while ((currentChar = *value) != 0) {
        getEncodedLength(currentChar, encodedLen);
        value++;
      }
    } else {
      const wchar_t* end = value + length;
      while (value < end) {
        currentChar = *value;
        getEncodedLength(currentChar, encodedLen);
        value++;
      }
    }
    if (valLength != nullptr) {
      *valLength = static_cast<uint32_t>(value - start);
    }
    return encodedLen;
  }

  /**
   * Write a <code>Serializable</code> object to the <code>DataOutput</code>.
   *
   * @param objptr smart pointer to the <code>Serializable</code> object
   *   to be written
   */
  template <class PTR>
  void writeObject(const std::shared_ptr<PTR>& objptr, bool isDelta = false) {
    writeObjectInternal(objptr.get(), isDelta);
  }

  /**
   * Write a <code>Serializable</code> object to the <code>DataOutput</code>.
   *
   * @param objptr pointer to the <code>Serializable</code> object
   *   to be written
   */
  void writeObject(const Serializable* objptr) { writeObjectInternal(objptr); }

  /**
   * Get an internal pointer to the current location in the
   * <code>DataOutput</code> byte array.
   */
  const uint8_t* getCursor() { return m_buf; }

  /**
   * Advance the buffer cursor by the given offset.
   *
   * @param offset the offset by which to advance the cursor
   */
  void advanceCursor(uint32_t offset) {
    ensureCapacity(offset);
    m_buf += offset;
  }

  /**
   * Rewind the buffer cursor by the given offset.
   *
   * @param offset the offset by which to rewind the cursor
   */
  void rewindCursor(uint32_t offset) { m_buf -= offset; }

  void updateValueAtPos(uint32_t offset, uint8_t value) {
    m_bytes[offset] = value;
  }

  uint8_t getValueAtPos(uint32_t offset) { return m_bytes[offset]; }
  /** Destruct a DataOutput, including releasing the created buffer. */
  ~DataOutput() {
    reset();
    DataOutput::checkinBuffer(m_bytes, m_size);
  }

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   */
  inline const uint8_t* getBuffer() const {
    // GF_R_ASSERT(!((uint32_t)(m_bytes) % 4));
    return m_bytes;
  }

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   */
  inline uint32_t getRemainingBufferLength() const {
    // GF_R_ASSERT(!((uint32_t)(m_bytes) % 4));
    return m_size - getBufferLength();
  }

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   *
   * @param rsize the size of buffer is filled in this output parameter;
   *   should not be nullptr
   */
  inline const uint8_t* getBuffer(uint32_t* rsize) const {
    *rsize = static_cast<uint32_t>(m_buf - m_bytes);
    // GF_R_ASSERT(!((uint32_t)(m_bytes) % 4));
    return m_bytes;
  }

  inline uint8_t* getBufferCopy() {
    uint32_t size = static_cast<uint32_t>(m_buf - m_bytes);
    uint8_t* result;
    result = (uint8_t*)std::malloc(size * sizeof(uint8_t));
    if (result == nullptr) {
      throw OutOfMemoryException("Out of Memory while resizing buffer");
    }
    std::memcpy(result, m_bytes, size);
    return result;
  }

  /**
   * Get the length of current data in the internal buffer of
   * <code>DataOutput</code>.
   */
  inline uint32_t getBufferLength() const {
    return static_cast<uint32_t>(m_buf - m_bytes);
  }

  /**
   * Reset the internal cursor to the start of the buffer.
   */
  inline void reset() {
    if (m_haveBigBuffer) {
      // free existing buffer
      std::free(m_bytes);
      // create smaller buffer
      m_bytes = (uint8_t*)std::malloc(m_lowWaterMark * sizeof(uint8_t));
      if (m_bytes == nullptr) {
        throw OutOfMemoryException("Out of Memory while resizing buffer");
      }
      m_size = m_lowWaterMark;
      // reset the flag
      m_haveBigBuffer = false;
      // release the lock
      releaseLock();
    }
    m_buf = m_bytes;
  }

  // make sure there is room left for the requested size item.
  inline void ensureCapacity(uint32_t size) {
    uint32_t offset = static_cast<uint32_t>(m_buf - m_bytes);
    if ((m_size - offset) < size) {
      uint32_t newSize = m_size * 2 + (8192 * (size / 8192));
      if (newSize >= m_highWaterMark && !m_haveBigBuffer) {
        // acquire the lock
        acquireLock();
        // set flag
        m_haveBigBuffer = true;
      }
      m_size = newSize;

      auto tmp = (uint8_t*)std::realloc(m_bytes, m_size * sizeof(uint8_t));
      if (tmp == nullptr) {
        throw OutOfMemoryException("Out of Memory while resizing buffer");
      }
      m_bytes = tmp;
      m_buf = m_bytes + offset;
    }
  }

  uint8_t* getBufferCopyFrom(const uint8_t* from, uint32_t length) {
    uint8_t* result;
    GF_NEW(result, uint8_t[length]);
    std::memcpy(result, from, length);

    return result;
  }

  static void safeDelete(uint8_t* src) { GF_SAFE_DELETE(src); }

  virtual const Cache* getCache();

 protected:
  /**
   * Construct a new DataOutput.
   */
  DataOutput(const CacheImpl* cache);

  DataOutput() : DataOutput(nullptr) {}

  virtual const SerializationRegistry& getSerializationRegistry() const;

 private:
  void writeObjectInternal(const Serializable* ptr, bool isDelta = false);

  static void acquireLock();
  static void releaseLock();

  // memory m_buffer to encode to.
  uint8_t* m_bytes;
  // cursor.
  uint8_t* m_buf;
  // size of m_bytes.
  uint32_t m_size;
  // high and low water marks for buffer size
  static uint32_t m_lowWaterMark;
  static uint32_t m_highWaterMark;
  // flag to indicate we have a big buffer
  volatile bool m_haveBigBuffer;
  const CacheImpl* m_cache;
  std::reference_wrapper<const std::string> m_poolName;

  inline void writeAscii(const std::string& value) {
    uint16_t len = static_cast<uint16_t>(
        std::min<size_t>(value.length(), std::numeric_limits<uint16_t>::max()));
    writeInt(len);
    for (size_t i = 0; i < len; i++) {
      // blindly assumes ascii so mask off only 7 bits
      write(static_cast<int8_t>(value.data()[i] & 0x7F));
    }
  }

  inline void writeAsciiHuge(const std::string& value) {
    uint32_t len = static_cast<uint32_t>(
        std::min<size_t>(value.length(), std::numeric_limits<uint32_t>::max()));
    writeInt(static_cast<uint32_t>(len));
    for (size_t i = 0; i < len; i++) {
      // blindly assumes ascii so mask off only 7 bits
      write(static_cast<int8_t>(value.data()[i] & 0x7F));
    }
  }

  void writeJavaModifiedUtf8(const std::u16string& value);

  void writeJavaModifiedUtf8(const std::string& value);

  void writeUtf16Huge(const std::string& value);

  inline void writeUtf16Huge(const std::u16string& value) {
    uint32_t len = static_cast<uint32_t>(
        std::min<size_t>(value.length(), std::numeric_limits<uint32_t>::max()));
    writeInt(len);
    ensureCapacity(len * 2);
    for (size_t i = 0; i < len; i++) {
      writeNoCheck(static_cast<uint8_t>(value.data()[i] >> 8));
      writeNoCheck(static_cast<uint8_t>(value.data()[i]));
    }
  }

  inline static size_t getJavaModifiedUtf8EncodedLength(
      const std::u16string& value) {
    size_t encodedLen = 0;
    for (const auto c : value) {
      if (c == 0) {
        // NUL
        encodedLen += 2;
      } else if (c < 0x80) {
        // ASCII
        encodedLen++;
      } else if (c < 0x800) {
        encodedLen += 2;
      } else {
        encodedLen += 3;
      }
    }
    return encodedLen;
  }

  inline static void getEncodedLength(const char val, int32_t& encodedLen) {
    if ((val == 0) || (val & 0x80)) {
      // two byte.
      encodedLen += 2;
    } else {
      // one byte.
      encodedLen++;
    }
  }

  inline static void getEncodedLength(const wchar_t val, int32_t& encodedLen) {
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

  inline void encodeChar(const char value) {
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

  // this will lose the character set encoding.
  inline void encodeChar(const wchar_t value) {
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

  inline void encodeJavaModifiedUtf8(const char16_t c) {
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

  inline void writeNoCheck(uint8_t value) { *(m_buf++) = value; }

  inline void writeNoCheck(int8_t value) {
    writeNoCheck(static_cast<uint8_t>(value));
  }

  const std::string& getPoolName() const { return m_poolName; }

  void setPoolName(const std::string& poolName) {
    m_poolName = std::ref(poolName);
  }

  static uint8_t* checkoutBuffer(uint32_t* size);
  static void checkinBuffer(uint8_t* buffer, uint32_t size);

  // disable copy constructor and assignment
  DataOutput(const DataOutput&);
  DataOutput& operator=(const DataOutput&);

  friend Cache;
  friend CacheImpl;
  friend DataOutputInternal;
  friend CacheableString;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATAOUTPUT_H_
