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
#include "internal/geode_base.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxReader;
class PdxWriter;

/**
 * An interface for objects whose contents can be serialized as PDX types.
 */
class APACHE_GEODE_EXPORT PdxSerializable : public virtual Serializable,
                                            public virtual CacheableKey {
 public:
  ~PdxSerializable() noexcept override {}

  std::string toString() const override;

  bool operator==(const CacheableKey& other) const override;

  int32_t hashcode() const override;

  /**
   *@brief Serialize this object in Geode PDX format
   *@param output to serialize the PDX object
   **/
  virtual void toData(PdxWriter& output) const = 0;

  /**
   *@brief Deserialize this object
   *@param input to deserialize the PDX object
   **/
  virtual void fromData(PdxReader& input) = 0;

  /**
   * Get the Type for the Object. Equivalent to the C# Type->GetType() API.
   */
  virtual const std::string& getClassName() const = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXSERIALIZABLE_H_
