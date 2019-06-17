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

#include "ExceptionTypes.hpp"
#include "internal/DSCode.hpp"
#include "internal/geode_globals.hpp"

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
class CacheImpl;
class DataInputInternal;
class Pool;

/**
 * Provide operations for reading primitive data values, byte arrays,
 * strings, <code>Serializable</code> objects from a byte stream.
 * This class is intentionally not thread safe.
 * @remarks None of the output parameters in the methods below can be nullptr
 *   unless otherwise noted.
 */
class APACHE_GEODE_EXPORT DataInput {
 public:
  /**
   * Read a signed byte from the <code>DataInput</code>.
   *
   * @@return signed byte read from stream
   */
  int8_t read();

  /**
   * Read a boolean value from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the boolean read from stream
   */
  bool readBoolean();

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
  void readBytesOnly(uint8_t* buffer, size_t len);

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
  void readBytesOnly(int8_t* buffer, size_t len);

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
  void readBytes(uint8_t** bytes, int32_t* len);

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
  void readBytes(int8_t** bytes, int32_t* len);

  /**
   * Read a 16-bit signed integer from the <code>DataInput</code>.
   *
   * @return 16-bit signed integer read from stream
   */
  int16_t readInt16();

  /**
   * Read a 32-bit signed integer from the <code>DataInput</code>.g
   *
   * @param value output parameter to hold the 32-bit signed integer
   *   read from stream
   */
  int32_t readInt32();

  /**
   * Read a 64-bit signed integer from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the 64-bit signed integer
   *   read from stream
   */
  int64_t readInt64();

  /**
   * Read a 32-bit signed integer array length value from the
   * <code>DataInput</code> in a manner compatible with java server's
   * <code>DataSerializer.readArrayLength</code>.
   *
   * @param len output parameter to hold the 32-bit signed length
   *   read from stream
   */
  int32_t readArrayLength();

  /**
   * Decode a 64 bit integer as a variable length array.
   *
   * This is taken from the varint encoding in protobufs (BSD licensed).
   * See https://developers.google.com/protocol-buffers/docs/encoding
   */
  int64_t readUnsignedVL();

  /**
   * Read a float from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the float read from stream
   */
  float readFloat();

  /**
   * Read a double precision number from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the double precision number
   *   read from stream
   */
  double readDouble();

  template <class CharT = char, class... Tail>
  std::basic_string<CharT, Tail...> readUTF() {
    std::basic_string<CharT, Tail...> value;
    readJavaModifiedUtf8(value);
    return value;
  }

  template <class CharT = char, class... Tail>
  std::basic_string<CharT, Tail...> readString() {
    std::basic_string<CharT, Tail...> value;
    auto type = static_cast<internal::DSCode>(read());
    switch (type) {
      case internal::DSCode::CacheableString:
        readJavaModifiedUtf8(value);
        break;
      case internal::DSCode::CacheableStringHuge:
        readUtf16Huge(value);
        break;
      case internal::DSCode::CacheableASCIIString:
        readAscii(value);
        break;
      case internal::DSCode::CacheableASCIIStringHuge:
        readAsciiHuge(value);
        break;
      case internal::DSCode::CacheableNullString:
        // empty string
        break;
      // TODO: What's the right response here?
      default:
        break;
    }
    return value;
  }

  bool readNativeBool();

  int32_t readNativeInt32();

  std::shared_ptr<Serializable> readDirectObject(int8_t typeId = -1);

  /**
   * Read a Serializable object from the DataInput.
   *
   * @return Serializable object or <code>nullptr</code>.
   */
  std::shared_ptr<Serializable> readObject();

  /**
   * Read a <code>Serializable</code> object from the <code>DataInput</code>.
   * Null objects are handled.
   */
  void readObject(std::shared_ptr<Serializable>& ptr);

  void readObject(char16_t* value);

  void readObject(bool* value);

  void readObject(int8_t* value);

  void readObject(int16_t* value);

  void readObject(int32_t* value);

  void readObject(int64_t* value);

  void readObject(float* value);

  void readObject(double* value);

  std::vector<char16_t> readCharArray();

  std::vector<bool> readBooleanArray();

  std::vector<int8_t> readByteArray();

  std::vector<int16_t> readShortArray();

  std::vector<int32_t> readIntArray();

  std::vector<int64_t> readLongArray();

  std::vector<float> readFloatArray();

  std::vector<double> readDoubleArray();

  std::vector<std::string> readStringArray();

  void readArrayOfByteArrays(int8_t*** arrayofBytearr, int32_t& arrayLength,
                             int32_t** elementLength);

  /**
   * Get the pointer to current buffer position. This should be treated
   * as readonly and modification of contents using this internal pointer
   * has undefined behavior.
   */
  const uint8_t* currentBufferPosition() const;

  /** get the number of bytes read in the buffer */
  size_t getBytesRead() const;

  /** get the number of bytes remaining to be read in the buffer */
  size_t getBytesRemaining() const;

  /** advance the cursor by given offset */
  void advanceCursor(size_t offset);

  /** rewind the cursor by given offset */
  void rewindCursor(size_t offset);

  /** reset the cursor to the start of buffer */
  void reset();

  void setBuffer();

  void resetPdx(size_t offset);

  size_t getPdxBytes() const;

  static uint8_t* getBufferCopy(const uint8_t* from, size_t length);

  void reset(size_t offset);

  uint8_t* getBufferCopyFrom(const uint8_t* from, size_t length);

  virtual Cache* getCache() const;

  DataInput() = delete;
  virtual ~DataInput() noexcept = default;
  DataInput(const DataInput&) = delete;
  DataInput& operator=(const DataInput&) = delete;
  DataInput(DataInput&&) = default;
  DataInput& operator=(DataInput&&) = default;

 protected:
  /** constructor given a pre-allocated byte array with size */
  DataInput(const uint8_t* buffer, size_t len, const CacheImpl* cache,
            Pool* pool);

  virtual const SerializationRegistry& getSerializationRegistry() const;

 private:
  const uint8_t* m_buf;
  const uint8_t* m_bufHead;
  size_t m_bufLength;
  Pool* m_pool;
  const CacheImpl* m_cache;

  std::shared_ptr<Serializable> readObjectInternal(int8_t typeId = -1);

  template <typename mType>
  void readObject(mType** value, int32_t& length) {
    auto arrayLen = readArrayLength();
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

  template <typename T>
  std::vector<T> readArray() {
    auto arrayLen = readArrayLength();
    std::vector<T> objArray;
    if (arrayLen >= 0) {
      objArray.reserve(arrayLen);
      int i = 0;
      for (i = 0; i < arrayLen; i++) {
        T tmp = 0;
        readObject(&tmp);
        objArray.push_back(tmp);
      }
    }
    return objArray;
  }

  char readPdxChar();

  void _checkBufferSize(size_t size, int32_t line);

  int8_t readNoCheck();

  int16_t readInt16NoCheck();

  template <class CharT, class... Tail>
  void readAscii(std::basic_string<CharT, Tail...>& value, size_t length) {
    _GEODE_CHECK_BUFFER_SIZE(length);
    value.reserve(length);
    while (length-- > 0) {
      // blindly assumes ASCII so mask off 7 bits
      value += readNoCheck() & 0x7F;
    }
  }

  template <class CharT, class... Tail>
  void readAscii(std::basic_string<CharT, Tail...>& value) {
    readAscii(value, static_cast<uint16_t>(readInt16()));
  }

  template <class CharT, class... Tail>
  void readAsciiHuge(std::basic_string<CharT, Tail...>& value) {
    readAscii(value, static_cast<uint32_t>(readInt32()));
  }

  template <class _CharT, class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<_CharT, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<char16_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<char, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
      std::basic_string<char32_t, _Traits, _Allocator>& value);

  template <class _Traits, class _Allocator>
  void readJavaModifiedUtf8(
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
  void readUtf16Huge(std::basic_string<char16_t, _Traits, _Allocator>& value) {
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
  void readUtf16Huge(std::basic_string<wchar_t, _Traits, _Allocator>& value) {
    // TODO string optimize
    typedef std::conditional<
        sizeof(wchar_t) == sizeof(char16_t), char16_t,
        std::conditional<sizeof(wchar_t) == sizeof(char32_t), char32_t,
                         char>::type>::type _WcharT;

    auto tmp = std::basic_string<_WcharT>();
    readUtf16Huge(tmp);
    value.assign(reinterpret_cast<const wchar_t*>(tmp.data()), tmp.length());
  }

  Pool* getPool() const;

  friend Cache;
  friend CacheImpl;
  friend DataInputInternal;
  friend CacheableString;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATAINPUT_H_
