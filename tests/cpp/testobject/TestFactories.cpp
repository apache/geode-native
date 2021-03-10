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

#include <geode/PartitionResolver.hpp>

#include "testobject_export.h"

using apache::geode::client::CacheableKey;
using apache::geode::client::EntryEvent;
using apache::geode::client::PartitionResolver;

namespace {
class TestLibPartitionResolver : public PartitionResolver {
 public:
  const std::string& getName() override {
    static std::string name = "TestLibPartitionResolver";
    return name;
  }

  std::shared_ptr<CacheableKey> getRoutingObject(const EntryEvent&) override {
    return {};
  }
};
}  // namespace

extern "C" TESTOBJECT_EXPORT PartitionResolver*
createTestLibPartitionResolver() {
  return new TestLibPartitionResolver{};
}
