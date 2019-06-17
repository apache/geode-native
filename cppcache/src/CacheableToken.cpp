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
#include "CacheableToken.hpp"

#include <geode/CacheableString.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

//---- statics
std::shared_ptr<CacheableToken> CacheableToken::invalidToken =
    std::make_shared<CacheableToken>(CacheableToken::INVALID);
std::shared_ptr<CacheableToken> CacheableToken::destroyedToken =
    std::make_shared<CacheableToken>(CacheableToken::DESTROYED);
std::shared_ptr<CacheableToken> CacheableToken::overflowedToken =
    std::make_shared<CacheableToken>(CacheableToken::OVERFLOWED);
std::shared_ptr<CacheableToken> CacheableToken::tombstoneToken =
    std::make_shared<CacheableToken>(CacheableToken::TOMBSTONE);

//----- serialization

std::shared_ptr<Serializable> CacheableToken::createDeserializable() {
  return std::make_shared<CacheableToken>();
}

void CacheableToken::toData(DataOutput& output) const {
  output.writeInt(static_cast<int32_t>(m_value));
}

void CacheableToken::fromData(DataInput& input) {
  m_value = static_cast<TokenType>(input.readInt32());
}

CacheableToken::CacheableToken() : m_value(CacheableToken::NOT_USED) {}

CacheableToken::CacheableToken(TokenType value) : m_value(value) {}

/**
 * Display this object as 'string', which depend on the implementation in
 * the subclasses
 * The default implementation renders the classname.
 */
std::string CacheableToken::toString() const {
  static const char* ctstrings[] = {
      "CacheableToken::NOT_USED", "CacheableToken::INVALID",
      "CacheableToken::DESTROYED", "CacheableToken::OVERFLOWED",
      "CacheableToken::TOMBSTONE"};

  return ctstrings[m_value];
}

size_t CacheableToken::objectSize() const { return sizeof(m_value); }

std::shared_ptr<CacheableToken>& CacheableToken::invalid() {
  return invalidToken;
}

std::shared_ptr<CacheableToken>& CacheableToken::destroyed() {
  return destroyedToken;
}

std::shared_ptr<CacheableToken>& CacheableToken::overflowed() {
  return overflowedToken;
}

std::shared_ptr<CacheableToken>& CacheableToken::tombstone() {
  return tombstoneToken;
}

bool CacheableToken::isInvalid() { return m_value == INVALID; }

bool CacheableToken::isDestroyed() { return m_value == DESTROYED; }

bool CacheableToken::isOverflowed() { return m_value == OVERFLOWED; }

bool CacheableToken::isTombstone() { return m_value == TOMBSTONE; }

bool CacheableToken::isToken(const std::shared_ptr<Cacheable>& ptr) {
  return (invalidToken == ptr) || (destroyedToken == ptr) ||
         (overflowedToken == ptr) || (tombstoneToken == ptr);
}

bool CacheableToken::isInvalid(const std::shared_ptr<Cacheable>& ptr) {
  return invalidToken == ptr;
}

bool CacheableToken::isDestroyed(const std::shared_ptr<Cacheable>& ptr) {
  return destroyedToken == ptr;
}

bool CacheableToken::isOverflowed(const std::shared_ptr<Cacheable>& ptr) {
  return overflowedToken == ptr;
}

bool CacheableToken::isTombstone(const std::shared_ptr<Cacheable>& ptr) {
  return tombstoneToken == ptr;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
