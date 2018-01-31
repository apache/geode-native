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

#ifndef GEODE_PDXINSTANCE_H_
#define GEODE_PDXINSTANCE_H_

#include "PdxSerializable.hpp"
#include "CacheableBuiltins.hpp"
#include "PdxFieldTypes.hpp"

namespace apache {
namespace geode {
namespace client {
class WritablePdxInstance;
class CacheableDate;
class CacheableObjectArray;
/**
 * PdxInstance provides run time access to the fields of a PDX without
 * deserializing the PDX. Preventing deserialization saves time
 * and memory.
 * The PdxInstance implementation
 * is a light weight wrapper that simply refers to the raw bytes of the PDX
 * that are kept in the cache.
 * Applications can choose to access PdxInstances instead of C++ objects by
 * configuring the Cache to prefer PDX instances during deserialization.
 * This can be done in <code>cache.xml</code> by setting the attribute
 * <code>read-serialized</code>
 * to true on the <code>pdx</code> element. Or it can be done programmatically
 * using
 * {@link CacheFactory#setPdxReadSerialized(boolean) setPdxReadSerialized}
 * method. Once this preference is configured, then any time deserialization of
 * a PDX is done it will deserialize into a PdxInstance. PdxInstance are
 * immutable. If you want to change one call {@link #createWriter}.
 */
class _GEODE_EXPORT PdxInstance : public PdxSerializable {
 public:
  /**
   * @brief destructor
   */
  virtual ~PdxInstance() {}

  /**
   * Deserializes and returns the domain object that this instance represents.
   * For deserialization C++ Native Client requires the domain class to be
   * registered.
   * @return the deserialized domain object.
   *
   * @see serializationRegistry->addPdxType
   */
  virtual std::shared_ptr<PdxSerializable> getObject() = 0;

  /**
   * Checks if the named field exists and returns the result.
   * This can be useful when writing code that handles more than one version of
   * a PDX class.
   * @param fieldname the name of the field to check
   * @return <code>true</code> if the named field exists; otherwise
   * <code>false</code>
   */
  virtual bool hasField(const std::string& fieldname) = 0;

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
      const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in bool type out param.
   * bool type is corresponding to java boolean type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with bool type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual bool getBooleanField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in signed char type out param.
   * signed char type is corresponding to java byte type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with signed char type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual int8_t getByteField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in int16_t type out param.
   * int16_t type is corresponding to java short type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int16_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual int16_t getShortField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in int32_t type out param.
   * int32_t type is corresponding to java int type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int32_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   */
  virtual int32_t getIntField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in int64_t type out param.
   * int64_t type is corresponding to java long type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int64_t type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual int64_t getLongField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in float type out param.
   * float type is corresponding to java float type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with float type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual float getFloatField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in double type out param.
   * double type is corresponding to java double type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with double type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual double getDoubleField(const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in char type out param.
   * char type is corresponding to java char type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with char type.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual char16_t getCharField(const std::string& fieldName) const = 0;

  /**
   * Reads the named field and set its value in std::string type out param.
   * std::string type is corresponding to java String type.
   * @param fieldname name of the field to read
   * @return string value for field.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual std::string getStringField(const std::string& fieldname) const = 0;

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
  virtual std::vector<bool> getBooleanArrayField(
      const std::string& fieldname) const = 0;

  /**
   * Reads the named field and set its value in signed char array type out
   * param. int8_t* type is corresponding to java byte[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with int8_t array type.
   * @param length length is set with number of int8_t elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual std::vector<int8_t> getByteArrayField(
      const std::string& fieldname) const = 0;

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
  virtual std::vector<int16_t> getShortArrayField(
      const std::string& fieldname) const = 0;

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
  virtual std::vector<int32_t> getIntArrayField(
      const std::string& fieldname) const = 0;

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
  virtual std::vector<int64_t> getLongArrayField(
      const std::string& fieldname) const = 0;

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
  virtual std::vector<float> getFloatArrayField(
      const std::string& fieldname) const = 0;

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
  virtual std::vector<double> getDoubleArrayField(
      const std::string& fieldname) const = 0;

  // charArray
  /**
   * Reads the named field and set its value in char array type out param.
   * char16_t* type is corresponding to java char[] type.
   * @param fieldname name of the field to read
   * @param value value of the field to be set with char array type.
   * @param length length is set with number of char16_t* elements.
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual std::vector<char16_t> getCharArrayField(
      const std::string& fieldName) const = 0;

  /**
   * Reads the named field as a string array.
   * std::vector<std::string> type is corresponding to java String[] type.
   * @param fieldname name of the field to read
   * @throws IllegalStateException if PdxInstance doesn't has the named field.
   *
   * @see PdxInstance#hasField
   */
  virtual std::vector<std::string> getStringArrayField(
      const std::string& fieldname) const = 0;

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
      const std::string& fieldname) const = 0;

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
  virtual void getField(const std::string& fieldName, int8_t*** value,
                        int32_t& arrayLength,
                        int32_t*& elementLength) const = 0;

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
      const std::string& fieldname) const = 0;

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
  virtual bool isIdentityField(const std::string& fieldname) = 0;

  /**
   * Creates and returns a {@link WritablePdxInstance} whose initial
   * values are those of this PdxInstance.
   * This call returns a copy of the current field values so modifications
   * made to the returned value will not modify this PdxInstance.
   * @return a {@link WritablePdxInstance}
   */
  virtual std::shared_ptr<WritablePdxInstance> createWriter() = 0;

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
  virtual int32_t hashcode() const override = 0;

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
  virtual std::string toString() const override = 0;

  /**
   * @brief serialize this object. This is an internal method.
   */
  virtual void toData(DataOutput& output) const override {
    PdxSerializable::toData(output);
  }

  /**
   * @brief deserialize this object, typical implementation should return
   * the 'this' pointer. This is an internal method.
   */
  virtual void fromData(DataInput& input) override {
    return PdxSerializable::fromData(input);
  }

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
  virtual bool operator==(const CacheableKey& other) const override = 0;

  /** @return the size of the object in bytes
   * This is an internal method.
   * It is used in case of heap LRU property is set.
   */
  virtual size_t objectSize() const override = 0;

  /**
   * Return an unmodifiable list of the field names on this PdxInstance.
   * @return an unmodifiable list of the field names on this PdxInstance
   */
  virtual std::shared_ptr<CacheableStringArray> getFieldNames() = 0;

  // From PdxSerializable
  /**
   * @brief serialize this object in geode PDX format. This is an internal
   * method.
   * @param PdxWriter to serialize the PDX object
   */
  virtual void toData(PdxWriter& output) const override = 0;

  /**
   * @brief Deserialize this object. This is an internal method.
   * @param PdxReader to Deserialize the PDX object
   */
  virtual void fromData(PdxReader& input) override = 0;

  /**
   * Return the full name of the class that this pdx instance represents.
   * @return the name of the class that this pdx instance represents.
   * @throws IllegalStateException if the PdxInstance typeid is not defined yet,
   * to get classname
   * or if PdxType is not defined for PdxInstance.
   */
  virtual const std::string& getClassName() const override = 0;

  /**
   * Return the type @see PdxInstance::PdxFieldTypes of the field in the pdx
   * instance.
   * @return the type @see PdxInstance::PdxFieldTypes of the field in the pdx
   * instance.
   * @throws IllegalStateException if the PdxInstance typeid is not defined yet,
   * to get classname or if PdxType is not defined for PdxInstance.
   */
  virtual PdxFieldTypes::PdxFieldType getFieldType(
      const std::string& fieldname) const = 0;

 protected:
  /**
   * @brief constructors
   */
  PdxInstance() {}

 private:
  // never implemented.
  PdxInstance(const PdxInstance& other);
  void operator=(const PdxInstance& other);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXINSTANCE_H_
