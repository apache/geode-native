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

#include <geode/ExpirationAction.hpp>

namespace apache {
namespace geode {
namespace client {

const std::string ExpirationAction::names[] = {"INVALIDATE", "LOCAL_INVALIDATE",
                                               "DESTROY", "LOCAL_DESTROY",
                                               "INVALID_ACTION"};

ExpirationAction::Action ExpirationAction::fromName(const std::string& name) {
  for (int i = INVALIDATE; i <= INVALID_ACTION; i++) {
    if (names[i] == name) {
      return static_cast<Action>(i);
    }
  }
  return INVALID_ACTION;
}

const std::string& ExpirationAction::fromOrdinal(const int ordinal) {
  if (INVALIDATE <= ordinal && ordinal <= LOCAL_DESTROY) {
    return names[ordinal];
  }
  return names[INVALID_ACTION];
}

}  // namespace client
}  // namespace geode
}  // namespace apache
