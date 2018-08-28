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

#ifndef GEODE_CACHEABLEENUM_H_
#define GEODE_CACHEABLEENUM_H_

#include <iosfwd>
#include <memory>
#include <string>

#include "CacheableKey.hpp"
#include "CacheableString.hpp"
#include "internal/geode_base.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Since C++ enums cannot be directly passed as a parameter to PdxWriter's
 * writeObject and PdxReader's readObject api
 * wrap C++ enum in to a immutable wrapper CacheableEnum class type by
 * specifying enum class name, enum value name and its ordinal. C++ enum allows
 * explicit setting of ordinal number, but it is up to the user to map java
 * enumName with that of C++ enumName. Currently this wrapper only works as part
 * of PdxSerializable member object and cannot be directly used in Region
 * operations.
 *
 * @see PdxWriter#writeObject
 * @see PdxReader#readObject
 */

class DataInput;
class DataOutput;
class Serializable;

class APACHE_GEODE_EXPORT CacheableEnum : public DataSerializablePrimitive,
                                          public CacheableKey {
 private:
  std::string m_enumClassName;
  std::string m_enumName;
  int32_t m_ordinal;
  int32_t m_hashcode;

 public:
  CacheableEnum();

  CacheableEnum(std::string enumClassName, std::string enumName,
                int32_t ordinal);

  /** Destructor */
  ~CacheableEnum() override = default;

  /**
   * @brief creation function for enum.
   */
  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableEnum>();
  }

  void toData(DataOutput& output) const override;

  virtual void fromData(DataInput& input) override;

  virtual size_t objectSize() const override {
    auto size = sizeof(CacheableEnum);
    size += m_enumClassName.length();
    size += m_enumName.length();
    return size;
  }

  virtual DSCode getDsCode() const override {
    return DSCode::CacheableEnum;
  }

  /**
   * Display this object as c string.
   */
  virtual std::string toString() const override { return "CacheableEnum"; }

  /**
   * Factory method for creating an instance of CacheableEnum.
   * @param className the name of the enum class that maps to the java enum
   * type.
   * @param enumName the name of the enum constant that maps to the java enum
   * type.
   * @param ordinal the ordinal value of the enum constant that maps to the java
   * enum type.
   * @return a {@link CacheableEnum} representing C++ enum.
   */
  static std::shared_ptr<CacheableEnum> create(std::string enumClassName,
                                               std::string enumName,
                                               int32_t ordinal) {
    return std::make_shared<CacheableEnum>(enumClassName, enumName, ordinal);
  }

  /**@return enum class name. */
  const std::string& getEnumClassName() const { return m_enumClassName; }

  /**@return enum name. */
  const std::string& getEnumName() const { return m_enumName; }

  /**@return enum ordinal. */
  int32_t getEnumOrdinal() const { return m_ordinal; }

  /** @return the hashcode for this key. */
  virtual int32_t hashcode() const override { return m_hashcode; }

  /** @return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const override;

 protected:
  void calculateHashcode();

 private:
  // never implemented.
  void operator=(const CacheableEnum& other);
  CacheableEnum(const CacheableEnum& other);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEENUM_H_
