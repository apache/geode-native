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
#include <geode/CacheableUndefined.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>
#include <geode/GeodeTypeIds.hpp>

#include "GeodeTypeIdsImpl.hpp"

namespace apache {
namespace geode {
namespace client {

void CacheableUndefined::toData(DataOutput& output) const {}

void CacheableUndefined::fromData(DataInput& input) {}

int32_t CacheableUndefined::classId() const { return 0; }

int8_t CacheableUndefined::typeId() const {
  return GeodeTypeIds::CacheableUndefined;
}

int8_t CacheableUndefined::DSFID() const {
  return GeodeTypeIdsImpl::FixedIDByte;
}

size_t CacheableUndefined::objectSize() const { return 0; }
}  // namespace client
}  // namespace geode
}  // namespace apache
