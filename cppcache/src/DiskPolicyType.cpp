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

#include <geode/DiskPolicyType.hpp>
#include "ace/OS.h"

namespace apache {
namespace geode {
namespace client {

const std::string DiskPolicyType::names[] = {"none", "overflows", "persist"};

DiskPolicyType::PolicyType DiskPolicyType::fromName(const std::string& name) {
  for (int i = NONE; i <= PERSIST; i++) {
    if (names[i] == name) {
      return static_cast<PolicyType>(i);
    }
  }
  return DiskPolicyType::NONE;
}

const std::string& DiskPolicyType::fromOrdinal(const uint8_t ordinal) {
  if (NONE <= ordinal && ordinal <= PERSIST) {
    return names[ordinal];
  }

  return names[NONE];
}

}  // namespace client
}  // namespace geode
}  // namespace apache
