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

#ifndef GEODE_DATAINPUT_H_
#define GEODE_DATAINPUT_H_

#include <cstring>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "internal/geode_globals.hpp"
#include "ExceptionTypes.hpp"
#include "GeodeTypeIds.hpp"
#include "ExceptionTypes.hpp"

/**
 * @file
 */

#define _GEODE_CHECK_BUFFER_SIZE(x) _checkBufferSize(x, __LINE__)

namespace apache {
namespace geode {
namespace client {

class Cache;
class CacheableString;
class DataInput;
class Serializable;
class SerializationRegistry;
class DataInputInternal;
class CacheImpl;
class DataInputInternal;

/**
 * Provide operations for reading primitive data values, byte arrays,
 * strings, <code>Serializable</code> objects from a byte stream.
 * This class is intentionally not thread safe.
 * @remarks None of the output parameters in the methods below can be nullptr
 *   unless otherwise noted.
 */
class _GEODE_EXPORT DataInput {
 public:
  /**
   * Read a signed byte from the <code>DataInput</code>.
   *
   * @@return signed byte read from stream
   */
  inline int8_t read() {
    _GEODE_CHECK_BUFFER_SIZE(1);
    return readNoCheck();
  }

  /**
   * Read a boolean value from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the boolean read from stream
   */
  inline bool readBoolean() {
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
  inline void readBytesOnly(uint8_t* buffer, size_t len) {
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
  inline void readBytesOnly(int8_t* buffer, size_t len) {
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
  inline void readBytes(uint8_t** bytes, int32_t* len) {
    int32_t length = readArrayLen();
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

  /**
   * Read an array of signed bytes from the <code>DataInput</code>
   * expecting to find the length of array in the stream at the start.
   * @remarks This method is complimentary to
   *   <code>DataOutput::writeBytes</code>.
   *
   * @param bytes output array to hold the bytes read from stream; the array
   *   is allocated by this method
   * @param len output parameter to hold the length of array read from stream
   */
  inline void readBytes(int8_t** bytes, int32_t* len) {
    int32_t length = readArrayLen();
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

  /**
   * Read a 16-bit signed integer from the <code>DataInput</code>.
   *
   * @return 16-bit signed integer read from stream
   */
  inline int16_t readInt16() {
    _GEODE_CHECK_BUFFER_SIZE(2);
    return readInt16NoCheck();
  }

  /**
   * Read a 32-bit signed integer from the <code>DataInput</code>.g
   *
   * @param value output parameter to hold the 32-bit signed integer
   *   read from stream
   */
  inline int32_t readInt32() {
    _GEODE_CHECK_BUFFER_SIZE(4);
    int32_t tmp = *(m_buf++);
    tmp = (tmp << 8) | *(m_buf++);
    tmp = (tmp << 8) | *(m_buf++);
    tmp = (tmp << 8) | *(m_buf++);
    return tmp;
  }

  /**
   * Read a 64-bit signed integer from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the 64-bit signed integer
   *   read from stream
   */
  inline int64_t readInt64() {
    _GEODE_CHECK_BUFFER_SIZE(8);
    int64_t tmp;
    if (sizeof(long) == 8) {
      tmp = *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
      tmp = (tmp << 8) | *(m_buf++);
    } else {
      uint32_t hword = *(m_buf++);
      hword = (hword << 8) | *(m_buf++);
      hword = (hword << 8) | *(m_buf++);
      hword = (hword << 8) | *(m_buf++);

      tmp = hword;
      hword = *(m_buf++);
      hword = (hword << 8) | *(m_buf++);
      hword = (hword << 8) | *(m_buf++);
      hword = (hword << 8) | *(m_buf++);
      tmp = (tmp << 32) | hword;
    }
    return tmp;
  }

  /**
   * Read a 32-bit signed integer array length value from the
   * <code>DataInput</code> in a manner compatible with java server's
   * <code>DataSerializer.readArrayLength</code>.
   *
   * @param len output parameter to hold the 32-bit signed length
   *   read from stream
   */
  inline int32_t readArrayLen() {
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

  /**
   * Decode a 64 bit integer as a variable length array.
   *
   * This is taken from the varint encoding in protobufs (BSD licensed).
   * See https://developers.google.com/protocol-buffers/docs/encoding
   */
  inline int64_t readUnsignedVL() {
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

  /**
   * Read a float from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the float read from stream
   */
  inline float readFloat() {
    _GEODE_CHECK_BUFFER_SIZE(4);
    union float_uint32_t {
      float f;
      uint32_t u;
    } v;
    v.u = readInt32();
    return v.f;
  }

  /**
   * Read a double precision number from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the double precision number
   *   read from stream
   */
  inline double readDouble() {
    _GEODE_CHECK_BUFFER_SIZE(8);
    union double_uint64_t {
      double d;
      uint64_t ll;
    } v;
    v.ll = readInt64();
    return v.d;
  }

  template <class CharT = char, class... Tail>
  inline std::basic_string<CharT, Tail...> readUTF() {
    std::basic_string<CharT, Tail...> value;
    readJavaModifiedUtf8(value);
    return value;
  }

  template <class CharT = char, class... Tail>
  inline std::basic_string<CharT, Tail...> readString() {
    std::basic_string<CharT, Tail...> value;
    const uint8_t type = read();
    switch (type) {
      case GeodeTypeIds::CacheableString:
        readJavaModifiedUtf8(value);
        break;
      case GeodeTypeIds::CacheableStringHuge:
        readUtf16Huge(value);
        break;
      case GeodeTypeIds::CacheableASCIIString:
        readAscii(value);
        break;
      case GeodeTypeIds::CacheableASCIIStringHuge:
        readAsciiHuge(value);
        break;
      case GeodeTypeIds::CacheableNullString:
        // empty string
        break;
    }
    return value;
  }

  inline bool readNativeBool() {
    read();  // ignore type id

    return readBoolean();
  }

  inline int32_t readNativeInt32() {
    read();  // ignore type id
    return readInt32();
  }

  inline std::shared_ptr<Serializable> readDirectObject(int8_t typeId = -1) {
    return readObjectInternal(typeId);
  }

  /**
   * Read a Serializable object from the DataInput.
   *
   * @return Serializable object or <code>nullptr</code>.
   */
  inline std::shared_ptr<Serializable> readObject() {
    return readObjectInternal();
  }

  /**
   * Read a <code>Serializable</code> object from the <code>DataInput</code>.
   * Null objects are handled.
   */
  inline void readObject(std::shared_ptr<Serializable>& ptr) {
    ptr = readObjectInternal();
  }

  inline void readObject(char16_t* value) { *value = readInt16(); }

  inline void readObject(bool* value) { *value = readBoolean(); }

  inline void readObject(int8_t* value) { *value = read(); }

  inline void readObject(int16_t* value) { *value = readInt16(); }

  inline void readObject(int32_t* value) { *value = readInt32(); }

  inline void readObject(int64_t* value) { *value = readInt64(); }

  inline void readObject(float* value) { *value = readFloat(); }

  inline void readObject(double* value) { *value = readDouble(); }

  inline void readCharArray(char16_t** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readBooleanArray(bool** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readByteArray(int8_t** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readShortArray(int16_t** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readIntArray(int32_t** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readLongArray(int64_t** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readFloatArray(float** value, int32_t& length) {
    readObject(value, length);
  }

  inline void readDoubleArray(double** value, int32_t& length) {
    readObject(value, length);
  }

  inline std::vector<std::string> readStringArray() {
    std::vector<std::string> value;

    int32_t arrLen = readArrayLen();
    if (arrLen > 0) {
      value.reserve(arrLen);
      for (int i = 0; i < arrLen; i++) {
        value.push_back(readString());
      }
    }

    return value;
  }

  inline void readArrayOfByteArrays(int8_t*** arrayofBytearr,
                                    int32_t& arrayLength,
                                    int32_t** elementLength) {
    int32_t arrLen = readArrayLen();
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

  /**
   * Get the length required to represent a given UTF-8 encoded string
   * (created using {@link DataOutput::writeUTF} or
   * <code>java.io.DataOutput.writeUTF</code>) in wide-character format.
   *
   * @param value The UTF-8 encoded stream.
   * @param length The length of the stream to be read.
   * @return The length of the decoded string.
   * @see DataOutput::getEncodedLength
   */
  static int32_t getDecodedLength(const uint8_t* value, int32_t length) {
    const uint8_t* end = value + length;
    int32_t decodedLen = 0;
    while (value < end) {
      // get next byte unsigned
      int32_t b = *value++ & 0xff;
      int32_t k = b >> 5;
      // classify based on the high order 3 bits
      switch (k) {
        case 6: {
          value++;
          break;
        }
        case 7: {
          value += 2;
          break;
        }
        default:
          break;
      }
      decodedLen += 1;
    }
    if (value > end) decodedLen--;
    return decodedLen;
  }

  /** destructor */
  ~DataInput() {}

  /**
   * Get the pointer to current buffer position. This should be treated
   * as readonly and modification of contents using this internal pointer
   * has undefined behavior.
   */
  inline const uint8_t* currentBufferPosition() const { return m_buf; }

  /** get the number of bytes read in the buffer */
  inline size_t getBytesRead() const { return m_buf - m_bufHead; }

  /** get the number of bytes remaining to be read in the buffer */
  inline size_t getBytesRemaining() const {
    return (m_bufLength - getBytesRead());
  }

  /** advance the cursor by given offset */
  inline void advanceCursor(size_t offset) { m_buf += offset; }

  /** rewind the cursor by given offset */
  inline void rewindCursor(size_t offset) { m_buf -= offset; }

  /** reset the cursor to the start of buffer */
  inline void reset() { m_buf = m_bufHead; }

  inline void setBuffer() {
    m_buf = currentBufferPosition();
    m_bufLength = getBytesRemaining();
  }

  inline void resetPdx(size_t offset) { m_buf = m_bufHead + offset; }

  inline size_t getPdxBytes() const { return m_bufLength; }

  static uint8_t* getBufferCopy(const uint8_t* from, size_t length) {
    uint8_t* result;
    _GEODE_NEW(result, uint8_t[length]);
    std::memcpy(result, from, length);

    return result;
  }

  inline void reset(size_t offset) { m_buf = m_bufHead + offset; }

  uint8_t* getBufferCopyFrom(const uint8_t* from, size_t length) {
    uint8_t* result;
    _GEODE_NEW(result, uint8_t[length]);
    std::memcpy(result, from, length);

    return result;
  }

  virtual const Cache* getCache();

 protected:
  /** constructor given a pre-allocated byte array with size */
  DataInput(const uint8_t* m_buffer, size_t len, const CacheImpl* cache)
      : m_buf(m_buffer),
        m_bufHead(m_buffer),
        m_bufLength(len),
        m_poolName(EMPTY_STRING),
        m_cache(cache) {}

  virtual const SerializationRegistry& getSerializationRegistry() const;

 private:
  const uint8_t* m_buf;
  const uint8_t* m_bufHead;
  size_t m_bufLength;
  std::reference_wrapper<const std::string> m_poolName;
  const CacheImpl* m_cache;

  std::shared_ptr<Serializable> readObjectInternal(int8_t typeId = -1);

  template <typename mType>
  void readObject(mType** value, int32_t& length) {
    int arrayLen = readArrayLen();
    length = arrayLen;
    mType* objArray;
    if (arrayLen > 0) {
      objArray = new mType[arrayLen];
      int i = 0;
      for (i = 0; i < arrayLen; i++) {
        mType tmp = 0;
        readObject(&tmp);
        objArray[i] = tmp;  //*value[i] = tmp;
      }
      *value = objArray;
    }
  }

  inline char readPdxChar() { return static_cast<char>(readInt16()); }

  inline void _checkBufferSize(size_t size, int32_t line) {
    if ((m_bufLength - (m_buf - m_bufHead)) < size) {
      throw OutOfRangeException(
          "DataInput: attempt to read beyond buffer at line " +
          std::to_string(line) + ": available buffer size " +
          std::to_string(m_bufLength - (m_buf - m_bufHead)) +
          ", attempted read of size " + std::to_string(size));
    }
  }

  inline char16_t decodeJavaModifiedUtf8Char() {
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

  inline int8_t readNoCheck() { return *(m_buf++); }

  inline int16_t readInt16NoCheck() {
    int16_t tmp = *(m_buf++);
    tmp = static_cast<int16_t>((tmp << 8) | *(m_buf++));
    return tmp;
  }

  template <class CharT, class... Tail>
  inline void readAscii(std::basic_string<CharT, Tail...>& value,
                        size_t length) {
    _GEODE_CHECK_BUFFER_SIZE(length);
    value.reserve(length);
    while (length-- > 0) {
      // blindly assumes ASCII so mask off 7 bits
      value += readNoCheck() & 0x7F;
    }
  }

  template <class CharT, class... Tail>
  inline void readAscii(std::basic_string<CharT, Tail...>& value) {
    readAscii(value, static_cast<uint16_t>(readInt16()));
  }

  template <class CharT, class... Tail>
  inline void readAsciiHuge(std::basic_string<CharT, Tail...>& value) {
    readAscii(value, static_cast<uint32_t>(readInt32()));
  }

  template <class _CharT, class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<_CharT, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void readJavaModifiedUtf8(
      std::basic_string<char16_t, _Traits, _Allocator>& value) {
    uint16_t length = readInt16();
    _GEODE_CHECK_BUFFER_SIZE(length);
    value.reserve(length);
    const auto end = m_buf + length;
    while (m_buf < end) {
      value += decodeJavaModifiedUtf8Char();
    }
  }

  template <class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<char, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<char32_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void readJavaModifiedUtf8(
      std::basic_string<wchar_t, _Traits, _Allocator>& value) {
    // TODO string optimize
    typedef std::conditional<
        sizeof(wchar_t) == sizeof(char16_t), char16_t,
        std::conditional<sizeof(wchar_t) == sizeof(char32_t), char32_t,
                         char>::type>::type _WcharT;

    auto tmp = std::basic_string<_WcharT>();
    readJavaModifiedUtf8(tmp);
    value.assign(reinterpret_cast<const wchar_t*>(tmp.data()), tmp.length());
  }

  template <class _CharT, class _Traits, class _Allocator>
  void readUtf16Huge(std::basic_string<_CharT, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void readUtf16Huge(
      std::basic_string<char16_t, _Traits, _Allocator>& value) {
    uint32_t length = readInt32();
    _GEODE_CHECK_BUFFER_SIZE(length);
    value.reserve(length);
    while (length-- > 0) {
      value += readInt16NoCheck();
    }
  }

  template <class _Traits, class _Allocator>
  void readUtf16Huge(std::basic_string<char, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void readUtf16Huge(std::basic_string<char32_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  inline void readUtf16Huge(
      std::basic_string<wchar_t, _Traits, _Allocator>& value) {
    // TODO string optimize
    typedef std::conditional<
        sizeof(wchar_t) == sizeof(char16_t), char16_t,
        std::conditional<sizeof(wchar_t) == sizeof(char32_t), char32_t,
                         char>::type>::type _WcharT;

    auto tmp = std::basic_string<_WcharT>();
    readUtf16Huge(tmp);
    value.assign(reinterpret_cast<const wchar_t*>(tmp.data()), tmp.length());
  }

  const std::string& getPoolName() const { return m_poolName; }

  void setPoolName(const std::string& poolName) {
    m_poolName = std::ref(poolName);
  }

  // disable other constructors and assignment
  DataInput() = delete;
  DataInput(const DataInput&) = delete;
  DataInput& operator=(const DataInput&) = delete;

  friend Cache;
  friend CacheImpl;
  friend DataInputInternal;
  friend CacheableString;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATAINPUT_H_
