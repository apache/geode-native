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

#include <geode/CacheableKey.hpp>

namespace apache {
namespace geode {
namespace client {

std::size_t CacheableKey::hash::operator()(const CacheableKey& s) const {
  return s.hashcode();
}

std::size_t CacheableKey::hash::operator()(const CacheableKey*& s) const {
  return s->hashcode();
}

std::size_t CacheableKey::hash::operator()(
    const std::shared_ptr<CacheableKey>& s) const {
  return s->hashcode();
}

bool CacheableKey::equal_to::operator()(const CacheableKey& lhs,
                                        const CacheableKey& rhs) const {
  return lhs == rhs;
}

bool CacheableKey::equal_to::operator()(const CacheableKey*& lhs,
                                        const CacheableKey*& rhs) const {
  return (*lhs) == (*rhs);
}

bool CacheableKey::equal_to::operator()(
    const std::shared_ptr<CacheableKey>& lhs,
    const std::shared_ptr<CacheableKey>& rhs) const {
  return (*lhs) == (*rhs);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
