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

#ifndef GEODE_STRUCT_H_
#define GEODE_STRUCT_H_

#include <unordered_map>
#include <vector>

#include "geode_globals.hpp"
#include "CacheableBuiltins.hpp"
#include "StructSet.hpp"
#include "SelectResults.hpp"
#include "Serializable.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class StructSet;

/**
 * @class Struct Struct.hpp
 * A Struct has a StructSet as its parent. It contains the field values
 * returned after executing a Query obtained from a QueryService which in turn
 * is obtained from a Cache.
 */
class CPPCACHE_EXPORT Struct : public Serializable {
 public:
  /**
   * Constructor - meant only for internal use.
   */
  Struct(StructSet* ssPtr,
         std::vector<std::shared_ptr<Serializable>>& fieldValues);

  /**
   * Factory function for registration of <code>Struct</code>.
   */
  static Serializable* createDeserializable();

  /**
   * Get the field value for the given index number.
   *
   * @param index the index number of the field value to get.
   * @returns A smart pointer to the field value or nullptr if index out of
   * bounds.
   */
  const std::shared_ptr<Serializable> operator[](int32_t index) const;

  /**
   * Get the field value for the given field name.
   *
   * @param fieldName the name of the field whos value is required.
   * @returns A smart pointer to the field value.
   * @throws IllegalArgumentException if the field name is not found.
   */
  const std::shared_ptr<Serializable> operator[](
      const std::string& fieldName) const;

  /**
   * Get the parent StructSet of this Struct.
   *
   * @returns A smart pointer to the parent StructSet of this Struct.
   */
  const std::shared_ptr<StructSet> getStructSet() const;

  /**
   * Check whether another field value is available to iterate over in this
   * Struct.
   *
   * @returns true if available otherwise false.
   */
  bool hasNext() const;

  /**
   * Get the number of field values available.
   *
   * @returns the number of field values available.
   */
  int32_t length() const;

  /**
   * Get the next field value item available in this Struct.
   *
   * @returns A smart pointer to the next item in the Struct or nullptr if no
   * more available.
   */
  const std::shared_ptr<Serializable> next();

  /**
   * Deserializes the Struct object from the DataInput. @TODO KN: better comment
   */
  virtual void fromData(DataInput& input);

  /**
   * Serializes this Struct object. @TODO KN: better comment
   */
  virtual void toData(DataOutput& output) const;

  /**
   * Returns the classId for internal use.
   */
  virtual int32_t classId() const;

  /**
   * Returns the typeId of Struct class.
   */
  virtual int8_t typeId() const;

  /**
   * Return the data serializable fixed ID size type for internal use.
   * @since GFE 5.7
   */
  virtual int8_t DSFID() const;

  /**
   * Returns the name of the field corresponding to the index number in the
   * Struct
   * @throws std::out_of_range if index is not found
   */
  virtual const std::string& getFieldName(const int32_t index) const;

  /**
   * always returns 0
   */
  virtual size_t objectSize() const {
    return 0;  // does not get cached, so no need to account for it
  }

 private:
  void skipClassName(DataInput& input);

  Struct();

  typedef std::unordered_map<std::string, int32_t> FieldNames;
  FieldNames m_fieldNames;
  std::vector<std::shared_ptr<Serializable>> m_fieldValues;

  StructSet* m_parent;

  int32_t m_lastAccessIndex;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STRUCT_H_
