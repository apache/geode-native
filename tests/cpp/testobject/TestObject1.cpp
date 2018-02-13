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
/*
 * TestObject1.cpp
 *
 *  Created on: Jul 15, 2009
 *      Author: abhaware
 */

#include "TestObject1.hpp"
using namespace testobject;

TestObject1::TestObject1()
    : name(nullptr), arr(CacheableBytes::create(std::vector<int8_t>(4 * 1024))), identifier(1) {}

TestObject1::TestObject1(std::string& str, int32_t id) {
  name = CacheableString::create(str.c_str());
  identifier = id;
  int8_t* bytes;
  _GEODE_NEW(bytes, int8_t[1024 * 4]);
  bytes[0] = 'A';
  for (int i = 1; i <= 1024 * 2; i = i * 2) {
    memcpy(bytes + i, bytes, i);
  }
  arr = CacheableBytes::create(std::vector<int8_t>(bytes, bytes + 1024 * 4));
  delete bytes;
}

TestObject1::TestObject1(TestObject1& rhs) {
  name = rhs.name == nullptr
             ? nullptr
             : CacheableString::create(rhs.name->value().c_str());
  identifier = rhs.identifier;
  arr = CacheableBytes::create(rhs.arr->value());
}

void TestObject1::toData(DataOutput& output) const {
  output.writeBytes(arr->value().data(), arr->length());
  output.writeObject(name);
  output.writeInt(identifier);
}

void TestObject1::fromData(DataInput& input) {
  int8_t* bytes;
  int32_t len;
  input.readBytes(&bytes, &len);
  arr = CacheableBytes::create(std::vector<int8_t>(bytes, bytes + len));
  delete bytes;
  name = std::static_pointer_cast<CacheableString>(input.readObject());
  identifier = input.readInt32();
}

Serializable* TestObject1::create() { return new TestObject1(); }
