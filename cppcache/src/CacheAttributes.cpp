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

#include <string>
#include <cstdlib>

#include <geode/GeodeTypeIds.hpp>
#include <geode/CacheAttributes.hpp>

#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

CacheAttributes::CacheAttributes()
    : m_redundancyLevel(0), m_endpoints(nullptr), m_cacheMode(false) {}

int CacheAttributes::getRedundancyLevel() { return m_redundancyLevel; }

const std::string& CacheAttributes::getEndpoints() { return m_endpoints; }

/** Return true if all the attributes are equal to those of other. */
bool CacheAttributes::operator==(const CacheAttributes& other) const {
  if (m_redundancyLevel != other.m_redundancyLevel) return false;
  if (m_endpoints != other.m_endpoints) return false;

  return true;
}

/** Return true if any of the attributes are not equal to those of other. */
bool CacheAttributes::operator!=(const CacheAttributes& other) const {
  return !(*this == other);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
