
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
#include <geode/WritablePdxInstance.hpp>

#include "CacheImpl.hpp"
#include "PdxType.hpp"
#include "WritablePdxInstanceImpl.hpp"
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
using apache::geode::client::PdxType;
using apache::geode::client::Properties;
using apache::geode::client::WritablePdxInstanceImpl;
using apache::geode::statistics::StatisticsFactory;

using ::testing::ContainerEq;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::Not;
using ::testing::NotNull;
using ::testing::SizeIs;

TEST(WritablePdxInstanceImplTest, testSetNonExsitantField) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN);
  fields.emplace_back(CacheableBoolean::create(true));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("notAField", true),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetBoolean) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  bool expectedValue = true;
  pdxType->addField("boolean", PdxFieldTypes::BOOLEAN);
  fields.emplace_back(CacheableBoolean::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getBooleanField("boolean"), Eq(expectedValue));

  expectedValue = false;
  pdxInstanceImpl.setField("boolean", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getBooleanField("boolean"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetBooleanInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolean", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("boolean", true),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetByte) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int8_t expectedValue = 6;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byte", PdxFieldTypes::BYTE);
  fields.emplace_back(CacheableByte::create(expectedValue));
  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getByteField("byte"), Eq(expectedValue));

  expectedValue = 5;
  pdxInstanceImpl.setField("byte", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getByteField("byte"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetByteInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byte", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("byte", static_cast<int8_t>(51)),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetShort) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int16_t expectedValue = 1300;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("short", PdxFieldTypes::SHORT);
  fields.emplace_back(CacheableInt16::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getShortField("short"), Eq(expectedValue));

  expectedValue = 1400;
  pdxInstanceImpl.setField("short", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getShortField("short"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetShortInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("short", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("short", static_cast<int16_t>(151)),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetInt) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int32_t expectedValue = 100000;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("int", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getIntField("int"), Eq(expectedValue));

  expectedValue = 20000;
  pdxInstanceImpl.setField("int", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getIntField("int"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetIntInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("int", PdxFieldTypes::LONG);
  fields.emplace_back(CacheableInt64::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("int", static_cast<int32_t>(64371)),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetLong) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  int64_t expectedValue = 1LL << 34;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("long", PdxFieldTypes::LONG);
  fields.emplace_back(CacheableInt64::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getLongField("long"), Eq(expectedValue));

  expectedValue = 1LL << 35;
  pdxInstanceImpl.setField("long", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getLongField("long"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetLongInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("long", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(
      pdxInstanceImpl.setField("long", static_cast<int64_t>(4362486975)),
      IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetFloat) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  float expectedValue = 3.14159265f;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("float", PdxFieldTypes::FLOAT);
  fields.emplace_back(CacheableFloat::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getFloatField("float"), Eq(expectedValue));

  expectedValue = 6.283f;
  pdxInstanceImpl.setField("float", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getFloatField("float"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetFloatInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("float", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("float", 3.1f), IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetDouble) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  double expectedValue = 3.1415926535;
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("double", PdxFieldTypes::DOUBLE);
  fields.emplace_back(CacheableDouble::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getDoubleField("double"), Eq(expectedValue));

  expectedValue = 6.283;
  pdxInstanceImpl.setField("double", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getDoubleField("double"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetDoubleInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("double", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.getDoubleField("double"), IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetChar) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  char16_t expectedValue = u'λ';
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("char", PdxFieldTypes::CHAR);
  fields.emplace_back(CacheableCharacter::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getCharField("char"), Eq(expectedValue));

  expectedValue = u'β';
  pdxInstanceImpl.setField("char", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getCharField("char"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetCharInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("char", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("char", u'λ'), IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetDate) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = CacheableDate::create(std::chrono::system_clock::now());
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("date", PdxFieldTypes::DATE);
  fields.emplace_back(expectedValue);

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  auto date = pdxInstanceImpl.getCacheableDateField("date");
  ASSERT_THAT(date, NotNull());
  ASSERT_THAT(*date, Eq(std::ref(*expectedValue)));

  expectedValue = CacheableDate::create(std::chrono::system_clock::now() +
                                        std::chrono::seconds{2});
  pdxInstanceImpl.setField("date", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getCacheableDateField("date"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetDateInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("date", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(
      pdxInstanceImpl.setField("date", CacheableDate::create(10000000000)),
      IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetString) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::string expectedValue = "This is a test string";
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("string", PdxFieldTypes::STRING);
  fields.emplace_back(CacheableString::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getStringField("string"), Eq(expectedValue));

  expectedValue = "This is some other test string";
  pdxInstanceImpl.setField("string", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getStringField("string"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetStringInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("string", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField("string", "Some other string"),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetObject) {
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

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  auto object = pdxInstanceImpl.getCacheableField("object");
  ASSERT_THAT(object, NotNull());

  auto string = std::dynamic_pointer_cast<CacheableString>(object);
  ASSERT_THAT(string, NotNull());

  ASSERT_THAT(*string, Eq(std::ref(*expectedValue)));

  expectedValue =
      CacheableString::create("This is some other string embed as an object");
  pdxInstanceImpl.setField("object", expectedValue);
  object = pdxInstanceImpl.getCacheableField("object");
  ASSERT_THAT(object, NotNull());

  string = std::dynamic_pointer_cast<CacheableString>(object);
  ASSERT_THAT(string, NotNull());

  ASSERT_THAT(*string, Eq(std::ref(*expectedValue)));
}

TEST(WritablePdxInstanceImplTest, testSetObjectInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("object", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField(
                   "object", CacheableString::create("This is some object")),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetByteArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<int8_t>{0, 1, 1, 2, 3, 5};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byteArray", PdxFieldTypes::BYTE_ARRAY);
  fields.emplace_back(CacheableBytes::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getByteArrayField("byteArray"),
              Eq(expectedValue));

  expectedValue = std::vector<int8_t>{8, 13, 21, 34, 55, 89};
  pdxInstanceImpl.setField("byteArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getByteArrayField("byteArray"),
              Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetByteArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("byteArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField(
                   "byteArray", std::vector<int8_t>{1, 1, 2, 3, 5, 8, 13}),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetBooleanArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<bool>{true, false, false, true, true, false};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolArray", PdxFieldTypes::BOOLEAN_ARRAY);
  fields.emplace_back(BooleanArray::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getBooleanArrayField("boolArray"),
              Eq(expectedValue));

  expectedValue = std::vector<bool>{false, true, true, false, false, true};
  pdxInstanceImpl.setField("boolArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getBooleanArrayField("boolArray"),
              Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetBooleanArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("boolArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField(
                   "boolArray",
                   std::vector<bool>{false, true, false, true, true, false}),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetShortArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<int16_t>{144, 233, 377, 610, 987};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("shortArray", PdxFieldTypes::SHORT_ARRAY);
  fields.emplace_back(CacheableInt16Array::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getShortArrayField("shortArray"),
              Eq(expectedValue));

  expectedValue = std::vector<int16_t>{987, 610, 377, 233, 144};
  pdxInstanceImpl.setField("shortArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getShortArrayField("shortArray"),
              Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetShortArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("shortArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField(
                   "shortArray", std::vector<int16_t>{1, 1, 2, 3, 5, 8, 13}),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetIntArray) {
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

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getIntArrayField("intArray"), Eq(expectedValue));

  expectedValue = std::vector<int32_t>{514229, 317811, 196418, 121393, 75025};
  pdxInstanceImpl.setField("intArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getIntArrayField("intArray"), Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetIntArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("intArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField(
                   "intArray", std::vector<int32_t>{1, 1, 2, 3, 5, 8, 13}),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetLongArray) {
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

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getLongArrayField("longArray"),
              Eq(expectedValue));

  expectedValue = std::vector<int64_t>{20365011074, 12586269025, 7778742049,
                                       4807526976, 2971215073};
  pdxInstanceImpl.setField("longArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getLongArrayField("longArray"),
              Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetLongArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("longArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(pdxInstanceImpl.setField(
                   "longArray", std::vector<int64_t>{1, 1, 2, 3, 5, 8, 13}),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetFloatArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<float>{0.f, 1.f, 1.f, 2.f, 3.f};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("floatArray", PdxFieldTypes::FLOAT_ARRAY);
  fields.emplace_back(CacheableFloatArray::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getFloatArrayField("floatArray"),
              Eq(expectedValue));

  expectedValue = std::vector<float>{4.f, 5.f, 6.f, 7.f, 8.f};
  pdxInstanceImpl.setField("floatArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getFloatArrayField("floatArray"),
              Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetFloatArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("floatArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(
      pdxInstanceImpl.setField(
          "floatArray", std::vector<float>{1.f, 1.f, 2.f, 3.f, 5.f, 8.f, 13.f}),
      IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetDoubleArray) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  auto expectedValue = std::vector<double>{5.0, 8.0, 13.0, 21.0, 34.0};
  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("doubleArray", PdxFieldTypes::DOUBLE_ARRAY);
  fields.emplace_back(CacheableDoubleArray::create(expectedValue));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  ASSERT_THAT(pdxInstanceImpl.getDoubleArrayField("doubleArray"),
              Eq(expectedValue));

  expectedValue = std::vector<double>{21., 34, 55., 89.};
  pdxInstanceImpl.setField("doubleArray", expectedValue);
  ASSERT_THAT(pdxInstanceImpl.getDoubleArrayField("doubleArray"),
              Eq(expectedValue));
}

TEST(WritablePdxInstanceImplTest, testSetDoubleArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("doubleArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  EXPECT_THROW(
      pdxInstanceImpl.setField(
          "doubleArray", std::vector<double>{1., 1., 2., 3., 5., 8., 13.}),
      IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetStringArray) {
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

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  auto &&array = pdxInstanceImpl.getStringArrayField("stringArray");

  ASSERT_THAT(array, SizeIs(expectedValue.size()));
  for (auto i = 0UL, n = array.size(); i < n; ++i) {
    ASSERT_THAT(array[i], Eq(expectedValue[i]->value()));
  }

  std::vector<std::string> expectedStrValue{"Boole", "Lorenz", "Hilbert",
                                            "Taylor", "Gauss"};
  pdxInstanceImpl.setField("stringArray", expectedStrValue.data(),
                           expectedStrValue.size());
  ASSERT_THAT(pdxInstanceImpl.getStringArrayField("stringArray"),
              ContainerEq(expectedStrValue));
}

TEST(WritablePdxInstanceImplTest, testSetStringArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("stringArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  std::vector<std::string> expectedArray{"Some string 1", "Some string 2",
                                         "Some string 3", "Some string 4"};
  EXPECT_THROW(pdxInstanceImpl.setField("stringArray", expectedArray.data(),
                                        expectedArray.size()),
               IllegalStateException);
}

TEST(WritablePdxInstanceImplTest, testSetObjectArray) {
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

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  auto &&array = pdxInstanceImpl.getCacheableObjectArrayField("objectArray");
  ASSERT_THAT(array, NotNull());

  ASSERT_THAT(*array, SizeIs(expectedValue->size()));
  for (auto i = 0UL, n = array->size(); i < n; ++i) {
    ASSERT_THAT((*array)[i], Eq((*expectedValue)[i]));
  }

  expectedValue = CacheableObjectArray::create();
  expectedValue->emplace_back(CacheableString::create("Taylor"));
  expectedValue->emplace_back(CacheableString::create("Lorenz"));
  expectedValue->emplace_back(
      CacheableDate::create(std::chrono::system_clock::now()));
  expectedValue->emplace_back(CacheableString::create("Hilbert"));
  expectedValue->emplace_back(CacheableString::create("Gauss"));

  pdxInstanceImpl.setField("objectArray", expectedValue);
  array = pdxInstanceImpl.getCacheableObjectArrayField("objectArray");
  ASSERT_THAT(array, NotNull());

  ASSERT_THAT(*array, SizeIs(expectedValue->size()));
  for (auto i = 0UL, n = array->size(); i < n; ++i) {
    ASSERT_THAT((*array)[i], Eq((*expectedValue)[i]));
  }
}

TEST(WritablePdxInstanceImplTest, testSetObjectArrayInvalidType) {
  auto properties = std::make_shared<Properties>();
  properties->insert("log-level", "none");
  auto cache = CacheFactory{}.set("log-level", "none").create();
  CacheImpl cacheImpl(&cache, properties, true, false, nullptr);

  std::vector<std::shared_ptr<Cacheable>> fields;
  auto pdxType = std::make_shared<PdxType>("Test", false);

  pdxType->addField("objectArray", PdxFieldTypes::INT);
  fields.emplace_back(CacheableInt32::create(5));

  WritablePdxInstanceImpl pdxInstanceImpl(fields, {}, pdxType, cacheImpl);
  auto &&expectedArray = CacheableObjectArray::create();
  expectedArray->emplace_back(CacheableString::create("Some object 1"));
  expectedArray->emplace_back(CacheableString::create("Some object 2"));
  expectedArray->emplace_back(CacheableString::create("Some object 3"));

  EXPECT_THROW(pdxInstanceImpl.setField("objectArray", expectedArray),
               IllegalStateException);
}

}  // namespace
