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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

#include "CacheableString.hpp"
#include "ExceptionTypes.hpp"
#include "Serializable.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class SerializationRegistry;
class DataOutputInternal;
class CacheImpl;
class Pool;
class TcrMessage;

/**
 * Provide operations for writing primitive data values, byte arrays,
 * strings, <code>Serializable</code> objects to a byte stream.
 * This class is intentionally not thread safe.
 */
class APACHE_GEODE_EXPORT DataOutput {
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
   * @param bytes the array of unsigned bytes to be written
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
   * @param bytes the array of signed bytes to be written
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
   * @param bytes the array of unsigned bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  inline void writeBytesOnly(const uint8_t* bytes, size_t len) {
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
   * @param bytes the array of signed bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  inline void writeBytesOnly(const int8_t* bytes, size_t len) {
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
    *(m_buf++) = static_cast<uint8_t>(value >> 56);
    *(m_buf++) = static_cast<uint8_t>(value >> 48);
    *(m_buf++) = static_cast<uint8_t>(value >> 40);
    *(m_buf++) = static_cast<uint8_t>(value >> 32);
    *(m_buf++) = static_cast<uint8_t>(value >> 24);
    *(m_buf++) = static_cast<uint8_t>(value >> 16);
    *(m_buf++) = static_cast<uint8_t>(value >> 8);
    *(m_buf++) = static_cast<uint8_t>(value);
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
   * @param len the 32-bit signed integer array length to be written
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

  template <class _CharT>
  inline void writeString(const _CharT* value) {
    // TODO string should we convert to empty string?
    if (nullptr == value) {
      write(static_cast<uint8_t>(DSCode::CacheableNullString));
    } else {
      writeString(std::basic_string<_CharT>(value));
    }
  }

  template <class _CharT, class... _Tail>
  inline void writeString(const std::basic_string<_CharT, _Tail...>& value) {
    // without scanning string, making worst case choices.
    // TODO constexp for each string type to jmutf8 length conversion
    if (value.length() * 3 <= (std::numeric_limits<uint16_t>::max)()) {
      write(static_cast<uint8_t>(DSCode::CacheableString));
      writeJavaModifiedUtf8(value);
    } else {
      write(static_cast<uint8_t>(DSCode::CacheableStringHuge));
      writeUtf16Huge(value);
    }
  }

  template <class _CharT>
  inline void writeUTF(const _CharT* value) {
    if (nullptr == value) {
      throw NullPointerException("Parameter value must not be null.");
    }
    writeUTF(std::basic_string<_CharT>(value));
  }

  template <class _CharT, class... Tail>
  inline void writeUTF(const std::basic_string<_CharT, Tail...>& value) {
    writeJavaModifiedUtf8(value);
  }

  /**
   * Writes a sequence of UTF-16 code units representing the given string value.
   * The output does not contain any length of termination charactes.
   *
   * @tparam _CharT matches character type of std::basic_string.
   * @tparam _Tail matches all remaining template parameters for
   * std::basic_string.
   * @param value string to write as UTF-16 units
   */
  template <class _CharT, class... _Tail>
  inline void writeChars(const std::basic_string<_CharT, _Tail...>& value) {
    writeUtf16(value);
  }

  /**
   * Writes a sequence of UTF-16 code units representing the given string value.
   * The output does not contain any length of termination charactes.
   *
   * Equivalent to:
   * @code
   * writeChars(std::basic_string<_CharT>(value));
   * @endcode
   *
   * @tparam _CharT matches character type used for std::basic_string.
   * @param value NULL (\u0000) terminated string to write as UTF-16 units
   */
  template <class _CharT>
  inline void writeChars(const _CharT* value) {
    writeChars(std::basic_string<_CharT>(value));
  }

  /**
   * Write a <code>Serializable</code> object to the <code>DataOutput</code>.
   *
   * @param objptr smart pointer to the <code>Serializable</code> object
   *   to be written
   */
  template <class PTR>
  void writeObject(const std::shared_ptr<PTR>& objptr, bool isDelta = false) {
    writeObjectInternal(objptr, isDelta);
  }

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
  void advanceCursor(size_t offset) {
    ensureCapacity(offset);
    m_buf += offset;
  }

  /**
   * Rewind the buffer cursor by the given offset.
   *
   * @param offset the offset by which to rewind the cursor
   */
  void rewindCursor(size_t offset) { m_buf -= offset; }

  void updateValueAtPos(size_t offset, uint8_t value) {
    m_bytes.get()[offset] = value;
  }

  uint8_t getValueAtPos(size_t offset) { return m_bytes.get()[offset]; }

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   */
  inline const uint8_t* getBuffer() const {
    // GF_R_ASSERT(!((uint32_t)(m_bytes) % 4));
    return m_bytes.get();
  }

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   */
  inline size_t getRemainingBufferLength() const {
    // GF_R_ASSERT(!((uint32_t)(m_bytes) % 4));
    return m_size - getBufferLength();
  }

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   *
   * @param rsize the size of buffer is filled in this output parameter;
   *   should not be nullptr
   */
  inline const uint8_t* getBuffer(size_t* rsize) const {
    *rsize = m_buf - m_bytes.get();
    // GF_R_ASSERT(!((uint32_t)(m_bytes) % 4));
    return m_bytes.get();
  }

  inline uint8_t* getBufferCopy() {
    size_t size = m_buf - m_bytes.get();
    auto result = static_cast<uint8_t*>(std::malloc(size * sizeof(uint8_t)));
    if (result == nullptr) {
      throw OutOfMemoryException("Out of Memory while resizing buffer");
    }
    std::memcpy(result, m_bytes.get(), size);
    return result;
  }

  /**
   * Get the length of current data in the internal buffer of
   * <code>DataOutput</code>.
   */
  inline size_t getBufferLength() const { return m_buf - m_bytes.get(); }

  /**
   * Reset the internal cursor to the start of the buffer.
   */
  inline void reset() {
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

  // make sure there is room left for the requested size item.
  inline void ensureCapacity(size_t size) {
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

  uint8_t* getBufferCopyFrom(const uint8_t* from, size_t length) {
    uint8_t* result;
    _GEODE_NEW(result, uint8_t[length]);
    std::memcpy(result, from, length);

    return result;
  }

  static void safeDelete(uint8_t* src) { _GEODE_SAFE_DELETE(src); }

  virtual Cache* getCache() const;

  DataOutput() = delete;

  /** Destruct a DataOutput, including releasing the created buffer. */
  virtual ~DataOutput() noexcept {
    reset();
    if (m_bytes) {
      DataOutput::checkinBuffer(m_bytes.release(), m_size);
    }
  }

  DataOutput(const DataOutput&) = delete;
  DataOutput& operator=(const DataOutput&) = delete;
  DataOutput(DataOutput&&) = default;
  DataOutput& operator=(DataOutput&&) = default;

 protected:
  /**
   * Construct a new DataOutput.
   */
  DataOutput(const CacheImpl* cache, Pool* pool);

  virtual const SerializationRegistry& getSerializationRegistry() const;

 private:
  void writeObjectInternal(const std::shared_ptr<Serializable>& ptr,
                           bool isDelta = false);

  static void acquireLock();
  static void releaseLock();

  struct FreeDeleter {
    void operator()(uint8_t* p) { free(p); }
  };

  // memory m_buffer to encode to.
  std::unique_ptr<uint8_t, FreeDeleter> m_bytes;
  // cursor.
  uint8_t* m_buf;
  // size of m_bytes.
  size_t m_size;
  // high and low water marks for buffer size
  static size_t m_lowWaterMark;
  static size_t m_highWaterMark;
  // flag to indicate we have a big buffer
  volatile bool m_haveBigBuffer;
  const CacheImpl* m_cache;
  Pool* m_pool;

  inline void writeAscii(const std::string& value) {
    uint16_t len = static_cast<uint16_t>(std::min<size_t>(
        value.length(), (std::numeric_limits<uint16_t>::max)()));
    writeInt(len);
    for (size_t i = 0; i < len; i++) {
      // blindly assumes ascii so mask off only 7 bits
      write(static_cast<int8_t>(value.data()[i] & 0x7F));
    }
  }

  inline void writeAsciiHuge(const std::string& value) {
    uint32_t len = static_cast<uint32_t>(std::min<size_t>(
        value.length(), (std::numeric_limits<uint32_t>::max)()));
    writeInt(static_cast<uint32_t>(len));
    for (size_t i = 0; i < len; i++) {
      // blindly assumes ascii so mask off only 7 bits
      write(static_cast<int8_t>(value.data()[i] & 0x7F));
    }
  }

  template <class _CharT, class _Traits, class _Allocator>
  void writeJavaModifiedUtf8(
      const std::basic_string<_CharT, _Traits, _Allocator>& value) {
    writeJavaModifiedUtf8(value.data(), value.length());
  }

  template <class _Traits, class _Allocator>
  void writeJavaModifiedUtf8(
      const std::basic_string<char, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void writeJavaModifiedUtf8(
      const std::basic_string<char32_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void writeJavaModifiedUtf8(
      const std::basic_string<wchar_t, _Traits, _Allocator>& value) {
    typedef std::conditional<
        sizeof(wchar_t) == sizeof(char16_t), char16_t,
        std::conditional<sizeof(wchar_t) == sizeof(char32_t), char32_t,
                         char>::type>::type _Convert;
    writeJavaModifiedUtf8(reinterpret_cast<const _Convert*>(value.data()),
                          value.length());
  }

  inline void writeJavaModifiedUtf8(const char16_t* data, size_t len) {
    if (0 == len) {
      writeInt(static_cast<uint16_t>(0));
    } else {
      auto encodedLen = static_cast<uint16_t>(
          std::min<size_t>(getJavaModifiedUtf8EncodedLength(data, len),
                           (std::numeric_limits<uint16_t>::max)()));
      writeInt(encodedLen);
      ensureCapacity(encodedLen);
      const auto end = m_buf + encodedLen;
      while (m_buf < end) {
        encodeJavaModifiedUtf8(*data++);
      }
      if (m_buf > end) m_buf = end;
    }
  }

  void writeJavaModifiedUtf8(const char32_t* data, size_t len);

  template <class _CharT, class _Traits, class _Allocator>
  inline void writeUtf16Huge(
      const std::basic_string<_CharT, _Traits, _Allocator>& value) {
    writeUtf16Huge(value.data(), value.length());
  }

  template <class _Traits, class _Allocator>
  void writeUtf16Huge(
      const std::basic_string<char, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void writeUtf16Huge(
      const std::basic_string<char32_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void writeUtf16Huge(
      const std::basic_string<wchar_t, _Traits, _Allocator>& value) {
    typedef std::conditional<
        sizeof(wchar_t) == sizeof(char16_t), char16_t,
        std::conditional<sizeof(wchar_t) == sizeof(char32_t), char32_t,
                         char>::type>::type _Convert;
    writeUtf16Huge(reinterpret_cast<const _Convert*>(value.data()),
                   value.length());
  }

  inline void writeUtf16Huge(const char16_t* data, size_t length) {
    uint32_t len = static_cast<uint32_t>(
        std::min<size_t>(length, (std::numeric_limits<uint32_t>::max)()));
    writeInt(len);
    writeUtf16(data, length);
  }

  void writeUtf16Huge(const char32_t* data, size_t len);

  template <class _CharT, class _Traits, class _Allocator>
  inline void writeUtf16(
      const std::basic_string<_CharT, _Traits, _Allocator>& value) {
    writeUtf16(value.data(), value.length());
  }

  template <class _Traits, class _Allocator>
  void writeUtf16(const std::basic_string<char, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void writeUtf16(
      const std::basic_string<char32_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void writeUtf16(
      const std::basic_string<wchar_t, _Traits, _Allocator>& value) {
    typedef std::conditional<
        sizeof(wchar_t) == sizeof(char16_t), char16_t,
        std::conditional<sizeof(wchar_t) == sizeof(char32_t), char32_t,
                         char>::type>::type _Convert;
    writeUtf16(reinterpret_cast<const _Convert*>(value.data()), value.length());
  }

  inline void writeUtf16(const char16_t* data, size_t length) {
    ensureCapacity(length * 2);
    for (; length > 0; length--, data++) {
      writeNoCheck(static_cast<uint8_t>(*data >> 8));
      writeNoCheck(static_cast<uint8_t>(*data));
    }
  }

  void writeUtf16(const char32_t* data, size_t len);

  static size_t getJavaModifiedUtf8EncodedLength(const char16_t* data,
                                                 size_t length);

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

  Pool* getPool() const { return m_pool; }

  static uint8_t* checkoutBuffer(size_t* size);
  static void checkinBuffer(uint8_t* buffer, size_t size);

  friend Cache;
  friend CacheImpl;
  friend DataOutputInternal;
  friend CacheableString;
  friend TcrMessage;
};

template void DataOutput::writeJavaModifiedUtf8(const std::u16string&);
// template void DataOutput::writeJavaModifiedUtf8(const std::u32string&);
template void DataOutput::writeJavaModifiedUtf8(const std::wstring&);

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATAOUTPUT_H_
