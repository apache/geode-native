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
#include <geode/internal/geode_globals.hpp>

#include "CacheableToken.hpp"

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/CacheableString.hpp>
#include "GeodeTypeIdsImpl.hpp"

using namespace apache::geode::client;

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

int8_t CacheableToken::getInternalId() const {
  return static_cast<int8_t>(GeodeTypeIdsImpl::CacheableToken);
}

//------ ctor

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
