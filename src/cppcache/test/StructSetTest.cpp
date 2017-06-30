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

#include <gtest/gtest.h>
#include <stdexcept>

#include <StructSetImpl.hpp>

using namespace apache::geode::client;

TEST(StructSetTest, Basic) {
  CacheableVectorPtr values = CacheableVector::create();
  std::vector<CacheableStringPtr> fieldNames;
  
  size_t numOfFields = 10;
  
  for (size_t i = 0; i < numOfFields; i++) {
    std::string value = "value";
    value += i;
    std::string field = "field";
    field += i;
    values->push_back(CacheableString::create(value.c_str()));
    fieldNames.push_back(CacheableString::create(field.c_str()));
  }
  
  StructSet* ss = new StructSetImpl(values, fieldNames);
  
  ss->size();

}
TEST(StructSetTest, MissingFieldIndex) {
  CacheableVectorPtr values = CacheableVector::create();
  std::vector<CacheableStringPtr> fieldNames;
  
  size_t numOfFields = 10;
  
  for (size_t i = 0; i < numOfFields; i++) {
    std::string value = "value";
    value += i;
    std::string field = "field";
    field += i;
    values->push_back(CacheableString::create(value.c_str()));
    fieldNames.push_back(CacheableString::create(field.c_str()));
  }
  
  StructSet* ss = new StructSetImpl(values, fieldNames);
  
  try {
    ss->getFieldIndex("test");
  } catch (const std::invalid_argument& e) {
    printf("Caught expected exception: %s", e.what());
  }

}
TEST(StructSetTest, MissingFieldName) {
  CacheableVectorPtr values = CacheableVector::create();
  std::vector<CacheableStringPtr> fieldNames;
  
  size_t numOfFields = 10;
  
  for (size_t i = 0; i < numOfFields; i++) {
    std::string value = "value";
    value += i;
    std::string field = "field";
    field += i;
    values->push_back(CacheableString::create(value.c_str()));
    fieldNames.push_back(CacheableString::create(field.c_str()));
  }
  
  StructSet* ss = new StructSetImpl(values, fieldNames);
  
  try {
    ss->getFieldName(100);
  } catch (const std::out_of_range& e) {
    printf("Caught expected exception: %s", e.what());
  }
}
