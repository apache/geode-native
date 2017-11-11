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

#ifndef GEODE_PDXINSTANCEIMPL_H_
#define GEODE_PDXINSTANCEIMPL_H_

#include <vector>
#include <map>

#include <geode/PdxInstance.hpp>
#include <geode/WritablePdxInstance.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxFieldTypes.hpp>

#include "PdxType.hpp"
#include "PdxLocalWriter.hpp"
#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<std::string, std::shared_ptr<Cacheable>> FieldVsValues;

class CPPCACHE_EXPORT PdxInstanceImpl : public WritablePdxInstance {
 public:
  /**
   * @brief destructor
   */
  virtual ~PdxInstanceImpl();

  /**
   * Deserializes and returns the domain object that this instance represents.
   * For deserialization C++ Native Client requires the domain class to be
   * registered.
   * @return the deserialized domain object.
   *
   * @see serializationRegistry->addPdxType
   */
  virtual std::shared_ptr<PdxSerializable> getObject();

  /**
   * Checks if the named field exists and returns the result.
   * This can be useful when writing code that handles more than one version of
   * a PDX class.
   * @param fieldname the name of the field to check
   * @return <code>true</code> if the named field exists; otherwise
   * <code>false</code>
   */
  virtual bool hasField(const char* fieldname);

  /**
   * Reads the named field and set its value in bool type out param.
   * bool type is corresponding to java boolean type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with bool type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual bool getBooleanField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in signed char type out param.
   * signed char type is corresponding to java byte type.
   * For C++ on Windows and Linux, signed char type is corresponding to int8_t
   * type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t to signed char.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with signed char type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual int8_t getByteField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in int16_t type out param.
   * int16_t type is corresponding to java short type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int16_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual int16_t getShortField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in int32_t type out param.
   * int32_t type is corresponding to java int type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int32_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   */
  virtual int32_t getIntField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in int64_t type out param.
   * int64_t type is corresponding to java long type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int64_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual int64_t getLongField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in float type out param.
   * float type is corresponding to java float type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with float type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual float getFloatField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in double type out param.
   * double type is corresponding to java double type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with double type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual double getDoubleField(const char* fieldname) const;

  /**
   * Reads the named field and set its value in char type out param.
   * char type is corresponding to java char type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with char type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual char16_t getCharField(const char* fieldName) const;

  /**
   * Reads the named field and set its value in bool array type out param.
   * bool* type is corresponding to java boolean[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with bool array type.
   * @param length length is set with number of bool elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, bool** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in signed char array type out
   * param. signed char* type is corresponding to java byte[] type. For C++ on
   * Windows and Linux, signed char* type is corresponding to int8_t* type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t* to signed char*.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with signed char array type.
   * @param length length is set with number of signed char elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, signed char** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in unsigned char array type out
   * param.
   * unsigned char* type is corresponding to java byte[] type.
   * For C++ on Windows and Linux, unsigned char* type is corresponding to
   * int8_t* type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t* to unsigned char*.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with unsigned char array type.
   * @param length length is set with number of unsigned char elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, unsigned char** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in int16_t array type out param.
   * int16_t* type is corresponding to java short[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int16_t array type.
   * @param length length is set with number of int16_t elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, int16_t** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in int32_t array type out param.
   * int32_t* type is corresponding to java int[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int32_t array type.
   * @param length length is set with number of int32_t elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, int32_t** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in int64_t array type out param.
   * int64_t* type is corresponding to java long[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int64_t array type.
   * @param length length is set with number of int64_t elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, int64_t** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in float array type out param.
   * float* type is corresponding to java float[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with float array type.
   * @param length length is set with number of float elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, float** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in double array type out param.
   * double* type is corresponding to java double[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with double array type.
   * @param length length is set with number of double elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, double** value,
                        int32_t& length) const;

  // charArray
  /**
   * Reads the named field and set its value in wchar_t array type out param.
   * wchar_t* type is corresponding to java String type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with wchar_t array type.
   * @param length length is set with number of wchar_t* elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldName, wchar_t** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in char array type out param.
   * char* type is corresponding to java String type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with char array type.
   * @param length length is set with number of char* elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldName, char** value,
                        int32_t& length) const;

  // String
  /**
   * Reads the named field and set its value in wchar_t* type out param.
   * wchar_t* type is corresponding to java String type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with wchar_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, wchar_t** value) const;

  /**
   * Reads the named field and set its value in char* type out param.
   * char* type is corresponding to java String type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with char* type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, char** value) const;

  // StringArray
  /**
   * Reads the named field and set its value in wchar_t* array type out param.
   * wchar_t** type is corresponding to java String[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with wchar_t* array type.
   * @param length length is set with number of wchar_t** elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, wchar_t*** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in char* array type out param.
   * char** type is corresponding to java String[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with char* array type.
   * @param length length is set with number of char** elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldname, char*** value,
                        int32_t& length) const;

  /**
   * Reads the named field and set its value in std::shared_ptr<CacheableDate>
   * type out param. std::shared_ptr<CacheableDate> type is corresponding to
   * java Java.util.date type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with
   * std::shared_ptr<CacheableDate> type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual std::shared_ptr<CacheableDate> getCacheableDateField(
      const char* fieldname) const;

  /**
   * Reads the named field and set its value in array of byte arrays type out
   * param.
   * int8_t** type is corresponding to java byte[][] type.
   * @param fieldname name of the field to read.
   * @param value value of the field to be set with array of byte arrays type.
   * @param arrayLength arrayLength is set to the number of byte arrays.
   * @param elementLength elementLength is set to individual byte array lengths.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual void getField(const char* fieldName, int8_t*** value,
                        int32_t& arrayLength, int32_t*& elementLength) const;

  /**
   * Reads the named field and set its value in std::shared_ptr<Cacheable> type
   * out param. std::shared_ptr<Cacheable> type is corresponding to java object
   * type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with std::shared_ptr<Cacheable>
   * type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   * For deserialization C++ Native Client requires the domain class to be
   * registered.
   *
   * @see serializationRegistry->addPdxType
   * @see PdxInstance#hasField
   */
  virtual std::shared_ptr<Cacheable> getCacheableField(
      const char* fieldname) const;

  /**
   * Reads the named field and set its value in
   * std::shared_ptr<CacheableObjectArray> type out param. For deserialization
   * C++ Native Client requires the domain class to be registered.
   * std::shared_ptr<CacheableObjectArray> type is corresponding to java
   * Object[] type.
   * @param fieldname name of the field to read.
   * @param value value of the field to be set with
   * std::shared_ptr<CacheableObjectArray> type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see serializationRegistry->addPdxType
   * @see PdxInstance#hasField
   */
  virtual std::shared_ptr<CacheableObjectArray> getCacheableObjectArrayField(
      const char* fieldname) const;

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * bool type is corresponding to java boolean type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type bool
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, bool value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * signed char type is corresponding to java byte type.
   * For C++ on Windows and Linux, signed char type is corresponding to int8_t
   * type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t to signed char.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type signed char
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, signed char value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * unsigned char type is corresponding to java byte type.
   * For C++ on Windows and Linux, unsigned char type is corresponding to int8_t
   * type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t to unsigned char.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type unsigned char
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, unsigned char value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int16_t type is corresponding to java short type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int16_t
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int16_t value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int32_t type is corresponding to java int type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int32_t
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int32_t value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int64_t type is corresponding to java long type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int64_t
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int64_t value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * float type is corresponding to java float type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type float
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, float value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * double type is corresponding to java double type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type double
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, double value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * char type is corresponding to java char type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type char
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, char16_t value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * std::shared_ptr<CacheableDate> type is corresponding to java Java.util.date
   * type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type
   * std::shared_ptr<CacheableDate>
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName,
                        std::shared_ptr<CacheableDate> value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * bool* type is corresponding to java boolean[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type bool array
   * @param length
   *          The number of elements in bool array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, bool* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * signed char* type is corresponding to java byte[] type.
   * For C++ on Windows and Linux, signed char* type is corresponding to int8_t*
   * type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t* to signed char*.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type signed char array
   * @param length
   *          The number of elements in signed char array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, signed char* value,
                        int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * unsigned char* type is corresponding to java byte[] type.
   * For C++ on Windows and Linux, unsigned char* type is corresponding to
   * int8_t* type.
   * However C++ users on Solaris should always use this api after casting
   * int8_t* to unsigned char*.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type unsigned char array
   * @param length
   *          The number of elements in unsigned char array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, unsigned char* value,
                        int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int16_t* type is corresponding to java short[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int16_t array
   * @param length
   *          The number of elements in int16_t array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int16_t* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int32_t* type is corresponding to java int[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int32_t array
   * @param length
   *          The number of elements in int32_t array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int32_t* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int64_t* type is corresponding to java long[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int64_t array
   * @param length
   *          The number of elements in int64_t array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int64_t* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * float* type is corresponding to java float[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type float array
   * @param length
   *          The number of elements in float array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, float* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * double* type is corresponding to java double[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type double array
   * @param length
   *          The number of elements in double array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, double* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * wchar_t* type is corresponding to java String type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type wchar_t*
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, const wchar_t* value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * char* type is corresponding to java String type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type char*
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, const char* value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * wchar_t* type is corresponding to java char[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type wchar_t array
   * @param length
   *          The number of elements in wchar_t array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, wchar_t* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * char* type is corresponding to java char[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type char array
   * @param length
   *          The number of elements in char array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, char* value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * wchar_t** type is corresponding to java String[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type wchar_t* array
   * @param length
   *          The number of elements in WCString array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, wchar_t** value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * char** type is corresponding to java String[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type char* array
   * @param length
   *          The number of elements in CString array type.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, char** value, int32_t length);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int8_t** type is corresponding to java byte[][] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type array of byte arrays
   * @param arrayLength
   *          The number of byte arrays.
   * @param elementLength
   *          The lengths of individual byte arrays.
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName, int8_t** value,
                        int32_t arrayLength, int32_t* elementLength);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   *  So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   *
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be assigned to the field of type
   * std::shared_ptr<Cacheable>
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName,
                        std::shared_ptr<Cacheable> value);

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * std::shared_ptr<CacheableObjectArray> type is corresponding to java
   * Object[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type
   * std::shared_ptr<CacheableObjectArray>
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const char* fieldName,
                        std::shared_ptr<CacheableObjectArray> value);

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
  virtual bool isIdentityField(const char* fieldname);

  /**
   * Creates and returns a {@link WritablePdxInstance} whose initial
   * values are those of this PdxInstance.
   * This call returns a copy of the current field values so modifications
   * made to the returned value will not modify this PdxInstance.
   * @return a {@link WritablePdxInstance}
   */
  virtual std::shared_ptr<WritablePdxInstance> createWriter();

  /**
   * Generates a hashcode based on the identity fields of
   * this PdxInstance.
   * <p>If a PdxInstance has marked identity fields using {@link
   * PdxWriter#markIdentityField}
   * then only the marked identity fields are its identity fields.
   * Otherwise all its fields are identity fields.
   * </p>
   * For deserialization C++ Native Client requires the domain class to be
   * registered.
   * If the field is an array then all array
   * elements are used for hashcode computation.
   * Otherwise the raw bytes of its value are used to compute the hash code.
   * @throws IllegalStateException if the field contains an element that is not
   * of CacheableKey derived type.
   *
   * @see serializationRegistry->addPdxType
   */
  virtual int32_t hashcode() const;

  /**
   * Prints out all of the identity fields of this PdxInstance.
   * <p>If a PdxInstance has marked identity fields using {@link
   * PdxWriter#markIdentityField}
   * then only the marked identity fields are its identity fields.
   * Otherwise all its fields are identity fields</p>.
   * For deserialization C++ Native Client requires the domain class to be
   * registered.
   *
   * @see serializationRegistry->addPdxType
   */
  virtual std::shared_ptr<CacheableString> toString() const;

  /**
   * @brief serialize this object. This is an internal method.
   */
  virtual void toData(DataOutput& output) const { PdxInstance::toData(output); }

  /**
   * @brief deserialize this object, typical implementation should return
   * the 'this' pointer. This is an internal method.
   */
  virtual void fromData(DataInput& input) { PdxInstance::fromData(input); }

  /**
   * Returns true if the given CacheableKey derived object is equals to this
   * instance.
   * <p>If <code>other</code> is not a PdxInstance then it is not equal to this
   * instance.
   * NOTE: Even if <code>other</code> is the result of calling {@link
   * #getObject()} it will not
   * be equal to this instance</p>.
   * <p>Otherwise equality of two PdxInstances is determined as follows:
   * <ol>
   * <li>The domain class name must be equal for both PdxInstances
   * <li>Each identity field must be equal.
   * </ol> </p>
   * If one of the instances does not have a field that the other one does then
   * equals will assume it
   * has the field with a default value.
   * If a PdxInstance has marked identity fields using {@link
   * PdxWriter#markIdentityField markIdentityField}
   * then only the marked identity fields are its identity fields.
   * Otherwise all its fields are identity fields.
   * <p>An identity field is equal if all the following are true:
   * <ol>
   * <li>The field name is equal.
   * <li>The field type is equal.
   * <li>The field value is equal.
   * </ol> </p>
   * If an identity field is of type derived from <code>Cacheable</code> then it
   * is deserialized. For deserialization C++ Native Client requires the domain
   * class to be registered.
   * If the deserialized object is an array then all array elements
   * are used to determine equality.
   * If an identity field is of type <code>CacheableObjectArray</code> then it
   * is deserialized and all array elements are used to determine equality. For
   * all other field types the value does not need to be deserialized. Instead
   * the serialized raw bytes are compared and used to determine equality.
   * @param other the other instance to compare to this.
   * @return <code>true</code> if this instance is equal to <code>other</code>.
   * @throws IllegalStateException if the field contains an element that is not
   * of CacheableKey derived type.
   *
   * @see serializationRegistry->addPdxType
   */
  virtual bool operator==(const CacheableKey& other) const;

  /** @return the size of the object in bytes
   * This is an internal method.
   * It is used in case of heap LRU property is set.
   */
  virtual uint32_t objectSize() const;

  /**
   * Return an unmodifiable list of the field names on this PdxInstance.
   * @return an unmodifiable list of the field names on this PdxInstance
   */
  virtual std::shared_ptr<CacheableStringArray> getFieldNames();

  // From PdxSerializable
  /**
   * @brief serialize this object in geode PDX format. This is an internal
   * method.
   * @param PdxWriter to serialize the PDX object
   */
  virtual void toData(std::shared_ptr<PdxWriter> output) /*const*/;

  /**
   * @brief Deserialize this object. This is an internal method.
   * @param PdxReader to Deserialize the PDX object
   */
  virtual void fromData(std::shared_ptr<PdxReader> input);

  /**
   * Return the full name of the class that this pdx instance represents.
   * @return the name of the class that this pdx instance represents.
   * @throws IllegalStateException if the PdxInstance typeid is not defined yet,
   * to get classname
   * or if PdxType is not defined for PdxInstance.
   */
  virtual const char* getClassName() const;

  virtual PdxFieldTypes::PdxFieldType getFieldType(const char* fieldname) const;

  void setPdxId(int32_t typeId);

 public:
  /**
   * @brief constructors
   */

  PdxInstanceImpl(uint8_t* buffer, int length, int typeId,
                  CachePerfStats* cacheStats,
                  std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry,
                  const Cache* cache, bool enableTimeStatistics)
      : m_buffer(DataInput::getBufferCopy(buffer, length)),
        m_bufferLength(length),
        m_typeId(typeId),
        m_pdxType(nullptr),
        m_cacheStats(cacheStats),
        m_pdxTypeRegistry(pdxTypeRegistry),
        m_cache(cache),
        m_enableTimeStatistics(enableTimeStatistics) {
    LOGDEBUG("PdxInstanceImpl::m_bufferLength = %d ", m_bufferLength);
  }

  PdxInstanceImpl(FieldVsValues fieldVsValue, std::shared_ptr<PdxType> pdxType,
                  CachePerfStats* cacheStats,
                  std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry,
                  const Cache* cache, bool enableTimeStatistics);

  PdxInstanceImpl(const PdxInstanceImpl& other) = delete;

  void operator=(const PdxInstanceImpl& other) = delete;

  std::shared_ptr<PdxType> getPdxType() const;

  void updatePdxStream(uint8_t* newPdxStream, int len);

 private:
  uint8_t* m_buffer;
  int m_bufferLength;
  int m_typeId;
  std::shared_ptr<PdxType> m_pdxType;
  FieldVsValues m_updatedFields;
  CachePerfStats* m_cacheStats;

  std::shared_ptr<PdxTypeRegistry> m_pdxTypeRegistry;
  const Cache* m_cache;
  bool m_enableTimeStatistics;

  std::vector<std::shared_ptr<PdxFieldType>> getIdentityPdxFields(
      std::shared_ptr<PdxType> pt) const;

  int getOffset(DataInput& dataInput, std::shared_ptr<PdxType> pt,
                int sequenceId) const;

  int getRawHashCode(std::shared_ptr<PdxType> pt,
                     std::shared_ptr<PdxFieldType> pField,
                     DataInput& dataInput) const;

  int getNextFieldPosition(DataInput& dataInput, int fieldId,
                           std::shared_ptr<PdxType> pt) const;

  int getSerializedLength(DataInput& dataInput,
                          std::shared_ptr<PdxType> pt) const;

  bool hasDefaultBytes(std::shared_ptr<PdxFieldType> pField,
                       DataInput& dataInput, int start, int end) const;

  bool compareDefaultBytes(DataInput& dataInput, int start, int end,
                           int8_t* defaultBytes, int32_t length) const;

  void writeField(std::shared_ptr<PdxWriter> writer, const char* fieldName,
                  int typeId, std::shared_ptr<Cacheable> value);

  void writeUnmodifieldField(DataInput& dataInput, int startPos, int endPos,
                             std::shared_ptr<PdxLocalWriter> localWriter);

  void setOffsetForObject(DataInput& dataInput, std::shared_ptr<PdxType> pt,
                          int sequenceId) const;

  bool compareRawBytes(PdxInstanceImpl& other, std::shared_ptr<PdxType> myPT,
                       std::shared_ptr<PdxFieldType> myF,
                       DataInput& myDataInput, std::shared_ptr<PdxType> otherPT,
                       std::shared_ptr<PdxFieldType> otherF,
                       DataInput& otherDataInput) const;

  void equatePdxFields(std::vector<std::shared_ptr<PdxFieldType>>& my,
                       std::vector<std::shared_ptr<PdxFieldType>>& other) const;

  std::shared_ptr<PdxTypeRegistry> getPdxTypeRegistry() const;

  static int deepArrayHashCode(std::shared_ptr<Cacheable> obj);

  static int enumerateMapHashCode(std::shared_ptr<CacheableHashMap> map);

  static int enumerateVectorHashCode(std::shared_ptr<CacheableVector> vec);

  static int enumerateArrayListHashCode(
      std::shared_ptr<CacheableArrayList> arrList);

  static int enumerateLinkedListHashCode(
      std::shared_ptr<CacheableLinkedList> linkedList);

  static int enumerateObjectArrayHashCode(
      std::shared_ptr<CacheableObjectArray> objArray);

  static int enumerateSetHashCode(std::shared_ptr<CacheableHashSet> set);

  static int enumerateLinkedSetHashCode(
      std::shared_ptr<CacheableLinkedHashSet> linkedset);

  static int enumerateHashTableCode(
      std::shared_ptr<CacheableHashTable> hashTable);

  static bool deepArrayEquals(std::shared_ptr<Cacheable> obj,
                              std::shared_ptr<Cacheable> otherObj);

  static bool enumerateObjectArrayEquals(
      std::shared_ptr<CacheableObjectArray> Obj,
      std::shared_ptr<CacheableObjectArray> OtherObj);

  static bool enumerateVectorEquals(std::shared_ptr<CacheableVector> Obj,
                                    std::shared_ptr<CacheableVector> OtherObj);

  static bool enumerateArrayListEquals(
      std::shared_ptr<CacheableArrayList> Obj,
      std::shared_ptr<CacheableArrayList> OtherObj);

  static bool enumerateMapEquals(std::shared_ptr<CacheableHashMap> Obj,
                                 std::shared_ptr<CacheableHashMap> OtherObj);

  static bool enumerateSetEquals(std::shared_ptr<CacheableHashSet> Obj,
                                 std::shared_ptr<CacheableHashSet> OtherObj);

  static bool enumerateLinkedSetEquals(
      std::shared_ptr<CacheableLinkedHashSet> Obj,
      std::shared_ptr<CacheableLinkedHashSet> OtherObj);

  static bool enumerateHashTableEquals(
      std::shared_ptr<CacheableHashTable> Obj,
      std::shared_ptr<CacheableHashTable> OtherObj);

  std::unique_ptr<DataInput> getDataInputForField(const char* fieldname) const;

  static int8_t m_BooleanDefaultBytes[];
  static int8_t m_ByteDefaultBytes[];
  static int8_t m_CharDefaultBytes[];
  static int8_t m_ShortDefaultBytes[];
  static int8_t m_IntDefaultBytes[];
  static int8_t m_LongDefaultBytes[];
  static int8_t m_FloatDefaultBytes[];
  static int8_t m_DoubleDefaultBytes[];
  static int8_t m_DateDefaultBytes[];
  static int8_t m_StringDefaultBytes[];
  static int8_t m_ObjectDefaultBytes[];
  static int8_t m_NULLARRAYDefaultBytes[];
  static std::shared_ptr<PdxFieldType> m_DefaultPdxFieldType;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXINSTANCEIMPL_H_
