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
#include "PdxTypes.hpp"

namespace {
using apache::geode::client::ByteArray;
using apache::geode::client::DataInputInternal;
using apache::geode::client::DataOutputInternal;
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxType;
using apache::geode::client::PdxTypeRegistry;
using apache::geode::client::PdxTypes;

const std::string gemfireJsonClassName = "__GEMFIRE_JSON";

class PdxTypeTest : public ::testing::Test, public ByteArrayFixture {};

TEST_F(PdxTypeTest, testTwoObjectsWithSameClassnameAndSameFieldsAreEquals) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);

  EXPECT_TRUE(m_pdxType1 == m_pdxType2);
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithSameClassnameAndSameFieldsInDifferentOrderAreEquals) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, gemfireJsonClassName, false);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);

  EXPECT_TRUE(m_pdxType1 == m_pdxType2);
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithDifferentClassnameButSameFieldsAreNotEquals) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, "otherClassName", false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);

  EXPECT_FALSE(m_pdxType1 == m_pdxType2);
}

TEST_F(PdxTypeTest, testTwoObjectsWithSameFieldsHaveTheSameHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);

  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;

  EXPECT_EQ(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST_F(PdxTypeTest, testTwoObjectsWithDifferentFieldsHaveDifferentHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);

  m_pdxType2.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar3", "string",
                                        PdxFieldTypes::STRING);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;

  EXPECT_NE(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithSameFieldsInDifferentOrderHaveTheSameHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);

  m_pdxType2.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;

  EXPECT_EQ(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST_F(PdxTypeTest,
       testTwoObjectsWithSameFieldsNamesButDifferentTypesHaveDifferentHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  PdxType m_pdxType1(pdxTypeRegistry, gemfireJsonClassName, false);
  PdxType m_pdxType2(pdxTypeRegistry, gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);

  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addFixedLengthTypeField("bar1", "bool", PdxFieldTypes::BOOLEAN,
                                     PdxTypes::PDX_BOOLEAN_SIZE);
  m_pdxType2.addFixedLengthTypeField("bar2", "int", PdxFieldTypes::INT,
                                     PdxTypes::PDX_INTEGER_SIZE);

  std::hash<PdxType> type1Hash;
  std::hash<PdxType> type2Hash;

  EXPECT_NE(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST_F(PdxTypeTest, testSerializeJavaPdxType) {
  PdxTypeRegistry pdx_type_registry{nullptr};
  PdxType pdx_type{pdx_type_registry, gemfireJsonClassName, false};

  pdx_type.addVariableLengthTypeField("foo", "", PdxFieldTypes::STRING);
  pdx_type.addVariableLengthTypeField("alice", "", PdxFieldTypes::STRING);
  pdx_type.addFixedLengthTypeField("bar1", "", PdxFieldTypes::BOOLEAN,
                                   PdxTypes::PDX_BOOLEAN_SIZE);

  DataOutputInternal out;
  pdx_type.toData(out);

  EXPECT_BYTEARRAY_EQ(
      "2D2B2A00256F72672E6170616368652E67656F64652E7064782E696E7465726E"
      "616C2E506478547970652A000E5F5F47454D464952455F4A534F4E0000000000"
      "00000001032A0003666F6F0000000000000000090000000000000000002A0005"
      "616C6963650000000100000001090000000000000000002A0004626172310000"
      "00020000000000000000000000000000",
      ByteArray(out.getBuffer(), out.getBufferLength()));
}

TEST_F(PdxTypeTest, testSerializeNoJavaPdxType) {
  PdxTypeRegistry pdx_type_registry{nullptr};
  PdxType pdx_type{pdx_type_registry, gemfireJsonClassName, false, false};

  pdx_type.addVariableLengthTypeField("foo", "", PdxFieldTypes::STRING);
  pdx_type.addVariableLengthTypeField("alice", "", PdxFieldTypes::STRING);
  pdx_type.addFixedLengthTypeField("bar1", "", PdxFieldTypes::BOOLEAN,
                                   PdxTypes::PDX_BOOLEAN_SIZE);

  DataOutputInternal out;
  pdx_type.toData(out);

  EXPECT_BYTEARRAY_EQ(
      "2D2B2A00256F72672E6170616368652E67656F64652E7064782E696E7465726E"
      "616C2E506478547970652A000E5F5F47454D464952455F4A534F4E0100000000"
      "00000001032A0003666F6F0000000000000000090000000000000000002A0005"
      "616C6963650000000100000001090000000000000000002A0004626172310000"
      "00020000000000000000000000000000",
      ByteArray(out.getBuffer(), out.getBufferLength()));
}

TEST_F(PdxTypeTest, testDeserializeJavaPdxType) {
  PdxTypeRegistry pdx_type_registry{nullptr};
  PdxType pdx_expected{pdx_type_registry, gemfireJsonClassName, false};

  pdx_expected.addVariableLengthTypeField("foo", "", PdxFieldTypes::STRING);
  pdx_expected.addVariableLengthTypeField("alice", "", PdxFieldTypes::STRING);
  pdx_expected.addFixedLengthTypeField("bar1", "", PdxFieldTypes::BOOLEAN,
                                       PdxTypes::PDX_BOOLEAN_SIZE);

  DataOutputInternal out;
  pdx_expected.toData(out);

  auto pdx_type = std::dynamic_pointer_cast<PdxType>(
      PdxType::CreateDeserializable(pdx_type_registry));
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  pdx_type->fromData(in);

  EXPECT_EQ(pdx_expected, *pdx_type);
}

TEST_F(PdxTypeTest, testDeserializeNoJavaPdxType) {
  PdxTypeRegistry pdx_type_registry{nullptr};
  PdxType pdx_expected{pdx_type_registry, gemfireJsonClassName, false, false};

  pdx_expected.addVariableLengthTypeField("foo", "", PdxFieldTypes::STRING);
  pdx_expected.addVariableLengthTypeField("alice", "", PdxFieldTypes::STRING);
  pdx_expected.addFixedLengthTypeField("bar1", "", PdxFieldTypes::BOOLEAN,
                                       PdxTypes::PDX_BOOLEAN_SIZE);

  DataOutputInternal out;
  pdx_expected.toData(out);

  auto pdx_type = std::dynamic_pointer_cast<PdxType>(
      PdxType::CreateDeserializable(pdx_type_registry));
  DataInputInternal in(out.getBuffer(), out.getBufferLength());
  pdx_type->fromData(in);

  EXPECT_EQ(pdx_expected, *pdx_type);
}

}  // namespace
