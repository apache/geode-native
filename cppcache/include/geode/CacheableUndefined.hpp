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

#ifndef GEODE_CACHEABLEUNDEFINED_H_
#define GEODE_CACHEABLEUNDEFINED_H_

#include <memory>

#include "internal/DataSerializableFixedId.hpp"
#include "internal/geode_globals.hpp"
#include "Serializable.hpp"
#include "internal/DSFixedId.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

class DataInput;
class DataOutput;
class Serializable;

class APACHE_GEODE_EXPORT CacheableUndefined
   : public internal::DataSerializableFixedId_t<internal::DSFid::CacheableUndefined> {
 public:
  void toData(DataOutput&) const override;

  void fromData(DataInput&) override;

  /**
   * @brief creation function for undefined query result
   */
  inline static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<CacheableUndefined>();
  }

  /**
   * Factory method for creating the default instance of CacheableUndefined.
   */
  inline static std::shared_ptr<CacheableUndefined> create() {
    return std::make_shared<CacheableUndefined>();
  }

  /** Constructor, used for deserialization. */
  inline CacheableUndefined() = default;

 private:
  // never implemented.
  CacheableUndefined& operator=(const CacheableUndefined& other) = delete;
  CacheableUndefined(const CacheableUndefined& other) = delete;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEUNDEFINED_H_
