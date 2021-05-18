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

#ifndef GEODE_STRINGPREFIXPARTITIONRESOLVER_H_
#define GEODE_STRINGPREFIXPARTITIONRESOLVER_H_

#include "PartitionResolver.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheableKey;
class EntryEvent;

/**
 * This class implements a partition resolver which routing object is
 * the prefix of a given key.
 * Delimiter is set by default to '|', still can be changed.
 * @note If prefix is not found in the key an IllegalArgumentException is thrown
 *
 * Examples:
 *  - Given key "key-1|timestamp", with delimiter '|', the routing object would
 *    be "key-1"
 *  - Given "key-1#DELIM#timestamp", with delimiter '|', then an exception is
 *    thrown.
 */
class APACHE_GEODE_EXPORT StringPrefixPartitionResolver
    : public PartitionResolver {
 public:
  StringPrefixPartitionResolver();
  explicit StringPrefixPartitionResolver(std::string delimiter);

  StringPrefixPartitionResolver(const StringPrefixPartitionResolver&) = delete;

  ~StringPrefixPartitionResolver() override = default;

  void operator=(const StringPrefixPartitionResolver&) = delete;

  const std::string& getName() override;

  std::shared_ptr<CacheableKey> getRoutingObject(
      const EntryEvent& opDetails) override;

 protected:
  std::string delimiter_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STRINGPREFIXPARTITIONRESOLVER_H_
