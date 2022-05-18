
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

#include <gmock/gmock.h>

#include <gtest/gtest.h>

#include <geode/AuthenticatedView.hpp>
#include <geode/Properties.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheImpl.hpp"
#include "PdxInstanceImpl.hpp"
#include "PdxType.hpp"
#include "statistics/StatisticsFactory.hpp"

namespace {
using apache::geode::client::BooleanArray;
using apache::geode::client::Cache;
using apache::geode::client::Cacheable;
using apache::geode::client::CacheableBoolean;
using apache::geode::client::CacheableByte;
using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableCharacter;
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableDouble;
using apache::geode::client::CacheableDoubleArray;
using apache::geode::client::CacheableFloat;
using apache::geode::client::CacheableFloatArray;
using apache::geode::client::CacheableInt16;
using apache::geode::client::CacheableInt16Array;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableInt32Array;
using apache::geode::client::CacheableInt64;
using apache::geode::client::CacheableInt64Array;
using apache::geode::client::CacheableObjectArray;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableStringArray;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheImpl;
using apache::geode::client::CachePerfStats;
using apache::geode::client::IllegalStateException;
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxInstanceImpl;
using apache::geode::client::PdxType;
using apache::geode::client::Properties;
using apache::geode::statistics::StatisticsFactory;

using ::testing::ContainerEq;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::Not;
using ::testing::NotNull;

TEST(PdxInstanceImplTest, testGetNonExsitantField) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN);
  fields.emplace_back(CacheableBoolean::create(true));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getBooleanField("notAField"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetBoolean) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN);
  fields.emplace_back(CacheableBoolean::create(true));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getBooleanField("boolean"), Eq(true));
}

TEST(PdxInstanceImplTest, testGetBooleanInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolean", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getBooleanField("boolean"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetByte) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int8_t expectedValue = 6;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byte", PdxFieldTypes::BYTE);
  fields.emplace_back(CacheableByte::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);

  ASSERT_THAT(pdxInstanceImpl.getByteField("byte"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetByteInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byte", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getByteField("byte"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetShort) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int16_t expectedValue = 1300;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("short", PdxFieldTypes::SHORT);
  fields.emplace_back(CacheableInt16::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getShortField("short"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetShortInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("short", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getShortField("short"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetInt) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int32_t expectedValue = 100000;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("int", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getIntField("int"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetIntInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("int", PdxFieldTypes::LONG);
  fields.emplace_back(CacheableInt64::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getIntField("int"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetLong) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int64_t expectedValue = 1LL << 34;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("long", PdxFieldTypes::LONG);
  fields.emplace_back(CacheableInt64::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getLongField("long"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetLongInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("long", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getLongField("long"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetFloat) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  float expectedValue = 3.14159265f;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("float", PdxFieldTypes::FLOAT);
  fields.emplace_back(CacheableFloat::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getFloatField("float"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetFloatInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("float", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getFloatField("float"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetDouble) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  double expectedValue = 3.1415926535;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("double", PdxFieldTypes::DOUBLE);
  fields.emplace_back(CacheableDouble::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getDoubleField("double"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetDoubleInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("double", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getDoubleField("double"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetChar) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  char16_t expectedValue = u'λ';
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("char", PdxFieldTypes::CHAR);
  fields.emplace_back(CacheableCharacter::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getCharField("char"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetCharInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("char", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getCharField("char"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetDate) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = CacheableDate::create(std::chrono::system_clock::now());
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("date", PdxFieldTypes::DATE);
  fields.emplace_back(expectedValue);

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  auto date = pdxInstanceImpl.getCacheableDateField("date");
  EXPECT_TRUE(date);

  ASSERT_THAT(*date, Eq(std::ref(*expectedValue)));
}

TEST(PdxInstanceImplTest, testGetDateInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("date", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getCacheableDateField("date"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetString) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::string expectedValue = "This is a test string";
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("string", PdxFieldTypes::STRING);
  fields.emplace_back(CacheableString::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getStringField("string"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetStringInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("string", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getStringField("string"), IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetObject) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue =
      CacheableString::create("This is an string embed as an object");
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("object", PdxFieldTypes::OBJECT);
  fields.emplace_back(expectedValue);

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  auto object = pdxInstanceImpl.getCacheableField("object");
  ASSERT_THAT(object, NotNull());

  auto string = std::dynamic_pointer_cast<CacheableString>(object);
  ASSERT_THAT(string, NotNull());

  ASSERT_THAT(*string, Eq(std::ref(*expectedValue)));
}

TEST(PdxInstanceImplTest, testGetObjectInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("object", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getCacheableField("object"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetByteArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<int8_t>{0, 1, 1, 2, 3, 5};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byteArray", PdxFieldTypes::BYTE_ARRAY);
  fields.emplace_back(CacheableBytes::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getByteArrayField("byteArray"),
              Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetByteArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byteArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getByteArrayField("byteArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetBooleanArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<bool>{true, false, false, true, true, false};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolArray", PdxFieldTypes::BOOLEAN_ARRAY);
  fields.emplace_back(BooleanArray::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getBooleanArrayField("boolArray"),
              Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetBooleanArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getBooleanArrayField("boolArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetShortArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<int16_t>{144, 233, 377, 610, 987};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("shortArray", PdxFieldTypes::SHORT_ARRAY);
  fields.emplace_back(CacheableInt16Array::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getShortArrayField("shortArray"),
              Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetShortArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("shortArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getShortArrayField("shortArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetIntArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue =
      std::vector<int32_t>{75025, 121393, 196418, 317811, 514229};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("intArray", PdxFieldTypes::INT_ARRAY);
  fields.emplace_back(CacheableInt32Array::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getIntArrayField("intArray"), Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetIntArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("intArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getIntArrayField("intArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetLongArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<int64_t>{2971215073, 4807526976, 7778742049,
                                            12586269025, 20365011074};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("longArray", PdxFieldTypes::LONG_ARRAY);
  fields.emplace_back(CacheableInt64Array::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getLongArrayField("longArray"),
              Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetLongArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("longArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getLongArrayField("longArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetFloatArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<float>{0.f, 1.f, 1.f, 2.f, 3.f};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("floatArray", PdxFieldTypes::FLOAT_ARRAY);
  fields.emplace_back(CacheableFloatArray::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getFloatArrayField("floatArray"),
              Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetFloatArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("floatArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getFloatArrayField("floatArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetDoubleArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<double>{5.0, 8.0, 13.0, 21.0, 34.0};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("doubleArray", PdxFieldTypes::DOUBLE_ARRAY);
  fields.emplace_back(CacheableDoubleArray::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getDoubleArrayField("doubleArray"),
              Eq(expectedValue));
}

TEST(PdxInstanceImplTest, testGetDoubleArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("doubleArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getDoubleArrayField("doubleArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetStringArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<std::shared_ptr<CacheableString>>{
      CacheableString::create("Gauss"), CacheableString::create("Taylor"),
      CacheableString::create("Hilbert"), CacheableString::create("Lorenz"),
      CacheableString::create("Boole")};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("stringArray", PdxFieldTypes::STRING_ARRAY);
  fields.emplace_back(CacheableStringArray::create(expectedValue));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  auto &&array = pdxInstanceImpl.getStringArrayField("stringArray");

  ASSERT_THAT(array.size(), Eq(expectedValue.size()));
  for (auto i = 0UL, n = array.size(); i < n; ++i) {
    ASSERT_THAT(array[i], Eq(expectedValue[i]->value()));
  }
}

TEST(PdxInstanceImplTest, testGetStringArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("stringArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getStringArrayField("stringArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetObjectArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = CacheableObjectArray::create();
  expectedValue->emplace_back(CacheableString::create("Gauss"));
  expectedValue->emplace_back(
      CacheableDate::create(std::chrono::system_clock::now()));
  expectedValue->emplace_back(CacheableString::create("Hilbert"));
  expectedValue->emplace_back(CacheableString::create("Lorenz"));
  expectedValue->emplace_back(CacheableString::create("Taylor"));

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("objectArray", PdxFieldTypes::OBJECT_ARRAY);
  fields.emplace_back(expectedValue);

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  auto &&array = pdxInstanceImpl.getCacheableObjectArrayField("objectArray");
  EXPECT_TRUE(array);

  ASSERT_THAT(array->size(), Eq(expectedValue->size()));
  for (auto i = 0UL, n = array->size(); i < n; ++i) {
    ASSERT_THAT((*array)[i], Eq((*expectedValue)[i]));
  }
}

TEST(PdxInstanceImplTest, testGetObjectArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("objectArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getCacheableObjectArrayField("objectArray"),
               IllegalStateException);
}

TEST(PdxInstanceImplTest, testGetFieldEmptyArrayOfByteArrays) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("array", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  fields.emplace_back(CacheableVector::create());

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);

  int32_t length;
  int8_t **result;
  int32_t *elementLength;
  pdxInstanceImpl.getField("array", &result, length, elementLength);

  ASSERT_THAT(length, Eq(0));
  ASSERT_THAT(elementLength, IsNull());
  ASSERT_THAT(result, IsNull());
}

TEST(PdxInstanceImplTest, testGetFieldArrayOfByteArrays) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  std::vector<std::vector<int8_t>> expectedResult{
      {0, 1, 1, 2}, {3, 5, 8, 13, 21}, {34, 55, 89, 55, 34, 21}};

  auto vector = CacheableVector::create();
  for (const auto &row : expectedResult) {
    vector->emplace_back(CacheableBytes::create(row));
  }

  pdxType->addField("array", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  fields.emplace_back(vector);

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);

  int32_t length;
  int8_t **result;
  int32_t *elementLength;
  pdxInstanceImpl.getField("array", &result, length, elementLength);

  ASSERT_THAT(result, NotNull());
  ASSERT_THAT(elementLength, NotNull());
  ASSERT_THAT(length, Eq(expectedResult.size()));

  for (auto i = 0UL, ni = expectedResult.size(); i < ni; ++i) {
    const auto &expectedRow = expectedResult[i];

    auto nj = expectedRow.size();
    ASSERT_THAT(elementLength[i], Eq(nj));

    auto row = result[i];
    ASSERT_THAT(row, NotNull());

    std::vector<int8_t> stlRow{row, row + nj};
    ASSERT_THAT(stlRow, ContainerEq(expectedRow));

    delete[] row;
  }

  delete[] elementLength;
  delete[] result;
}

TEST(PdxInstanceImplTest, testGetFieldArrayOfByteArraysOneNullRow) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  std::vector<std::vector<int8_t>> expectedResult{
      {0, 1, 1, 2}, {3, 5, 8, 13, 21}, {34, 55, 89, 55, 34, 21}};

  auto vector = CacheableVector::create();
  for (const auto &row : expectedResult) {
    vector->emplace_back(CacheableBytes::create(row));
  }

  vector->emplace_back(std::shared_ptr<CacheableBytes>{});

  pdxType->addField("array", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  fields.emplace_back(vector);

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);

  int32_t length;
  int8_t **result;
  int32_t *elementLength;
  pdxInstanceImpl.getField("array", &result, length, elementLength);

  ASSERT_THAT(result, NotNull());
  ASSERT_THAT(elementLength, NotNull());
  ASSERT_THAT(length, Eq(expectedResult.size() + 1));

  for (auto i = 0UL, ni = expectedResult.size(); i < ni; ++i) {
    const auto &expectedRow = expectedResult[i];

    auto nj = expectedRow.size();
    ASSERT_THAT(elementLength[i], Eq(nj));

    auto row = result[i];
    ASSERT_THAT(row, NotNull());

    std::vector<int8_t> stlRow{row, row + nj};
    ASSERT_THAT(stlRow, ContainerEq(expectedRow));

    delete[] row;
  }

  ASSERT_THAT(elementLength[expectedResult.size()], Eq(0));
  ASSERT_THAT(result[expectedResult.size()], IsNull());

  delete[] elementLength;
  delete[] result;
}

TEST(PdxInstanceImplTest, testGetFieldArrayOfByteArraysOneEmptyRow) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  std::vector<std::vector<int8_t>> expectedResult{
      {0, 1, 1, 2}, {3, 5, 8, 13, 21}, {34, 55, 89, 55, 34, 21}};

  auto vector = CacheableVector::create();
  for (const auto &row : expectedResult) {
    vector->emplace_back(CacheableBytes::create(row));
  }

  vector->emplace_back(CacheableBytes::create());

  pdxType->addField("array", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
  fields.emplace_back(vector);

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);

  int32_t length;
  int8_t **result;
  int32_t *elementLength;
  pdxInstanceImpl.getField("array", &result, length, elementLength);

  EXPECT_TRUE(result);
  EXPECT_TRUE(elementLength);
  ASSERT_THAT(length, expectedResult.size() + 1);

  for (auto i = 0UL, ni = expectedResult.size(); i < ni; ++i) {
    const auto &expectedRow = expectedResult[i];

    auto nj = expectedRow.size();
    ASSERT_THAT(elementLength[i], Eq(nj));

    auto row = result[i];
    ASSERT_THAT(row, NotNull());

    std::vector<int8_t> stlRow{row, row + nj};
    ASSERT_THAT(stlRow, ContainerEq(expectedRow));

    delete[] row;
  }

  ASSERT_THAT(elementLength[expectedResult.size()], Eq(0));
  ASSERT_THAT(result[expectedResult.size()], IsNull());

  delete[] elementLength;
  delete[] result;
}

TEST(PdxInstanceImplTest, testHasField) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("field", PdxFieldTypes::BOOLEAN);
  fields.emplace_back(CacheableBoolean::create(true));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.hasField("field"), IsTrue());
}

TEST(PdxInstanceImplTest, testDoesntHaveField) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("field", PdxFieldTypes::BOOLEAN);
  fields.emplace_back(CacheableBoolean::create(true));

  PdxInstanceImpl pdxInstanceImpl(fields, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.hasField("notAField"), IsFalse());
}

TEST(PdxInstanceImplTest, testIdentityFields) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto pdxType = std::make_shared<PdxType>("Test", false);
  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN)->setIdentity(true);
  PdxInstanceImpl pdxInstanceImpl(std::vector<std::shared_ptr<Cacheable>>{},
                                  pdxType, cacheImpl);

  ASSERT_THAT(pdxInstanceImpl.isIdentityField("boolean"), IsTrue());
}

TEST(PdxInstanceImplTest, testFieldNames) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto pdxType = std::make_shared<PdxType>("Test", false);
  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN);
  pdxType->addField("byte", PdxFieldTypes::BYTE);
  pdxType->addField("short", PdxFieldTypes::SHORT);
  pdxType->addField("int", PdxFieldTypes::INT);
  pdxType->addField("long", PdxFieldTypes::LONG);
  pdxType->addField("float", PdxFieldTypes::FLOAT);
  pdxType->addField("double", PdxFieldTypes::DOUBLE);
  PdxInstanceImpl pdxInstanceImpl(std::vector<std::shared_ptr<Cacheable>>{},
                                  pdxType, cacheImpl);

  auto &&fieldNames = pdxInstanceImpl.getFieldNames();
  ASSERT_THAT(fieldNames, NotNull());
  ASSERT_THAT(fieldNames->length(), Eq(7));

  std::vector<std::string> fields;
  fields.reserve(fieldNames->length());

  for (const auto &field : fieldNames->value()) {
    fields.emplace_back(field->toString());
  }

  std::vector<std::string> expectedFields{"boolean", "byte",  "short", "int",
                                          "long",    "float", "double"};
  ASSERT_THAT(fields, ContainerEq(expectedFields));
}

TEST(PdxInstanceImplTest, testClassName) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto pdxType = std::make_shared<PdxType>("Test", false);
  PdxInstanceImpl pdxInstanceImpl(std::vector<std::shared_ptr<Cacheable>>{},
                                  pdxType, cacheImpl);

  ASSERT_THAT(pdxInstanceImpl.getClassName(), Eq("Test"));
}

TEST(PdxInstanceImplTest, testFieldType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto pdxType = std::make_shared<PdxType>("Test", false);
  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN);
  pdxType->addField("byte", PdxFieldTypes::BYTE);
  pdxType->addField("short", PdxFieldTypes::SHORT);
  pdxType->addField("int", PdxFieldTypes::INT);
  pdxType->addField("long", PdxFieldTypes::LONG);
  pdxType->addField("float", PdxFieldTypes::FLOAT);
  pdxType->addField("double", PdxFieldTypes::DOUBLE);
  PdxInstanceImpl pdxInstanceImpl(std::vector<std::shared_ptr<Cacheable>>{},
                                  pdxType, cacheImpl);

  for (const auto &field : pdxType->getFields()) {
    ASSERT_THAT(pdxInstanceImpl.getFieldType(field->getName()),
                Eq(field->getType()));
  }
}

TEST(PdxInstanceImplTest, testEquals) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::shared_ptr<PdxInstanceImpl> instance1;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-1", PdxFieldTypes::STRING)->setIdentity(true);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("identity-2", PdxFieldTypes::DATE)->setIdentity(true);
    fields.emplace_back(CacheableDate::create(10000000));
    instance1 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  std::shared_ptr<PdxInstanceImpl> instance2;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-2", PdxFieldTypes::DATE)->setIdentity(true);
    fields.emplace_back(CacheableDate::create(10000000));

    pdxType->addField("identity-1", PdxFieldTypes::STRING)->setIdentity(true);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::LONG);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("field-2", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(2491));

    instance2 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  ASSERT_THAT(*instance1, Eq(std::ref(*instance2)));
}

TEST(PdxInstanceImplTest, testNotEquals) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::shared_ptr<PdxInstanceImpl> instance1;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-1", PdxFieldTypes::STRING)->setIdentity(true);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("identity-2", PdxFieldTypes::DATE);
    fields.emplace_back(CacheableDate::create(10000000));
    instance1 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  std::shared_ptr<PdxInstanceImpl> instance2;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-2", PdxFieldTypes::DATE)->setIdentity(true);
    fields.emplace_back(CacheableDate::create(10000000));

    pdxType->addField("identity-1", PdxFieldTypes::STRING)->setIdentity(true);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::LONG);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("field-2", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(2491));

    instance2 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  ASSERT_THAT(*instance1, Not(Eq(std::ref(*instance2))));
}

TEST(PdxInstanceImplTest, testEqualsIdentityNotMarked) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::shared_ptr<PdxInstanceImpl> instance1;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-1", PdxFieldTypes::STRING);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("identity-2", PdxFieldTypes::DATE);
    fields.emplace_back(CacheableDate::create(10000000));
    instance1 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  std::shared_ptr<PdxInstanceImpl> instance2;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-2", PdxFieldTypes::DATE);
    fields.emplace_back(CacheableDate::create(10000000));

    pdxType->addField("identity-1", PdxFieldTypes::STRING);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(102));

    instance2 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  ASSERT_THAT(*instance1, Eq(std::ref(*instance2)));
}

TEST(PdxInstanceImplTest, testNotEqualsIdentityNotMarked) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::shared_ptr<PdxInstanceImpl> instance1;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-1", PdxFieldTypes::STRING);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("identity-2", PdxFieldTypes::DATE);
    fields.emplace_back(CacheableDate::create(10000000));
    instance1 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  std::shared_ptr<PdxInstanceImpl> instance2;
  {
    std::vector<std::shared_ptr<Cacheable>> fields;
    auto pdxType = std::make_shared<PdxType>("Test", false);

    pdxType->addField("identity-2", PdxFieldTypes::DATE);
    fields.emplace_back(CacheableDate::create(10000000));

    pdxType->addField("identity-1", PdxFieldTypes::STRING);
    fields.emplace_back(CacheableString::create("uuid"));

    pdxType->addField("field-1", PdxFieldTypes::INT);
    fields.emplace_back(CacheableInt32::create(102));

    pdxType->addField("field-2", PdxFieldTypes::LONG);
    fields.emplace_back(CacheableInt32::create(2491));

    instance2 = std::make_shared<PdxInstanceImpl>(fields, pdxType, cacheImpl);
  }

  ASSERT_THAT(*instance1, Not(Eq(std::ref(*instance2))));
}

}  // namespace
