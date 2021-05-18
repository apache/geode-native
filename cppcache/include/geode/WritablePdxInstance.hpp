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

#ifndef GEODE_WRITABLEPDXINSTANCE_H_
#define GEODE_WRITABLEPDXINSTANCE_H_

#include <memory>

#include "PdxInstance.hpp"
#include "Serializable.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheableObjectArray;

/**
 * WritablePdxInstance is a {@link PdxInstance} that also supports field
 * modification
 * using the {@link #setField} method.
 * To get a WritablePdxInstance call {@link PdxInstance#createWriter}.
 */
class APACHE_GEODE_EXPORT WritablePdxInstance : public PdxInstance {
 public:
  ~WritablePdxInstance() noexcept override = default;

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * std::shared_ptr<Cacheable> type is corresponding to java object type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type
   * std::shared_ptr<Cacheable>
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<Cacheable> value) = 0;

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
  virtual void setField(const std::string& fieldName, bool value) = 0;

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
  virtual void setField(const std::string& fieldName, signed char value) = 0;

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
  virtual void setField(const std::string& fieldName, unsigned char value) = 0;

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
  virtual void setField(const std::string& fieldName, int16_t value) = 0;

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
  virtual void setField(const std::string& fieldName, int32_t value) = 0;

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
  virtual void setField(const std::string& fieldName, int64_t value) = 0;

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
  virtual void setField(const std::string& fieldName, float value) = 0;

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
  virtual void setField(const std::string& fieldName, double value) = 0;

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
  virtual void setField(const std::string& fieldName, char16_t value) = 0;

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
  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<CacheableDate> value) = 0;

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
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<bool>& value) = 0;

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * int8_t* type is corresponding to java byte[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type int8_t array
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<int8_t>& value) = 0;

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
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<int16_t>& value) = 0;

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
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<int32_t>& value) = 0;

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
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<int64_t>& value) = 0;

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
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<float>& value) = 0;

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
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<double>& value) = 0;

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
  virtual void setField(const std::string& fieldName,
                        const std::string& value) = 0;

  /**
   * Set the existing named field to the given value.
   * The setField method has copy-on-write semantics.
   * So for the modifications to be stored in the cache the WritablePdxInstance
   * must be put into a region after setField has been called one or more times.
   * char16_t* type is corresponding to java char[] type.
   * @param fieldName
   *          name of the field whose value will be set
   * @param value
   *          value that will be set to the field of type char16_t array
   * @throws IllegalStateException if the named field does not exist
   * or if the type of the value is not compatible with the field.
   */
  virtual void setField(const std::string& fieldName,
                        const std::vector<char16_t>& value) = 0;

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
  virtual void setField(const std::string& fieldName, std::string* value,
                        int32_t length) = 0;

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
  virtual void setField(const std::string& fieldName, int8_t** value,
                        int32_t arrayLength, int32_t* elementLength) = 0;

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
  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<CacheableObjectArray> value) = 0;

 protected:
  /**
   * @brief constructors
   */
  WritablePdxInstance() = default;

 private:
  WritablePdxInstance(const WritablePdxInstance& other) = delete;
  void operator=(const WritablePdxInstance& other) = delete;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_WRITABLEPDXINSTANCE_H_
