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
    value += std::to_string(i);
    std::string field = "field";
    field += std::to_string(i);
    values->push_back(CacheableString::create(value.c_str()));
    fieldNames.push_back(CacheableString::create(field.c_str()));
  }
  
  auto ss = StructSetImpl(values, fieldNames);
  
  ASSERT_EQ(1, ss.size());

}

TEST(StructSetTest, MissingFieldIndex) {
  CacheableVectorPtr values = CacheableVector::create();
  std::vector<CacheableStringPtr> fieldNames;
  
  size_t numOfFields = 10;
  
  for (size_t i = 0; i < numOfFields; i++) {
    std::string value = "value";
    value += std::to_string(i);
    std::string field = "field";
    field += std::to_string(i);
    values->push_back(CacheableString::create(value.c_str()));
    fieldNames.push_back(CacheableString::create(field.c_str()));
  }
  
  auto ss = StructSetImpl(values, fieldNames);
  
  ASSERT_THROW(ss.getFieldIndex("test"), std::invalid_argument);
}

TEST(StructSetTest, MissingFieldName) {
  CacheableVectorPtr values = CacheableVector::create();
  std::vector<CacheableStringPtr> fieldNames;
  
  size_t numOfFields = 10;
  
  for (size_t i = 0; i < numOfFields; i++) {
    std::string value = "value";
    value += std::to_string(i);
    std::string field = "field";
    field += std::to_string(i);
    values->push_back(CacheableString::create(value.c_str()));
    fieldNames.push_back(CacheableString::create(field.c_str()));
  }
  
  auto ss = StructSetImpl(values, fieldNames);
  
  ASSERT_THROW(ss.getFieldName(100), std::out_of_range);
}
