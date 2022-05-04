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

#include "ByteArrayFixture.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"
#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"

namespace {
using apache::geode::client::ByteArray;
using apache::geode::client::DataInputInternal;
using apache::geode::client::DataOutputInternal;
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxType;
using apache::geode::client::PdxTypeRegistry;

const std::string gemfireJsonClassName = "__GEMFIRE_JSON";

class PdxTypeTest : public ::testing::Test, public ByteArrayFixture {};

TEST_F(PdxTypeTest, testTwoObjectsWithSameClassnameAndSameFieldsAreEquals) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar0", PdxFieldTypes::STRING);
  pdxType1.addField("bar1", PdxFieldTypes::STRING);

  PdxType pdxType2{gemfireJsonClassName};
  pdxType2.addField("bar0", PdxFieldTypes::STRING);
  pdxType2.addField("bar1", PdxFieldTypes::STRING);

  EXPECT_TRUE(pdxType1 == pdxType2);
}

TEST_F(
    PdxTypeTest,
    testTwoObjectsWithSameClassnameAndSameFieldsInDifferentOrderAreDifferent) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar1", PdxFieldTypes::STRING);
  pdxType1.addField("bar0", PdxFieldTypes::STRING);

  PdxType pdxType2{gemfireJsonClassName};
  pdxType2.addField("bar0", PdxFieldTypes::STRING);
  pdxType2.addField("bar1", PdxFieldTypes::STRING);

  EXPECT_FALSE(pdxType1 == pdxType2);
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithDifferentClassnameButSameFieldsAreNotEquals) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar0", PdxFieldTypes::STRING);
  pdxType1.addField("bar1", PdxFieldTypes::STRING);

  PdxType pdxType2{"otherClassName"};
  pdxType2.addField("bar0", PdxFieldTypes::STRING);
  pdxType2.addField("bar1", PdxFieldTypes::STRING);

  EXPECT_FALSE(pdxType1 == pdxType2);
}

TEST_F(PdxTypeTest, testTwoObjectsWithSameFieldsHaveTheSameHash) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar0", PdxFieldTypes::STRING);
  pdxType1.addField("bar1", PdxFieldTypes::STRING);
  pdxType1.addField("bar2", PdxFieldTypes::STRING);

  PdxType pdxType2{gemfireJsonClassName};
  pdxType2.addField("bar0", PdxFieldTypes::STRING);
  pdxType2.addField("bar1", PdxFieldTypes::STRING);
  pdxType2.addField("bar2", PdxFieldTypes::STRING);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;
  EXPECT_EQ(type1Hash(pdxType1), type2Hash(pdxType2));
}

TEST_F(PdxTypeTest, testTwoObjectsWithDifferentFieldsHaveDifferentHash) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar0", PdxFieldTypes::STRING);
  pdxType1.addField("bar1", PdxFieldTypes::STRING);

  PdxType pdxType2{gemfireJsonClassName};
  pdxType2.addField("bar2", PdxFieldTypes::STRING);
  pdxType2.addField("bar3", PdxFieldTypes::STRING);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;

  EXPECT_NE(type1Hash(pdxType1), type2Hash(pdxType2));
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithSameFieldsInDifferentOrderHaveTheSameHash) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar0", PdxFieldTypes::STRING);
  pdxType1.addField("bar1", PdxFieldTypes::STRING);
  pdxType1.addField("bar2", PdxFieldTypes::STRING);

  PdxType pdxType2{gemfireJsonClassName};
  pdxType2.addField("bar1", PdxFieldTypes::STRING);
  pdxType2.addField("bar2", PdxFieldTypes::STRING);
  pdxType2.addField("bar0", PdxFieldTypes::STRING);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;

  EXPECT_EQ(type1Hash(pdxType1), type2Hash(pdxType2));
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithSameFieldsNamesButDifferentTypesHaveDifferentHash) {
  PdxType pdxType1{gemfireJsonClassName};
  pdxType1.addField("bar0", PdxFieldTypes::STRING);
  pdxType1.addField("bar1", PdxFieldTypes::STRING);
  pdxType1.addField("bar2", PdxFieldTypes::STRING);

  PdxType pdxType2{gemfireJsonClassName};
  pdxType2.addField("bar0", PdxFieldTypes::STRING);
  pdxType2.addField("bar1", PdxFieldTypes::BOOLEAN);
  pdxType2.addField("bar2", PdxFieldTypes::INT);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;
  EXPECT_NE(type1Hash(pdxType1), type2Hash(pdxType2));
}

TEST_F(PdxTypeTest, testSerializeJavaPdxType) {
  PdxType pdxType{gemfireJsonClassName};
  pdxType.addField("foo", PdxFieldTypes::STRING);
  pdxType.addField("alice", PdxFieldTypes::STRING);
  pdxType.addField("bar1", PdxFieldTypes::BOOLEAN);

  DataOutputInternal out;
  pdxType.toData(out);

  EXPECT_BYTEARRAY_EQ(
      "2D2B2A00256F72672E6170616368652E67656F64652E7064782E696E7465726E"
      "616C2E506478547970652A000E5F5F47454D464952455F4A534F4E0000000000"
      "00000001032A0003666F6F0000000000000000090000000000000000002A0005"
      "616C6963650000000100000001090000000000000000002A0004626172310000"
      "00020000000000000000000000000000",
      ByteArray(out.getBuffer(), out.getBufferLength()));
}

TEST_F(PdxTypeTest, testSerializeNoJavaPdxType) {
  PdxType pdxType{gemfireJsonClassName, false};
  pdxType.addField("foo", PdxFieldTypes::STRING);
  pdxType.addField("alice", PdxFieldTypes::STRING);
  pdxType.addField("bar1", PdxFieldTypes::BOOLEAN);

  DataOutputInternal out;
  pdxType.toData(out);

  EXPECT_BYTEARRAY_EQ(
      "2D2B2A00256F72672E6170616368652E67656F64652E7064782E696E7465726E"
      "616C2E506478547970652A000E5F5F47454D464952455F4A534F4E0100000000"
      "00000001032A0003666F6F0000000000000000090000000000000000002A0005"
      "616C6963650000000100000001090000000000000000002A0004626172310000"
      "00020000000000000000000000000000",
      ByteArray(out.getBuffer(), out.getBufferLength()));
}

TEST_F(PdxTypeTest, testDeserializeJavaPdxType) {
  PdxType pdxExpected{gemfireJsonClassName};
  pdxExpected.addField("foo", PdxFieldTypes::STRING);
  pdxExpected.addField("alice", PdxFieldTypes::STRING);
  pdxExpected.addField("bar1", PdxFieldTypes::BOOLEAN);

  DataOutputInternal out;
  pdxExpected.toData(out);

  auto pdxType =
      std::dynamic_pointer_cast<PdxType>(PdxType::createDeserializable());
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  pdxType->fromData(in);

  EXPECT_EQ(pdxExpected, *pdxType);
}

TEST_F(PdxTypeTest, testDeserializeNoJavaPdxType) {
  PdxType pdxExpected{gemfireJsonClassName, false};
  pdxExpected.addField("foo", PdxFieldTypes::STRING);
  pdxExpected.addField("alice", PdxFieldTypes::STRING);
  pdxExpected.addField("bar1", PdxFieldTypes::BOOLEAN);

  DataOutputInternal out;
  pdxExpected.toData(out);

  auto pdxType =
      std::dynamic_pointer_cast<PdxType>(PdxType::createDeserializable());
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  pdxType->fromData(in);

  EXPECT_EQ(pdxExpected, *pdxType);
}

}  // namespace
