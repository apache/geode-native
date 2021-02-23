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
#include <geode/EntryEvent.hpp>
#include <geode/StringPrefixPartitionResolver.hpp>

namespace {
const char* const DEFAULT_DELIMITER = "|";
}

namespace apache {
namespace geode {
namespace client {

StringPrefixPartitionResolver::StringPrefixPartitionResolver()
    : StringPrefixPartitionResolver(DEFAULT_DELIMITER) {}

StringPrefixPartitionResolver::StringPrefixPartitionResolver(
    std::string delimiter)
    : PartitionResolver(), delimiter_(std::move(delimiter)) {}

const std::string& StringPrefixPartitionResolver::getName() {
  static std::string name = "StringPrefixPartitionResolver";
  return name;
}

std::shared_ptr<CacheableKey> StringPrefixPartitionResolver::getRoutingObject(
    const EntryEvent& event) {
  auto&& key = event.getKey();
  if (key == nullptr) {
    return {};
  }

  auto key_str = key->toString();
  auto pos = key_str.find(delimiter_);

  if (pos == std::string::npos) {
    throw IllegalArgumentException("The key \"" + key_str +
                                   "\" does not contains the \"" + delimiter_ +
                                   "\" delimiter.");
  }

  return CacheableKey::create(key_str.substr(0, pos));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
