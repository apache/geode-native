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

#ifndef GEODE_PDXINSTANCEFACTORY_H_
#define GEODE_PDXINSTANCEFACTORY_H_

#include <map>

#include "CacheableBuiltins.hpp"
#include "CacheableDate.hpp"
#include "CacheableObjectArray.hpp"
#include "PdxInstance.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxType;
typedef std::map<std::string, std::shared_ptr<Cacheable>> FieldVsValues;
class CachePerfStats;
class PdxTypeRegistry;
class AuthenticatedView;

/**
 * PdxInstanceFactory gives you a way to create PdxInstances.
 * Call the write methods to populate the field data and then call {@link
 * #create}
 * to produce an actual instance that contains the data.
 * To create a factory call {@link Cache#createPdxInstanceFactory}.
 * A factory can only create a single instance. To create multiple instances
 * create
 * multiple factories or use {@link PdxInstance#createWriter} to create
 * subsequent instances.
 */
class APACHE_GEODE_EXPORT PdxInstanceFactory {
 public:
  PdxInstanceFactory() = delete;
  ~PdxInstanceFactory() noexcept = default;

 public:
  /**
   * Create a {@link PdxInstance}. The instance
   * will contain any data written to this factory
   * using the write methods.
   * @return the created Pdxinstance
   * @throws IllegalStateException if called more than once
   */
  std::shared_ptr<PdxInstance> create();

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>char16_t</code>.
   * <p>Java char is mapped to C++ char16_t.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeChar(const std::string& fieldName, char16_t value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>char</code>.
   * <p>Java char is mapped to C++ char.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeChar(const std::string& fieldName, char value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>bool</code>.
   * <p>Java boolean is mapped to C++ bool.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeBoolean(const std::string& fieldName, bool value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int8_t</code>.
   * <p>Java byte is mapped to C++ int8_t</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeByte(const std::string& fieldName, int8_t value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int16_t</code>.
   * <p>Java short is mapped to C++ int16_t.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeShort(const std::string& fieldName, int16_t value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int32_t</code>.
   * <p>Java int is mapped to C++ int32_t.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeInt(const std::string& fieldName, int32_t value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int64_t</code>.
   * <p>Java long is mapped to C++ int64_t.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeLong(const std::string& fieldName, int64_t value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>float</code>.
   * <p>Java float is mapped to C++ float.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeFloat(const std::string& fieldName, float value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>double</code>.
   * <p>Java double is mapped to C++ double.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeDouble(const std::string& fieldName, double value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>CacheableDatePtr</code>.
   * <p>Java Date is mapped to C++ std::shared_ptr<CacheableDate>.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeDate(const std::string& fieldName,
                                std::shared_ptr<CacheableDate> value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>std::string</code>.
   * <p>Java String is mapped to C++ std::string.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeString(const std::string& fieldName,
                                  const std::string& value);

  PdxInstanceFactory& writeString(const std::string& fieldName,
                                  std::string&& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>CacheablePtr</code>.
   * <p>Java object is mapped to C++ std::shared_ptr<Cacheable>.</p>
   * It is best to use one of the other writeXXX methods if your field type
   * will always be XXX. This method allows the field value to be anything
   * that is an instance of Object. This gives you more flexibility but more
   * space is used to store the serialized field.
   *
   * Note that some Java objects serialized with this method may not be
   * compatible with non-java languages.
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeObject(const std::string& fieldName,
                                  std::shared_ptr<Cacheable> value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>bool*</code>.
   * <p>Java boolean[] is mapped to C++ bool*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeBooleanArray(const std::string& fieldName,
                                        const std::vector<bool>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>char16_t*</code>.
   * <p>Java char[] is mapped to C++ char16_t*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeCharArray(const std::string& fieldName,
                                     const std::vector<char16_t>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int8_t*</code>.
   * <p>Java byte[] is mapped to C++ int8_t*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeByteArray(const std::string& fieldName,
                                     const std::vector<int8_t>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int16_t*</code>.
   * <p>Java short[] is mapped to C++ int16_t*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeShortArray(const std::string& fieldName,
                                      const std::vector<int16_t>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int32_t*</code>.
   * <p>Java int[] is mapped to C++ int32_t*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeIntArray(const std::string& fieldName,
                                    const std::vector<int32_t>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int64_t*</code>.
   * <p>Java long[] is mapped to C++ int64_t*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeLongArray(const std::string& fieldName,
                                     const std::vector<int64_t>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>float*</code>.
   * <p>Java float[] is mapped to C++ float*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeFloatArray(const std::string& fieldName,
                                      const std::vector<float>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>double*</code>.
   * <p>Java double[] is mapped to C++ double*.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeDoubleArray(const std::string& fieldName,
                                       const std::vector<double>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>char**</code>.
   * <p>Java String[] is mapped to C++ char**.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeStringArray(const std::string& fieldName,
                                       const std::vector<std::string>& value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>CacheableObjectArrayPtr</code>.
   * Java Object[] is mapped to C++ std::shared_ptr<CacheableObjectArray>.
   * For how each element of the array is a mapped to C++ see {@link
   * #writeObject}.
   * Note that this call may serialize elements that are not compatible with
   * non-java languages.
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeObjectArray(
      const std::string& fieldName,
      std::shared_ptr<CacheableObjectArray> value);

  /**
   * Writes the named field with the given value to the serialized form.
   * The fields type is <code>int8_t**</code>.
   * <p>Java byte[][] is mapped to C++ int8_t**.</p>
   * @param fieldName the name of the field to write
   * @param value the value of the field to write
   * @param arrayLength the length of the actual byte array field holding
   * individual byte arrays to write
   * @param elementLength the length of the individual byte arrays to write
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field has already been written
   * or fieldName is nullptr or empty.
   */
  PdxInstanceFactory& writeArrayOfByteArrays(const std::string& fieldName,
                                             int8_t** value,
                                             int32_t arrayLength,
                                             int32_t* elementLength);

  /**
   * Indicate that the named field should be included in hashCode and equals
   * (operator==()) checks
   * of this object on a server that is accessing {@link PdxInstance}
   * or when a client executes a query on a server.
   *
   * The fields that are marked as identity fields are used to generate the
   * hashCode and
   * equals (operator==()) methods of {@link PdxInstance}. Because of this, the
   * identity fields should themselves
   * either be primitives, or implement hashCode and equals (operator==()).
   *
   * If no fields are set as identity fields, then all fields will be used in
   * hashCode and equals (operator==())
   * checks.
   *
   * The identity fields should make marked after they are written using a
   * write* method.
   *
   * @param fieldName the name of the field to mark as an identity field.
   * @return this PdxInstanceFactory
   * @throws IllegalStateException if the named field does not exist.
   */
  PdxInstanceFactory& markIdentityField(const std::string& fieldName);

 private:
  bool m_created;
  std::shared_ptr<PdxType> m_pdxType;
  FieldVsValues m_FieldVsValues;
  CachePerfStats& m_cachePerfStats;
  PdxTypeRegistry& m_pdxTypeRegistry;
  const CacheImpl& m_cacheImpl;
  bool m_enableTimeStatistics;
  void isFieldAdded(const std::string& fieldName);

  PdxInstanceFactory(const std::string& className, bool expectDomainClass,
                     CachePerfStats& cachePerfStats,
                     PdxTypeRegistry& m_pdxTypeRegistry, const CacheImpl& cache,
                     bool enableTimeStatistics);

  friend CacheImpl;
  friend AuthenticatedView;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXINSTANCEFACTORY_H_
