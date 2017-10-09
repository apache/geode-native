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

#include "geode_globals.hpp"
#include "ExceptionTypes.hpp"
#include <cstring>
#include <string>
#include "geode_types.hpp"
#include "Serializable.hpp"
#include "CacheableString.hpp"

/**
 * @file
 */

#if GF_DEBUG_ASSERTS == 1
#define DINP_THROWONERROR_DEFAULT true
#else
#define DINP_THROWONERROR_DEFAULT false
#endif

#define checkBufferSize(x) _checkBufferSize(x, __LINE__)

namespace apache {
namespace geode {
namespace client {

extern int gf_sprintf(char* buffer, const char* fmt, ...);

class SerializationRegistry;
class DataInputInternal;

/**
 * Provide operations for reading primitive data values, byte arrays,
 * strings, <code>Serializable</code> objects from a byte stream.
 * This class is intentionally not thread safe.
 * @remarks None of the output parameters in the methods below can be nullptr
 *   unless otherwise noted.
 */
class CPPCACHE_EXPORT DataInput {
 public:

  /**
   * Read a signed byte from the <code>DataInput</code>.
   *
   * @@return signed byte read from stream
   */
  inline int8_t read() {
    checkBufferSize(1);
    return *(m_buf++);
  }

  /**
   * Read a boolean value from the <code>DataInput</code>.
   *
   * @param value output parameter to hold the boolean read from stream
   */
  inline bool readBoolean() {
    checkBufferSize(1);
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
  inline void readBytesOnly(uint8_t* buffer, uint32_t len) {
    if (len > 0) {
      checkBufferSize(len);
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
  inline void readBytesOnly(int8_t* buffer, uint32_t len) {
    if (len > 0) {
      checkBufferSize(len);
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
      checkBufferSize(length);
      GF_NEW(buffer, uint8_t[length]);
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
      checkBufferSize(length);
      GF_NEW(buffer, int8_t[length]);
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
  inline int16_t  readInt16() {
    checkBufferSize(2);
    int16_t tmp = *(m_buf++);
    tmp = static_cast<int16_t>((tmp << 8) | *(m_buf++));
    return tmp;
  }

  /**
   * Read a 32-bit signed integer from the <code>DataInput</code>.g
   *
   * @param value output parameter to hold the 32-bit signed integer
   *   read from stream
   */
  inline int32_t readInt32() {
    checkBufferSize(4);
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
    checkBufferSize(8);
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
          uint16_t val =  readInt16();
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
    checkBufferSize(4);
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
    checkBufferSize(8);
    union double_uint64_t {
      double d;
      uint64_t ll;
    } v;
    v.ll = readInt64();
     return v.d;
  }

  /**
   * free the C string allocated by <code>readASCII</code>,
   * <code>readASCIIHuge</code>, <code>readUTF</code>,
   * <code>readUTFHuge</code> methods
   */
  static inline void freeUTFMemory(char* value) { delete[] value; }

  /**
   * free the wide-characted string allocated by <code>readASCII</code>,
   * <code>readASCIIHuge</code>, <code>readUTF</code>,
   * <code>readUTFHuge</code> methods
   */
  static inline void freeUTFMemory(wchar_t* value) { delete[] value; }

  /**
   * Allocates a c string buffer, and reads an ASCII string
   * having maximum length of 64K from <code>DataInput</code> into it.
   * @remarks Sets integer at length to hold the strlen of the string. Value
   *   is modified to point to the new allocation. The chars are allocated as
   *   an array, so the caller must use <code>freeUTFMemory</code> when done.
   *   Like <code>DataOutput::writeASCII</code> the maximum length supported by
   *   this method is 64K; use <code>readASCIIHuge</code> or
   *   <code>readBytes</code> to read strings of length larger than this.
   *
   * @param value output C string to hold the read characters; it is allocated
   *   by this method
   * @param len output parameter to hold the number of characters read from
   *   stream; not set if nullptr
   */
  inline void readASCII(char** value, uint16_t* len = nullptr) {
    uint16_t length = readInt16();
    checkBufferSize(length);
    if (len != nullptr) {
      *len = length;
    }
    char* str;
    GF_NEW(str, char[length + 1]);
    *value = str;
    readBytesOnly(reinterpret_cast<int8_t*>(str), length);
    str[length] = '\0';
  }

  /**
   * Allocates a c string buffer, and reads an ASCII string
   * from <code>DataInput</code> into it.
   * @remarks Sets integer at length to hold the strlen of the string. Value
   *   is modified to point to the new allocation. The chars are allocated as
   *   an array, so the caller must use <code>freeUTFMemory</code> when done.
   *   Use this instead of <code>readUTF</code> when reading a string of length
   *   greater than 64K.
   *
   * @param value output C string to hold the read characters; it is allocated
   *   by this method
   * @param len output parameter to hold the number of characters read from
   *   stream; not set if nullptr
   */
  inline void readASCIIHuge(char** value, uint32_t* len = nullptr) {
    uint32_t length = readInt32();
    if (len != nullptr) {
      *len = length;
    }
    char* str;
    GF_NEW(str, char[length + 1]);
    *value = str;
    readBytesOnly(reinterpret_cast<int8_t*>(str), length);
    str[length] = '\0';
  }

  /**
   * Allocates a c string buffer, and reads a java modified UTF-8
   * encoded string having maximum encoded length of 64K from
   * <code>DataInput</code> into it.
   * @remarks Sets integer at length to hold the strlen of the string. Value
   *   is modified to point to the new allocation. The chars are allocated as
   *   an array, so the caller must use <code>freeUTFMemory</code> when done.
   *   Like <code>DataOutput::writeUTF</code> the maximum length supported by
   *   this method is 64K; use <code>readAUTFHuge</code> to read strings of
   *   length larger than this.
   *
   * @param value output C string to hold the read characters; it is allocated
   *   by this method
   * @param len output parameter to hold the number of characters read from
   *   stream; not set if nullptr
   */
  inline void readUTF(char** value, uint16_t* len = nullptr) {
    uint16_t length = readInt16();
    checkBufferSize(length);
    uint16_t decodedLen =
        static_cast<uint16_t>(getDecodedLength(m_buf, length));
    if (len != nullptr) {
      *len = decodedLen;
    }
    char* str;
    GF_NEW(str, char[decodedLen + 1]);
    *value = str;
    for (uint16_t i = 0; i < decodedLen; i++) {
      decodeChar(str++);
    }
    *str = '\0';  // null terminate for c-string.
  }

  /**
   * Reads a java modified UTF-8 encoded string having maximum encoded length
   * of 64K without reading the length which must be passed as a parameter.
   * Allocates a c string buffer, and deserializes into it. Sets integer at
   * length to hold the length of the string. Value is modified to point to the
   * new allocation. The chars are allocated as an array, so the caller must
   * use freeUTFMemory when done.
   * If len == nullptr, then the decoded string length is not set.
   */
  inline void readUTFNoLen(wchar_t** value, uint16_t decodedLen) {
    wchar_t* str;
    GF_NEW(str, wchar_t[decodedLen + 1]);
    *value = str;
    for (uint16_t i = 0; i < decodedLen; i++) {
      decodeChar(str++);
    }
    *str = L'\0';  // null terminate for c-string.
  }

  /**
   * Allocates a c string buffer, and reads a java modified UTF-8
   * encoded string from <code>DataInput</code> into it.
   * @remarks Sets integer at length to hold the strlen of the string. Value
   *   is modified to point to the new allocation. The chars are allocated as
   *   an array, so the caller must use <code>freeUTFMemory</code> when done.
   *   Use this instead of <code>readUTF</code> when reading a string of length
   *   greater than 64K.
   *
   * @param value output C string to hold the read characters; it is allocated
   *   by this method
   * @param len output parameter to hold the number of characters read from
   *   stream; not set if nullptr
   */
  inline void readUTFHuge(char** value, uint32_t* len = nullptr) {
    uint32_t length = readInt32();
    if (len != nullptr) {
      *len = length;
    }
    char* str;
    GF_NEW(str, char[length + 1]);
    *value = str;
    for (uint32_t i = 0; i < length; i++) {
      read();  // ignore this - should be higher order zero byte
      *str = read();
      str++;
    }
    *str = '\0';  // null terminate for c-string.
  }

  /**
   * Allocates a wide-character string buffer, and reads a java
   * modified UTF-8 encoded string having maximum encoded length of 64K from
   * <code>DataInput</code> into it.
   * @remarks Sets integer at length to hold the strlen of the string. Value
   *   is modified to point to the new allocation. The chars are allocated as
   *   an array, so the caller must use <code>freeUTFMemory</code> when done.
   *   Like <code>DataOutput::writeUTF</code> the maximum length supported by
   *   this method is 64K; use <code>readAUTFHuge</code> to read strings of
   *   length larger than this.
   *
   * @param value output wide-character string to hold the read characters;
   *   it is allocated by this method
   * @param len output parameter to hold the number of characters read from
   *   stream; not set if nullptr
   */
  inline void readUTF(wchar_t** value, uint16_t* len = nullptr) {
    uint16_t length = readInt16();
    checkBufferSize(length);
    uint16_t decodedLen =
        static_cast<uint16_t>(getDecodedLength(m_buf, length));
    if (len != nullptr) {
      *len = decodedLen;
    }
    wchar_t* str;
    GF_NEW(str, wchar_t[decodedLen + 1]);
    *value = str;
    for (uint16_t i = 0; i < decodedLen; i++) {
      decodeChar(str++);
    }
    *str = L'\0';  // null terminate for c-string.
  }

  /**
   * Allocates a wide-character string buffer, and reads a java
   * modified UTF-8 encoded string from <code>DataInput</code> into it.
   * @remarks Sets integer at length to hold the strlen of the string. Value
   *   is modified to point to the new allocation. The chars are allocated as
   *   an array, so the caller must use <code>freeUTFMemory</code> when done.
   *   Use this instead of <code>readUTF</code> when reading a string of length
   *   greater than 64K.
   *
   * @param value output wide-character string to hold the read characters;
   *   it is allocated by this method
   * @param len output parameter to hold the number of characters read from
   *   stream; not set if nullptr
   */
  inline void readUTFHuge(wchar_t** value, uint32_t* len = nullptr) {
    uint32_t length = readInt32();
    if (len != nullptr) {
      *len = length;
    }
    wchar_t* str;
    GF_NEW(str, wchar_t[length + 1]);
    *value = str;
    for (uint32_t i = 0; i < length; i++) {
      const auto hibyte = read();
      const auto lobyte = read();
      *str = ((static_cast<uint16_t>(hibyte)) << 8) |
             static_cast<uint16_t>(lobyte);
      str++;
    }
    *str = L'\0';  // null terminate for c-string.
  }

  /**
   * Read a <code>Serializable</code> object from the <code>DataInput</code>.
   * Null objects are handled.
   * This accepts an argument <code>throwOnError</code> that
   * specifies whether to check the type dynamically and throw a
   * <code>ClassCastException</code> when the cast fails.
   *
   * @param ptr The object to be read which is output by reference.
   *            The type of this must match the type of object that
   *            the application expects.
   * @param throwOnError Throw a <code>ClassCastException</code> when
   *                     the type of object does not match <code>ptr</code>.
   *                     Default is true when <code>GF_DEBUG_ASSERTS</code>
   *                     macro is set and false in normal case.
   * @throws ClassCastException When <code>dynCast</code> fails
   *                            for the given <code>ptr</code>.
   * @see dynCast
   * @see staticCast
   */
  template <class PTR>
  inline std::shared_ptr<PTR> readObject(
                         bool throwOnError = DINP_THROWONERROR_DEFAULT) {
    SerializablePtr sPtr = readObjectInternal();
    if (throwOnError) {
      return std::dynamic_pointer_cast<PTR>(sPtr);
    } else {
      return std::static_pointer_cast<PTR>(sPtr);
    }
  }

  inline bool readNativeBool() {
    read(); // ignore type id

    return readBoolean();
  }

  inline int32_t readNativeInt32() {
    read(); // ignore type id
    return readInt32();
  }

  inline CacheableStringPtr readNativeString() {
    CacheableStringPtr csPtr;
    const int64_t compId = read();
    if (compId == GeodeTypeIds::NullObj) {
      csPtr = nullptr;
    } else if (compId == GeodeTypeIds::CacheableNullString) {
      csPtr = CacheableStringPtr(dynamic_cast<CacheableString*>(
          CacheableString::createDeserializable()));
    } else if (compId ==
               apache::geode::client::GeodeTypeIds::CacheableASCIIString) {
      csPtr = CacheableStringPtr(dynamic_cast<CacheableString*>(
          CacheableString::createDeserializable()));
      csPtr->fromData(*this);
    } else if (compId ==
               apache::geode::client::GeodeTypeIds::CacheableASCIIStringHuge) {
      csPtr = CacheableStringPtr(dynamic_cast<CacheableString*>(
          CacheableString::createDeserializableHuge()));
      csPtr->fromData(*this);
    } else if (compId == apache::geode::client::GeodeTypeIds::CacheableString) {
      csPtr = CacheableStringPtr(dynamic_cast<CacheableString*>(
          CacheableString::createUTFDeserializable()));
      csPtr->fromData(*this);
    } else if (compId ==
               apache::geode::client::GeodeTypeIds::CacheableStringHuge) {
      csPtr = CacheableStringPtr(dynamic_cast<CacheableString*>(
          CacheableString::createUTFDeserializableHuge()));
      csPtr->fromData(*this);
    } else {
      LOGDEBUG("In readNativeString something is wrong while expecting string");
      rewindCursor(1);
      csPtr = nullptr;
    }
    return csPtr;
  }

  inline SerializablePtr readDirectObject(int8_t typeId = -1) {
    return readObjectInternal(typeId);
  }

  /**
   * Read a <code>Serializable</code> object from the <code>DataInput</code>.
   * Null objects are handled.
   */
  inline void readObject(SerializablePtr& ptr) { ptr = readObjectInternal(); }

  inline void readObject(wchar_t* value) {
    uint16_t temp = readInt16();
    *value = static_cast<wchar_t>(temp);
  }

  inline void readObject(bool* value) { *value = readBoolean(); }

  inline void readObject(int8_t* value) { *value = read(); }

  inline void readObject(int16_t* value) { *value = readInt16(); }

  inline void readObject(int32_t* value) { *value = readInt32(); }

  inline void readObject(int64_t* value) { *value = readInt64(); }

  inline void readObject(float* value) { *value = readFloat(); }

  inline void readObject(double* value) { *value = readDouble(); }

  inline void readCharArray(char** value, int32_t& length) {
    int arrayLen = readArrayLen();
    length = arrayLen;
    char* objArray = nullptr;
    if (arrayLen > 0) {
      objArray = new char[arrayLen];
      int i = 0;
      for (i = 0; i < arrayLen; i++) {
        char tmp = readPdxChar();
        objArray[i] = tmp;
      }
      *value = objArray;
    }
  }

  inline void readWideCharArray(wchar_t** value, int32_t& length) {
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

  inline void readString(char** value) {
    const auto typeId = read();

    // Check for nullptr String
    if (typeId == GeodeTypeIds::CacheableNullString) {
      *value = nullptr;
      return;
    }
    /*
    if (typeId == GeodeTypeIds::CacheableString) {
      readUTF(value);
    } else {
      readUTFHuge(value);
    }
    */
    if (typeId == static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString) ||
        typeId == static_cast<int8_t>(GeodeTypeIds::CacheableString)) {
      // readUTF( value);
      readASCII(value);
      // m_len = shortLen;
    } else if (typeId == static_cast<int8_t>(
                             GeodeTypeIds::CacheableASCIIStringHuge) ||
               typeId ==
                   static_cast<int8_t>(GeodeTypeIds::CacheableStringHuge)) {
      // readUTFHuge( value);
      readASCIIHuge(value);
    } else {
      throw IllegalArgumentException(
          "DI readString error:: String type not supported ");
    }
  }

  inline void readWideString(wchar_t** value) {
    const auto typeId = read();

    // Check for nullptr String
    if (typeId == GeodeTypeIds::CacheableNullString) {
      *value = nullptr;
      return;
    }

    if (typeId == static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString) ||
        typeId == static_cast<int8_t>(GeodeTypeIds::CacheableString)) {
      readUTF(value);
    } else if (typeId == static_cast<int8_t>(
                             GeodeTypeIds::CacheableASCIIStringHuge) ||
               typeId ==
                   static_cast<int8_t>(GeodeTypeIds::CacheableStringHuge)) {
      readUTFHuge(value);
    } else {
      throw IllegalArgumentException(
          "DI readWideString error:: WideString type provided is not "
          "supported ");
    }
  }

  inline void readStringArray(char*** strArray, int32_t& length) {
    int32_t arrLen = readArrayLen();
    length = arrLen;
    if (arrLen == -1) {
      *strArray = nullptr;
      return;
    } else {
      char** tmpArray;
      GF_NEW(tmpArray, char * [arrLen]);
      for (int i = 0; i < arrLen; i++) {
        readString(&tmpArray[i]);
      }
      *strArray = tmpArray;
    }
  }

  inline void readWideStringArray(wchar_t*** strArray, int32_t& length) {
    int32_t arrLen = readArrayLen();
    length = arrLen;
    if (arrLen == -1) {
      *strArray = nullptr;
      return;
    } else {
      wchar_t** tmpArray;
      GF_NEW(tmpArray, wchar_t * [arrLen]);
      for (int i = 0; i < arrLen; i++) {
        readWideString(&tmpArray[i]);
      }
      *strArray = tmpArray;
    }
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
      GF_NEW(tmpArray, int8_t * [arrLen]);
      GF_NEW(tmpLengtharr, int32_t[arrLen]);
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
  inline int32_t getBytesRead() const {
    return static_cast<int32_t>(m_buf - m_bufHead);
  }

  /** get the number of bytes remaining to be read in the buffer */
  inline int32_t getBytesRemaining() const {
    return (m_bufLength - getBytesRead());
  }

  /** advance the cursor by given offset */
  inline void advanceCursor(int32_t offset) { m_buf += offset; }

  /** rewind the cursor by given offset */
  inline void rewindCursor(int32_t offset) { m_buf -= offset; }

  /** reset the cursor to the start of buffer */
  inline void reset() { m_buf = m_bufHead; }

  inline void setBuffer() {
    m_buf = currentBufferPosition();
    m_bufLength = getBytesRemaining();
  }

  inline void resetPdx(int32_t offset) { m_buf = m_bufHead + offset; }

  inline int32_t getPdxBytes() const { return m_bufLength; }

  static uint8_t* getBufferCopy(const uint8_t* from, uint32_t length) {
    uint8_t* result;
    GF_NEW(result, uint8_t[length]);
    std::memcpy(result, from, length);

    return result;
  }

  inline void reset(int32_t offset) { m_buf = m_bufHead + offset; }

  uint8_t* getBufferCopyFrom(const uint8_t* from, uint32_t length) {
    uint8_t* result;
    GF_NEW(result, uint8_t[length]);
    std::memcpy(result, from, length);

    return result;
  }

  /*
   * This is for internal use
   */
  const char* getPoolName() { return m_poolName; }

  /*
   * This is for internal use
   */
  void setPoolName(const char* poolName) { m_poolName = poolName; }

  virtual const Cache* getCache();

 protected:
  /** constructor given a pre-allocated byte array with size */
  DataInput(const uint8_t* m_buffer, int32_t len, const Cache* cache)
      : m_buf(m_buffer),
        m_bufHead(m_buffer),
        m_bufLength(len),
        m_poolName(nullptr),
        m_cache(cache) {}

  virtual const SerializationRegistry& getSerializationRegistry() const;

 private:
  const uint8_t* m_buf;
  const uint8_t* m_bufHead;
  int32_t m_bufLength;
  const char* m_poolName;
  const Cache* m_cache;

  SerializablePtr readObjectInternal(int8_t typeId = -1);

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

  inline char readPdxChar() {
    return static_cast<char>(readInt16());
  }

  inline void _checkBufferSize(int32_t size, int32_t line) {
    if ((m_bufLength - (m_buf - m_bufHead)) < size) {
      char exMsg[128];
      gf_sprintf(exMsg,
                 "DataInput: attempt to read beyond buffer at line %d: "
                 "available buffer size %d, attempted read of size %d ",
                 line, m_bufLength - (m_buf - m_bufHead), size);
      throw OutOfRangeException(exMsg);
    }
  }

  inline void decodeChar(char* str) {
    uint8_t bt = *(m_buf++);
    if (bt & 0x80) {
      if (bt & 0x20) {
        // three bytes.
        *str =
            static_cast<char>(((bt & 0x0f) << 12) | (((*m_buf++) & 0x3f) << 6));
        *str |= static_cast<char>((*m_buf++) & 0x3f);
      } else {
        // two bytes.
        *str = static_cast<char>(((bt & 0x1f) << 6) | ((*m_buf++) & 0x3f));
      }
    } else {
      // single byte...
      *str = bt;
    }
  }

  inline void decodeChar(wchar_t* str) {
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
        *str = (y << 6 | x);
        break;
      }
      case 7: {
        // three byte encoding
        // 1110zzzz 10yyyyyy 10xxxxxx
        // assert ( b & 0x10 )
        //     == 0 : "UTF8Decoder does not handle 32-bit characters";
        // use low order 4 bits
        int32_t z = b & 0x0f;
        // use low order 6 bits of the next byte
        // It should have high order bits 10, which we don't check.
        int32_t y = *m_buf++ & 0x3f;
        // use low order 6 bits of the next byte
        // It should have high order bits 10, which we don't check.
        int32_t x = *m_buf++ & 0x3f;
        // zzzzyyyy yyxxxxxx
        int32_t asint = (z << 12 | y << 6 | x);
        *str = asint;
        break;
      }
      default:
        // one byte encoding
        // 0xxxxxxx
        // use just low order 7 bits
        // 00000000 0xxxxxxx
        *str = (b & 0x7f);
        break;
    }
  }

  // disable other constructors and assignment
  DataInput() = delete;
  DataInput(const DataInput&) = delete;
  DataInput& operator=(const DataInput&) = delete;

  friend Cache;
  friend DataInputInternal;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATAINPUT_H_
