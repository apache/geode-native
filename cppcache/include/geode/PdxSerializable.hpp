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

#ifndef GEODE_PDXSERIALIZABLE_H_
#define GEODE_PDXSERIALIZABLE_H_

#include <iosfwd>
#include <memory>
#include <string>

#include "CacheableKey.hpp"
#include "geode_base.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxReader;
class PdxWriter;
class DataInput;
class DataOutput;

class CPPCACHE_EXPORT PdxSerializable : public CacheableKey {
 public:
  PdxSerializable();
  virtual ~PdxSerializable();

  /**
   *@brief serialize this object in geode PDX format
   *@param PdxWriter to serialize the PDX object
   **/
  virtual void toData(PdxWriter& output) const = 0;

  /**
   *@brief Deserialize this object
   *@param PdxReader to Deserialize the PDX object
   **/
  virtual void fromData(PdxReader& input) = 0;

  /**
   *@brief return the typeId byte of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   *
   * Note that this should not be overridden by custom implementations
   * and is reserved only for builtin types.
   */
  virtual int8_t typeId() const override;

  /** return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const override;

  /** return the hashcode for this key. */
  virtual int32_t hashcode() const override;

  /**
   *@brief serialize this object
   **/
  virtual void toData(DataOutput& output) const override;

  /**
   *@brief deserialize this object, typical implementation should return
   * the 'this' pointer.
   **/
  virtual void fromData(DataInput& input) override;

  /**
   *@brief return the classId of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and derserialize into.
   */
  virtual int32_t classId() const override { return 0x10; }

  /**
   * Display this object as 'string', which depends on the implementation in
   * the subclasses.
   * The default implementation renders the classname.
   */
  virtual std::string toString() const override;

  /**
   * Get the Type for the Object. Equivalent to the C# Type->GetType() API.
   */
  virtual const std::string& getClassName() const = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXSERIALIZABLE_H_
