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

#ifndef GEODE_TESTOBJECT_TESTOBJECT1_H_
#define GEODE_TESTOBJECT_TESTOBJECT1_H_

#include <geode/CacheableBuiltins.hpp>
#include <string>

#ifdef _WIN32
#ifdef BUILD_TESTOBJECT
#define TESTOBJECT_EXPORT _GEODE_LIBEXP
#else
#define TESTOBJECT_EXPORT _GEODE_LIBIMP
#endif
#else
#define TESTOBJECT_EXPORT
#endif

using namespace apache::geode::client;
namespace testobject {

class TESTOBJECT_EXPORT TestObject1 : public Cacheable {
 private:
  std::shared_ptr<CacheableString> name;
  std::shared_ptr<CacheableBytes> arr;
  int32_t identifier;

 public:
  TestObject1();
  TestObject1(int32_t id)
      : name(nullptr), arr(CacheableBytes::create(std::vector<int8_t>(4 * 1024))), identifier(id) {}
  TestObject1(std::string& str, int32_t id);
  TestObject1(TestObject1& rhs);
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;

  int32_t getIdentifier() { return identifier; }

  int32_t classId() const override { return 31; }

  size_t objectSize() const override { return 0; }

  static Serializable* create();
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_TESTOBJECT1_H_
