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
  void write(uint8_t value);

  /**
   * Write a signed byte to the <code>DataOutput</code>.
   *
   * @param value the signed byte to be written
   */
  void write(int8_t value);

  /**
   * Write a boolean value to the <code>DataOutput</code>.
   *
   * @param value the boolean value to be written
   */
  void writeBoolean(bool value);

  /**
   * Write an array of unsigned bytes to the <code>DataOutput</code>.
   *
   * @param value the array of unsigned bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  void writeBytes(const uint8_t* bytes, int32_t len);

  /**
   * Write an array of signed bytes to the <code>DataOutput</code>.
   *
   * @param value the array of signed bytes to be written
   * @param len the number of bytes from the start of array to be written
   */
  void writeBytes(const int8_t* bytes, int32_t len);

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
  void writeBytesOnly(const uint8_t* bytes, size_t len);

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
  void writeBytesOnly(const int8_t* bytes, size_t len);

  /**
   * Write a 16-bit unsigned integer value to the <code>DataOutput</code>.
   *
   * @param value the 16-bit unsigned integer value to be written
   */
  void writeInt(uint16_t value);

  /**
   * Write a 16-bit Char (wchar_t) value to the <code>DataOutput</code>.
   *
   * @param value the 16-bit wchar_t value to be written
   */
  void writeChar(uint16_t value);

  /**
   * Write a 32-bit unsigned integer value to the <code>DataOutput</code>.
   *
   * @param value the 32-bit unsigned integer value to be written
   */
  void writeInt(uint32_t value);

  /**
   * Write a 64-bit unsigned integer value to the <code>DataOutput</code>.
   *
   * @param value the 64-bit unsigned integer value to be written
   */
  void writeInt(uint64_t value);

  /**
   * Write a 16-bit signed integer value to the <code>DataOutput</code>.
   *
   * @param value the 16-bit signed integer value to be written
   */
  void writeInt(int16_t value);

  /**
   * Write a 32-bit signed integer value to the <code>DataOutput</code>.
   *
   * @param value the 32-bit signed integer value to be written
   */
  void writeInt(int32_t value);

  /**
   * Write a 64-bit signed integer value to the <code>DataOutput</code>.
   *
   * @param value the 64-bit signed integer value to be written
   */
  void writeInt(int64_t value);

  /**
   * Write a 32-bit signed integer array length value to the
   * <code>DataOutput</code> in a manner compatible with java server's
   * <code>DataSerializer.writeArrayLength</code>.
   *
   * @param value the 32-bit signed integer array length to be written
   */
  void writeArrayLen(int32_t len);

  /**
   * Write a float value to the <code>DataOutput</code>.
   *
   * @param value the float value to be written
   */
  void writeFloat(float value);

  /**
   * Write a double precision real number to the <code>DataOutput</code>.
   *
   * @param value the double precision real number to be written
   */
  void writeDouble(double value);
  void writeString(const char* value);
  void writeString(const wchar_t* value);
  void writeString(const char16_t* value);
  void writeString(const char32_t* value);

  void writeString(const std::string& value);
  void writeString(const std::wstring& value);
  void writeString(const std::u16string& value);
  void writeString(const std::u32string& value);

  void writeUTF(const char* value);
  void writeUTF(const wchar_t* value);
  void writeUTF(const char16_t* value);
  void writeUTF(const char32_t* value);

  void writeUTF(const std::string& value);
  void writeUTF(const std::wstring& value);
  void writeUTF(const std::u16string& value);
  void writeUTF(const std::u32string& value);

  /**
   * Writes a sequence of UTF-16 code units representing the given string value.
   * The output does not contain any length of termination charactes.
   *
   * @tparam _CharT matches character type of std::basic_string.
   * @tparam _Tail matches all remaining template parameters for
   * std::basic_string.
   * @param value string to write as UTF-16 units
   */
  void writeChars(const std::string& value);
  void writeChars(const std::wstring& value);
  void writeChars(const std::u16string& value);
  void writeChars(const std::u32string& value);

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
  void writeChars(const char* value);
  void writeChars(const wchar_t* value);
  void writeChars(const char16_t* value);
  void writeChars(const char32_t* value);

  /**
   * Write a <code>Serializable</code> object to the <code>DataOutput</code>.
   *
   * @param objptr smart pointer to the <code>Serializable</code> object
   *   to be written
   */
  void writeObject(const std::shared_ptr<Serializable>& objptr,
                   bool isDelta = false);

  /**
   * Get an internal pointer to the current location in the
   * <code>DataOutput</code> byte array.
   */
  const uint8_t* getCursor();

  /**
   * Advance the buffer cursor by the given offset.
   *
   * @param offset the offset by which to advance the cursor
   */
  void advanceCursor(size_t offset);

  /**
   * Rewind the buffer cursor by the given offset.
   *
   * @param offset the offset by which to rewind the cursor
   */
  void rewindCursor(size_t offset);

  void updateValueAtPos(size_t offset, uint8_t value);

  uint8_t getValueAtPos(size_t offset);

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   */
  const uint8_t* getBuffer() const;

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   */
  size_t getRemainingBufferLength() const;

  /**
   * Get a pointer to the internal buffer of <code>DataOutput</code>.
   *
   * @param rsize the size of buffer is filled in this output parameter;
   *   should not be nullptr
   */
  const uint8_t* getBuffer(size_t* rsize) const;

  uint8_t* getBufferCopy();

  /**
   * Get the length of current data in the internal buffer of
   * <code>DataOutput</code>.
   */
  size_t getBufferLength() const;

  /**
   * Reset the internal cursor to the start of the buffer.
   */
  void reset();

  // make sure there is room left for the requested size item.
  void ensureCapacity(size_t size);

  uint8_t* getBufferCopyFrom(const uint8_t* from, size_t length);

  static void safeDelete(uint8_t* src);

  virtual Cache* getCache() const;

  DataOutput() = delete;

  /** Destruct a DataOutput, including releasing the created buffer. */
  virtual ~DataOutput();

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

  // memory m_buffer to encode to.
  std::unique_ptr<uint8_t[]> m_bytes;
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

  void writeAscii(const std::string& value);

  void writeAsciiHuge(const std::string& value);

  void writeJavaModifiedUtf8(const std::u16string& value);

  void writeJavaModifiedUtf8(const std::string& value);

  void writeJavaModifiedUtf8(const std::u32string& value);

  void writeJavaModifiedUtf8(const std::wstring& value);

  void writeJavaModifiedUtf8(const char16_t* data, size_t len);

  void writeJavaModifiedUtf8(const char32_t* data, size_t len);

  void writeUtf16Huge(const std::u16string& value);

  void writeUtf16Huge(const std::string& value);

  void writeUtf16Huge(const std::u32string& value);

  void writeUtf16Huge(const std::wstring& value);

  void writeUtf16Huge(const char16_t* data, size_t length);

  void writeUtf16Huge(const char32_t* data, size_t len);

  void writeUtf16(const std::u16string& value);

  void writeUtf16(const std::string& value);

  void writeUtf16(const std::u32string& value);

  void writeUtf16(const std::wstring& value);

  void writeUtf16(const char16_t* data, size_t length);

  void writeUtf16(const char32_t* data, size_t len);

  static size_t getJavaModifiedUtf8EncodedLength(const char16_t* data,
                                                 size_t length);

  static void getEncodedLength(const char val, int32_t& encodedLen);

  static void getEncodedLength(const wchar_t val, int32_t& encodedLen);

  void encodeChar(const char value);

  // this will lose the character set encoding.
  void encodeChar(const wchar_t value);

  void encodeJavaModifiedUtf8(const char16_t c);

  void writeNoCheck(uint8_t value);

  void writeNoCheck(int8_t value);

  Pool* getPool() const;

  static uint8_t* checkoutBuffer(size_t* size);
  static void checkinBuffer(uint8_t* buffer, size_t size);

  friend Cache;
  friend CacheImpl;
  friend DataOutputInternal;
  friend CacheableString;
  friend TcrMessage;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATAOUTPUT_H_
