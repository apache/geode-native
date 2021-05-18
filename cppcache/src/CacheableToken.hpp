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

#ifndef GEODE_CACHEABLETOKEN_H_
#define GEODE_CACHEABLETOKEN_H_

#include <geode/Serializable.hpp>
#include <geode/internal/DataSerializableInternal.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

/** Implement a non-mutable int64_t wrapper that can serve as a distributable
 * key object for cacheing as well as being a 64 bit value. */
class APACHE_GEODE_EXPORT CacheableToken
    : public internal::DataSerializableInternal {
 private:
  enum TokenType { NOT_USED = 0, INVALID, DESTROYED, OVERFLOWED, TOMBSTONE };

  TokenType m_value;

  static std::shared_ptr<CacheableToken> invalidToken;
  static std::shared_ptr<CacheableToken> destroyedToken;
  static std::shared_ptr<CacheableToken> overflowedToken;
  static std::shared_ptr<CacheableToken> tombstoneToken;

 public:
  inline static std::shared_ptr<CacheableToken>& invalid() {
    return invalidToken;
  }
  inline static std::shared_ptr<CacheableToken>& destroyed() {
    return destroyedToken;
  }
  inline static std::shared_ptr<CacheableToken>& overflowed() {
    return overflowedToken;
  }
  inline static std::shared_ptr<CacheableToken>& tombstone() {
    return tombstoneToken;
  }
  /**
   *@brief serialize this object
   **/
  void toData(DataOutput& output) const override;

  /**
   *@brief deserialize this object
   **/
  virtual void fromData(DataInput& input) override;

  /**
   * @brief creation function for strings.
   */
  static std::shared_ptr<Serializable> createDeserializable();

  ~CacheableToken() override = default;

  inline bool isInvalid() { return m_value == INVALID; }

  inline bool isDestroyed() { return m_value == DESTROYED; }

  inline bool isOverflowed() { return m_value == OVERFLOWED; }

  inline bool isTombstone() { return m_value == TOMBSTONE; }

  static bool isToken(const std::shared_ptr<Cacheable>& ptr) {
    return (invalidToken == ptr) || (destroyedToken == ptr) ||
           (overflowedToken == ptr) || (tombstoneToken == ptr);
  }

  static bool isInvalid(const std::shared_ptr<Cacheable>& ptr) {
    return invalidToken == ptr;
  }

  static bool isDestroyed(const std::shared_ptr<Cacheable>& ptr) {
    return destroyedToken == ptr;
  }

  static bool isOverflowed(const std::shared_ptr<Cacheable>& ptr) {
    return overflowedToken == ptr;
  }

  static bool isTombstone(const std::shared_ptr<Cacheable>& ptr) {
    return tombstoneToken == ptr;
  }

  /**
   * Display this object as 'string', which depend on the implementation in
   * the subclasses. The default implementation renders the classname.
   * This returns constant strings of the form "CacheableToken::INVALID".
   */
  virtual std::string toString() const override;

  virtual size_t objectSize() const override;

  CacheableToken();  // used for deserialization.
  explicit CacheableToken(TokenType value);

 private:
  // never implemented.
  void operator=(const CacheableToken& other);
  CacheableToken(const CacheableToken& other);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLETOKEN_H_
