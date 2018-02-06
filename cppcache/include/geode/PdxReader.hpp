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

#ifndef GEODE_PDXREADER_H_
#define GEODE_PDXREADER_H_

#include "CacheableBuiltins.hpp"
#include "PdxUnreadFields.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxReader;
class CacheableObjectArray;
class CacheableDate;

/**
 * A PdxReader will be passed to PdxSerializable.fromData or
 * during deserialization of a PDX. The domain class needs to deserialize field
 * members
 * using this abstract class. This class is implemented by Native Client.
 * Each readXXX call will return the field's value. If the serialized
 * PDX does not contain the named field then a default value will
 * be returned. Standard Java defaults are used. For Objects this is
 * null and for primitives it is 0 or 0.0.
 *
 * @note Implementations of PdxReader that are internal to the Native
 *       Client library may be returned to clients via instances of
 *       PdxReader&. For those implementations, any
 *       non-<tt>nullptr</tt>, non-empty strings returned from
 *       PdxReader::readString() or PdxReader::readWideString() must
 *       be freed with DataInput::freeUTFMemory(). Arrays returned
 *       from PdxReader::readStringArray() or
 *       PdxReader::readWideStringArray() must be freed with
 *       <tt>GF_SAFE_DELETE_ARRAY</tt> once their constituent strings
 *       have been freed with DataInput::freeUTFMemory().
 * @note Custom implementations of PdxReader are not subject
 *       to this restriction.
 */
class _GEODE_EXPORT PdxReader {
 public:
  /**
   * @brief constructors
   */
  PdxReader() {}

  /**
   * @brief destructor
   */
  virtual ~PdxReader() {}

  /**
   * Read a wide char value from the <code>PdxReader</code>.
   * <p>C++ char16_t is mapped to Java char</p>
   * @param fieldName name of the field to read.
   * @return value of type wchar_t.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual char16_t readChar(const std::string& fieldName) = 0;

  /**
   * Read a bool value from the <code>PdxReader</code>.
   * <p>C++ bool is mapped to Java boolean</p>
   * @param fieldName name of the field to read
   * @return value of type bool.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual bool readBoolean(const std::string& fieldName) = 0;

  /**
   * Read a int8_t value from the <code>PdxReader</code>.
   * <p>C++ int8_t is mapped to Java byte</p>
   * @param fieldName name of the field to read
   * @return value of type int8_t.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual int8_t readByte(const std::string& fieldName) = 0;

  /**
   * Read a int16_t value from the <code>PdxReader</code>.
   * <p>C++ int16_t is mapped to Java short</p>
   * @param fieldName name of the field to read
   * @return value of type int16_t.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual int16_t readShort(const std::string& fieldName) = 0;

  /**
   * Read a int32_t value from the <code>PdxReader</code>.
   * <p>C++ int32_t is mapped to Java int</p>
   * @param fieldName name of the field to read
   * @return value of type int32_t.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual int32_t readInt(const std::string& fieldName) = 0;

  /**
   * Read a int64_t value from the <code>PdxReader</code>.
   * <p>C++ int64_t is mapped to Java long</p>
   * @param fieldName name of the field to read
   * @return value of type int64_t.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual int64_t readLong(const std::string& fieldName) = 0;

  /**
   * Read a float value from the <code>PdxReader</code>.
   * <p>C++ float is mapped to Java float</p>
   * @param fieldName name of the field to read
   * @return value of type float.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual float readFloat(const std::string& fieldName) = 0;

  /**
   * Read a double value from the <code>PdxReader</code>.
   * <p>C++ double is mapped to Java double</p>
   * @param fieldName name of the field to read
   * @return value of type double.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual double readDouble(const std::string& fieldName) = 0;

  /**
   * Read a std::string value from the <code>PdxReader</code>.
   * <p>C++ std::string is mapped to Java String</p>
   * @param fieldName name of the field to read
   * @return value of type std::string*. Refer to the class description for
   *         how to free the return value.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::string readString(const std::string& fieldName) = 0;

  /**
   * Read a std::shared_ptr<Cacheable> value from the <code>PdxReader</code>.
   * <p>C++ std::shared_ptr<Cacheable> is mapped to Java object</p>
   * @param fieldName name of the field to read
   * @return value of type std::shared_ptr<Cacheable>.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::shared_ptr<Cacheable> readObject(
      const std::string& fieldName) = 0;

  /**
   * Read a char16_t* value from the <code>PdxReader</code> and sets array
   * length. <p>C++ char16_t* is mapped to Java char[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of char16_t elements.
   * @return value of type char16_t*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<char16_t> readCharArray(const std::string& fieldName) = 0;

  /**
   * Read a bool* value from the <code>PdxReader</code> and sets array length.
   * <p>C++ bool* is mapped to Java boolean[]</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of bool elements.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<bool> readBooleanArray(const std::string& fieldName) = 0;

  /**
   * Read a int8_t* value from the <code>PdxReader</code> and sets array length.
   * <p>C++ int8_t* is mapped to Java byte[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of int8_t elements
   * @return value of type int8_t*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<int8_t> readByteArray(const std::string& fieldName) = 0;

  /**
   * Read a int16_t* value from the <code>PdxReader</code> and sets array
   * length.
   * <p>C++ int16_t* is mapped to Java short[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of int16_t elements
   * @return value of type int16_t*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<int16_t> readShortArray(const std::string& fieldName) = 0;

  /**
   * Read a int32_t* value from the <code>PdxReader</code> and sets array
   * length.
   * <p>C++ int32_t* is mapped to Java int[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of int32_t elements
   * @return value of type int32_t*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<int32_t> readIntArray(const std::string& fieldName) = 0;

  /**
   * Read a int64_t* value from the <code>PdxReader</code> and sets array
   * length.
   * <p>C++ int64_t* is mapped to Java long[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of int64_t elements
   * @return value of type int64_t*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<int64_t> readLongArray(const std::string& fieldName) = 0;

  /**
   * Read a float* value from the <code>PdxReader</code> and sets array length.
   * <p>C++ float* is mapped to Java float[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of float elements
   * @return value of type float*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<float> readFloatArray(const std::string& fieldName) = 0;

  /**
   * Read a double* value from the <code>PdxReader</code> and sets array length.
   * <p>C++ double* is mapped to Java double[].</p>
   * @param fieldName name of the field to read
   * @param length length is set with number of double elements
   * @return value of type double*.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<double> readDoubleArray(const std::string& fieldName) = 0;

  /**
   * Read a array of strings from the <code>PdxReader</code>.
   * <p>C++ std::vector<std::string> is mapped to Java String[].</p>
   * @param fieldName name of the field to read
   * @return value of type std::vector<std::string>. Refer to the class
   * description for how to free the return value.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::vector<std::string> readStringArray(
      const std::string& fieldName) = 0;

  /**
   * Read a std::shared_ptr<CacheableObjectArray> value from the
   * <code>PdxReader</code>. C++ std::shared_ptr<CacheableObjectArray> is mapped
   * to Java Object[].
   * @param fieldName name of the field to read
   * @return value of type std::shared_ptr<CacheableObjectArray>.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::shared_ptr<CacheableObjectArray> readObjectArray(
      const std::string& fieldName) = 0;

  /**
   * Read a int8_t** value from the <code>PdxReader</code> and sets
   * ArrayOfByteArray's length and individual ByteArray's length.
   * <p>C++ int8_t** is mapped to Java byte[][].</p>
   * @param fieldName name of the field to read
   * @param arrayLength length is set with number of int8_t* elements
   * @param elementLength elementLength is set with the length value of
   * individual byte arrays.
   * @return value of type int8_t**.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual int8_t** readArrayOfByteArrays(const std::string& fieldName,
                                         int32_t& arrayLength,
                                         int32_t** elementLength) = 0;

  /**
   * Read a std::shared_ptr<CacheableDate> value from the
   * <code>PdxReader</code>. <p>C++ std::shared_ptr<CacheableDate> is mapped to
   * Java Date</p>
   * @param fieldName name of the field to read
   * @return value of type std::shared_ptr<CacheableDate>.
   * @throws IllegalStateException if PdxReader doesn't has the named field.
   *
   * @see PdxReader#hasField
   */
  virtual std::shared_ptr<CacheableDate> readDate(
      const std::string& fieldName) = 0;

  /**
   * Checks if the named field exists and returns the result.
   * This can be useful when writing code that handles more than one version of
   * a PDX class.
   * @param fieldname the name of the field to check
   * @return <code>true</code> if the named field exists; otherwise
   * <code>false</code>
   */
  virtual bool hasField(const std::string& fieldName) = 0;

  /**
   * Checks if the named field was {@link PdxWriter#markIdentityField}marked as
   * an identity field.
   * Note that if no fields have been marked then all the fields are used as
   * identity fields even though
   * this method will return <code>false</code> since none of them have been
   * <em>marked</em>.
   * @param fieldname the name of the field to check
   * @return <code>true</code> if the named field exists and was marked as an
   * identify field; otherwise <code>false</code>
   */
  virtual bool isIdentityField(const std::string& fieldName) = 0;

  /**
   * This method returns an object that represents all the unread fields which
   * must be
   * passed to {@link PdxWriter#writeUnreadFields} in the toData code.
   * <P>Note that if {@link CacheFactory#setPdxIgnoreUnreadFields}
   * is set to <code>true</code> then this method will always return an object
   * that has no unread fields.
   *
   * @return an object that represents the unread fields.
   */
  virtual std::shared_ptr<PdxUnreadFields> readUnreadFields() = 0;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREADER_H_
