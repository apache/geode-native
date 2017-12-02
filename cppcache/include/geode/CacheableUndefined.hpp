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

#include "geode_globals.hpp"
#include "Cacheable.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * Encapsulate an undefined query result.
 */
class CPPCACHE_EXPORT CacheableUndefined : public Cacheable {
 public:
  /**
   *@brief serialize this object
   **/
  virtual void toData(DataOutput& output) const override;

  /**
   *@brief deserialize this object
   **/
  virtual void fromData(DataInput& input) override;

  /**
   * @brief creation function for undefined query result
   */
  inline static Serializable* createDeserializable() {
    return new CacheableUndefined();
  }

  /**
   *@brief Return the classId of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int32_t classId() const override;

  /**
   *@brief return the typeId byte of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int8_t typeId() const override;

  /**
   * @brief Return the data serialization fixed ID size type for internal use.
   * @since GFE 5.7
   */
  virtual int8_t DSFID() const override;

  /**
   * Factory method for creating the default instance of CacheableUndefined.
   */
  inline static std::shared_ptr<CacheableUndefined> create() {
    return std::make_shared<CacheableUndefined>();
  }

  virtual uint32_t objectSize() const override;

 protected:
  /** Constructor, used for deserialization. */
  inline CacheableUndefined() = default;

 private:
  // never implemented.
  CacheableUndefined& operator=(const CacheableUndefined& other) = delete;
  CacheableUndefined(const CacheableUndefined& other) = delete;

  FRIEND_STD_SHARED_PTR(CacheableUndefined)
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEUNDEFINED_H_
