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

#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "PdxTypes.hpp"

namespace {
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxTypeRegistry;
using apache::geode::client::PdxTypes;

const std::string gemfireJsonClassName = "__GEMFIRE_JSON";

TEST(PdxTypeTest, testTwoObjectsWithSameClassnameAndSameFieldsAreEquals) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry,
                                            gemfireJsonClassName, false);

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

TEST(PdxTypeTest,
     testTwoObjectsWithSameClassnameAndSameFieldsInDifferentOrderAreEquals) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
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

TEST(PdxTypeTest,
     testTwoObjectsWithDifferentClassnameButSameFieldsAreNotEquals) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry, "otherClassName",
                                            false);

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

TEST(PdxTypeTest, testTwoObjectsWithSameFieldsHaveTheSameHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry,
                                            gemfireJsonClassName, false);

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

  std::hash<apache::geode::client::PdxType> type1Hash;
  std::hash<apache::geode::client::PdxType> type2Hash;

  EXPECT_EQ(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST(PdxTypeTest, testTwoObjectsWithDifferentFieldsHaveDifferentHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry,
                                            gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);

  m_pdxType2.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addVariableLengthTypeField("bar3", "string",
                                        PdxFieldTypes::STRING);

  std::hash<apache::geode::client::PdxType> type1Hash;
  std::hash<apache::geode::client::PdxType> type2Hash;

  EXPECT_NE(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST(PdxTypeTest, testTwoObjectsWithSameFieldsInDifferentOrderHaveTheSameHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry,
                                            gemfireJsonClassName, false);

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

  std::hash<apache::geode::client::PdxType> type1Hash;
  std::hash<apache::geode::client::PdxType> type2Hash;

  EXPECT_EQ(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

TEST(PdxTypeTest,
     testTwoObjectsWithSameFieldsNamesButDifferentTypesHaveDifferentHash) {
  PdxTypeRegistry pdxTypeRegistry(nullptr);

  apache::geode::client::PdxType m_pdxType1(pdxTypeRegistry,
                                            gemfireJsonClassName, false);
  apache::geode::client::PdxType m_pdxType2(pdxTypeRegistry,
                                            gemfireJsonClassName, false);

  m_pdxType1.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar1", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType1.addVariableLengthTypeField("bar2", "string",
                                        PdxFieldTypes::STRING);

  m_pdxType2.addVariableLengthTypeField("bar0", "string",
                                        PdxFieldTypes::STRING);
  m_pdxType2.addFixedLengthTypeField("bar1", "bool", PdxFieldTypes::BOOLEAN,
                                     PdxTypes::BOOLEAN_SIZE);
  m_pdxType2.addFixedLengthTypeField("bar2", "int", PdxFieldTypes::INT,
                                     PdxTypes::INTEGER_SIZE);

  std::hash<apache::geode::client::PdxType> type1Hash;
  std::hash<apache::geode::client::PdxType> type2Hash;

  EXPECT_NE(type1Hash(m_pdxType1), type2Hash(m_pdxType2));
}

}  // namespace
