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

#include <iostream>

#include <gtest/gtest.h>

#include <geode/CacheFactory.hpp>
#include <geode/CqState.hpp>

#include "ByteArrayFixture.hpp"
#include "SerializationRegistry.hpp"
#include "TcrMessage.hpp"
#include "TcrMessageDestroy.hpp"
#include "TcrMessagePut.hpp"

namespace {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableHashSet;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheableVector;
using apache::geode::client::CqState;
using apache::geode::client::EventOperation;
using apache::geode::client::DataOutput;
using apache::geode::client::InterestResultPolicy;
using apache::geode::client::Region;
using apache::geode::client::Serializable;
using apache::geode::client::SerializationRegistry;
using apache::geode::client::TcrMessage;
using apache::geode::client::TcrMessageReply;
using apache::geode::client::ThinClientBaseDM;

class DataOutputUnderTest : public DataOutput {
 public:
  using DataOutput::DataOutput;
  DataOutputUnderTest() : DataOutput(nullptr, nullptr) {}
  ~DataOutputUnderTest() noexcept override {}

 protected:
  virtual const SerializationRegistry &getSerializationRegistry()
      const override {
    return m_serializationRegistry;
  }

 private:
  SerializationRegistry m_serializationRegistry;
};

#define EXPECT_MESSAGE_EQ(e, a) EXPECT_PRED_FORMAT2(assertMessageEqual, e, a)

class TcrMessageTest : public ::testing::Test, protected ByteArrayFixture {
 public:
  ::testing::AssertionResult assertMessageEqual(
      const char *expectedStr, const char *bytesStr, const char *expected,
      const apache::geode::client::TcrMessage &msg) {
    apache::geode::client::ByteArray bytes(
        reinterpret_cast<const uint8_t *>(msg.getMsgData()),
        static_cast<const std::size_t>(msg.getMsgLength()));
    return ByteArrayFixture::assertByteArrayEqual(expectedStr, bytesStr,
                                                  expected, bytes);
  }
};

TEST_F(TcrMessageTest, intializeDefaultConstructor) {
  TcrMessageReply message(true, nullptr);

  EXPECT_EQ(TcrMessage::INVALID, message.getMessageType());
}

TEST_F(TcrMessageTest, testConstructor1MessageDataContentWithDestoryRegion) {
  using apache::geode::client::TcrMessageDestroyRegion;

  const Region *region = nullptr;
  const std::shared_ptr<Serializable> aCallbackArgument = nullptr;
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;

  TcrMessageDestroyRegion message(new DataOutputUnderTest(), region,
                                  aCallbackArgument, messageResponseTimeout,
                                  connectionDM);

  EXPECT_EQ(TcrMessage::DESTROY_REGION, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000000B0000003800000003\\h{8}"
      "000000001300494E56414C49445F524547494F4E5F4E414D450000001200030000000000"
      "00000103\\h{16}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testConstructor1MessageDataContentWithClearRegion) {
  using apache::geode::client::TcrMessageClearRegion;

  const Region *region = nullptr;
  const std::shared_ptr<Serializable> aCallbackArgument = nullptr;
  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;

  TcrMessageClearRegion message(new DataOutputUnderTest(), region,
                                aCallbackArgument, messageResponseTimeout,
                                connectionDM);

  EXPECT_MESSAGE_EQ(
      "000000240000003800000003\\h{8}"
      "000000001300494E56414C49445F524547494F4E5F4E414D450000001200030000000000"
      "00000103\\h{16}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorMessageDataContent) {
  using apache::geode::client::TcrMessageCloseCQ;

  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;

  TcrMessageCloseCQ message(new DataOutputUnderTest(), "myRegionName",
                            messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::CLOSECQ_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002D0000003100000003FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "00120003000000000000000103\\h{16}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorWithQuery) {
  using apache::geode::client::TcrMessageQuery;

  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;

  TcrMessageQuery message(new DataOutputUnderTest(), "aRegionName",
                          messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::QUERY, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000220000003000000003FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "120003000000000000000103\\h{16}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorWithStopCq) {
  using apache::geode::client::TcrMessageStopCQ;

  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;

  TcrMessageStopCQ message(new DataOutputUnderTest(), "aRegionName",
                           messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::STOPCQ_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002C0000003000000003FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "120003000000000000000103\\h{16}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testQueryConstructorWithCloseCq) {
  using apache::geode::client::TcrMessageCloseCQ;

  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;

  TcrMessageCloseCQ message(new DataOutputUnderTest(), "aRegionName",
                            messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::CLOSECQ_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002D0000003000000003FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "120003000000000000000103\\h{16}0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest,
       testParameterizedQueryConstructorWithQueryWithParameters) {
  using apache::geode::client::TcrMessageQueryWithParameters;

  std::chrono::milliseconds messageResponseTimeout{1000};
  ThinClientBaseDM *connectionDM = nullptr;
  const std::shared_ptr<Serializable> aCallbackArgument = nullptr;
  auto paramList = CacheableVector::create();

  TcrMessageQueryWithParameters message(
      new DataOutputUnderTest(), "aRegionName", aCallbackArgument, paramList,
      messageResponseTimeout, connectionDM);

  EXPECT_EQ(TcrMessage::QUERY_WITH_PARAMETERS, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000500000002B00000004FFFFFFFF000000000B0061526567696F6E4E616D65000000"
      "04000000000000000004000000000F0000000400000003E8",
      message);
}

TEST_F(TcrMessageTest, testConstructorWithContainsKey) {
  using apache::geode::client::TcrMessageContainsKey;

  TcrMessageContainsKey message(
      new DataOutputUnderTest(), static_cast<const Region *>(nullptr),
      CacheableString::create(
          "mykey"),  // static_cast<const
                     // std::shared_ptr<CacheableKey>>(nullptr),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      true,  // isContainsKey
      static_cast<ThinClientBaseDM *>(nullptr));
  EXPECT_EQ(TcrMessage::CONTAINS_KEY, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000260000002E00000003FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579000000040000000000",
      message);
}

TEST_F(TcrMessageTest, testConstructorWithGetDurableCqs) {
  using apache::geode::client::TcrMessageGetDurableCqs;

  TcrMessageGetDurableCqs message(new DataOutputUnderTest(),
                                  static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GETDURABLECQS_MSG_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000690000000600000001FFFFFFFF00000000010000", message);
}

TEST_F(TcrMessageTest, testConstructor2WithREQUEST) {
  using apache::geode::client::TcrMessageRequest;

  TcrMessageRequest message(
      new DataOutputUnderTest(), static_cast<const Region *>(nullptr),
      CacheableString::create(
          "mykey"),  // static_cast<const
                     // std::shared_ptr<CacheableKey>>(nullptr),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REQUEST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000000000002500000002FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579",
      message);
}

TEST_F(TcrMessageTest, testDestroy) {
  using apache::geode::client::TcrMessageDestroy;

  TcrMessageDestroy message(new DataOutputUnderTest(), nullptr,
                            CacheableString::create("mykey"), nullptr, nullptr,
                            EventOperation::DESTROY, nullptr);

  EXPECT_EQ(TcrMessage::DESTROY, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000090000004800000005FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579000000010129000000010013000000120003"
      "000000000000000103\\h{16}",
      message);
}

TEST_F(TcrMessageTest, testRemove) {
  using apache::geode::client::TcrMessageDestroy;

  TcrMessageDestroy message(new DataOutputUnderTest(), nullptr,
                            CacheableString::create("mykey"), nullptr, nullptr,
                            EventOperation::REMOVE, nullptr);

  EXPECT_EQ(TcrMessage::DESTROY, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000090000004800000005FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B657900000001012900000001002E000000120003"
      "000000000000000103\\h{16}",
      message);
}

TEST_F(TcrMessageTest, testConstructor2WithInvalidate) {
  using apache::geode::client::TcrMessageInvalidate;

  TcrMessageInvalidate message(
      new DataOutputUnderTest(), static_cast<const Region *>(nullptr),
      CacheableString::create(
          "mykey"),  // static_cast<const
                     // std::shared_ptr<CacheableKey>>(nullptr),
      static_cast<const std::shared_ptr<Serializable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::INVALIDATE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000530000003C00000003FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000008015700056D796B6579000000120003000000000000000103\\h{"
      "16}",
      message);
}

TEST_F(TcrMessageTest, testPut) {
  using apache::geode::client::TcrMessagePut;

  TcrMessagePut message(
      new DataOutputUnderTest(), nullptr, CacheableString::create("mykey"),
      CacheableString::create("myvalue"), nullptr, EventOperation::UPDATE,
      false, nullptr, false, false, "myRegionName");

  EXPECT_EQ(TcrMessage::PUT, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000070000005A00000007FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "0001000C00000004000000000000000008015700056D796B657900000002013500000000"
      "0A015700076D7976616C7565000000120003000000000000000103\\h"
      "{16}",
      message);
}

TEST_F(TcrMessageTest, testPutIfAbsent) {
  using apache::geode::client::TcrMessagePut;

  TcrMessagePut message(new DataOutputUnderTest(), nullptr,
                        CacheableString::create("mykey"),
                        CacheableString::create("myvalue"), nullptr,
                        EventOperation::PUT_IF_ABSENT, false, nullptr, false,
                        false, "myRegionName");

  EXPECT_EQ(TcrMessage::PUT, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000070000005A00000007FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "0001002C00000004000000000000000008015700056D796B657900000002013500000000"
      "0A015700076D7976616C7565000000120003000000000000000103\\h"
      "{16}",
      message);
}

TEST_F(TcrMessageTest, testCreate) {
  using apache::geode::client::TcrMessagePut;

  TcrMessagePut message(
      new DataOutputUnderTest(), nullptr,
      CacheableString::create("mykey"), CacheableString::create("myvalue"),
      nullptr,
      EventOperation::CREATE,
      false, nullptr,
      false,
      false,
      "myRegionName");

  EXPECT_EQ(TcrMessage::PUT, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000070000005A00000007FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "0001000100000004000000000000000008015700056D796B657900000002013500000000"
      "0A015700076D7976616C7565000000120003000000000000000103\\h"
      "{16}",
      message);
}

TEST_F(TcrMessageTest, testReplace) {
  using apache::geode::client::TcrMessagePut;

  TcrMessagePut message(
      new DataOutputUnderTest(), nullptr, CacheableString::create("mykey"),
      CacheableString::create("myvalue"), nullptr, EventOperation::REPLACE,
      false, nullptr, false, false, "myRegionName");

  EXPECT_EQ(TcrMessage::PUT, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000070000005A00000007FFFFFFFF000000000C006D79526567696F6E4E616D650000"
      "0001002D00000004000000000000000008015700056D796B657900000002013500000000"
      "0A015700076D7976616C7565000000120003000000000000000103\\h"
      "{16}",
      message);
}

TEST_F(TcrMessageTest, testConstructor4) {
  using apache::geode::client::TcrMessageClearRegion;

  TcrMessageReply message(false,  // decodeAll
                          static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::INVALID, message.getMessageType());
}

TEST_F(TcrMessageTest, TcrMessageRegisterInterestList) {
  using apache::geode::client::TcrMessageRegisterInterestList;

  std::vector<std::shared_ptr<CacheableKey>> keys;
  keys.push_back(CacheableString::create("mykey"));

  TcrMessageRegisterInterestList message(
      new DataOutputUnderTest(), static_cast<const Region *>(nullptr), keys,
      false,  // isDurable
      false,  // isCacheingEnabled
      false,  // receiveValues
      InterestResultPolicy::NONE, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST_LIST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000180000004200000006FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000003010125000000000100000000000A0141015700056D796B65790000"
      "0001000100000002000000",
      message);
}

TEST_F(TcrMessageTest, TcrMessageRegisterInterestListWithManyKeys) {
  using apache::geode::client::TcrMessageRegisterInterestList;

  auto keys = std::vector<std::shared_ptr<CacheableKey>>{
      CacheableString::create("mykey1"), CacheableString::create("mykey2"),
      CacheableString::create("mykey3"), CacheableString::create("mykey4")};

  TcrMessageRegisterInterestList message(
      new DataOutputUnderTest(), static_cast<const Region *>(nullptr), keys,
      false,  // isDurable
      false,  // isCacheingEnabled
      false,  // receiveValues
      InterestResultPolicy::NONE, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST_LIST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000180000005E00000006FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D450000000301012500000000010000000000260141045700066D796B65793157"
      "00066D796B6579325700066D796B6579335700066D796B65793400000001000100000002"
      "000000",
      message);
}

TEST_F(TcrMessageTest, testConstructor5WithUnregisterInteresetList) {
  using apache::geode::client::TcrMessageUnregisterInterestList;

  std::vector<std::shared_ptr<CacheableKey>> keys;
  keys.push_back(CacheableString::create("mykey"));

  TcrMessageUnregisterInterestList message(
      new DataOutputUnderTest(), static_cast<const Region *>(nullptr), keys,
      false, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::UNREGISTER_INTEREST_LIST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000190000003A00000005FFFFFFFF000000001300494E56414C49445F524547494F4E"
      "5F4E414D4500000001000000000001000000000004000000000100000008015700056D79"
      "6B6579",
      message);
}

TEST_F(TcrMessageTest, testConstructorGetFunctionAttributes) {
  using apache::geode::client::TcrMessageGetFunctionAttributes;

  TcrMessageGetFunctionAttributes message(
      new DataOutputUnderTest(), std::string("myFunction"),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_FUNCTION_ATTRIBUTES, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000005B0000000F00000001FFFFFFFF000000000A006D7946756E6374696F6E",
      message);
}

TEST_F(TcrMessageTest, testConstructorKeySet) {
  using apache::geode::client::TcrMessageKeySet;

  TcrMessageKeySet message(new DataOutputUnderTest(),
                           std::string("myFunctionKeySet"),
                           static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::KEY_SET, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000280000001500000001FFFFFFFF0000000010006D7946756E6374696F6E4B657953"
      "6574",
      message);
}

TEST_F(TcrMessageTest, testConstructor6WithCreateRegion) {
  using apache::geode::client::TcrMessageCreateRegion;

  TcrMessageCreateRegion message(new DataOutputUnderTest(), "parentRegionName",
                                 "regionName",
                                 false,  // isDurable
                                 false,  // receiveValues
                                 static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::CREATE_REGION, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000001D0000002400000002FFFFFFFF000000001000706172656E74526567696F6E4E61"
      "6D650000000A00726567696F6E4E616D65",
      message);
}

TEST_F(TcrMessageTest, testConstructor6WithRegisterInterest) {
  using apache::geode::client::TcrMessageRegisterInterestRegex;

  TcrMessageRegisterInterestRegex message(
      new DataOutputUnderTest(), "regionName", "regexString",
      InterestResultPolicy::NONE,
      false,  // isDurable
      false,  // isCacheingEnabled
      false,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000004300000007FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "000000000100000003010125000000000100000000000B007265676578537472696E6700"
      "000001000100000002000000",
      message);

  TcrMessageRegisterInterestRegex message2(
      new DataOutputUnderTest(), "regionName", "regexString",
      InterestResultPolicy::NONE,
      true,   // isDurable
      false,  // isCacheingEnabled
      false,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message2.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000004300000007FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "000000000100000003010125000000000100010000000B007265676578537472696E6700"
      "000001000100000002000000",
      message2);

  TcrMessageRegisterInterestRegex message3(
      new DataOutputUnderTest(), "regionName", "regexString",
      InterestResultPolicy::NONE,
      false,  // isDurable
      true,   // isCacheingEnabled
      false,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message3.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000004300000007FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "000000000100000003010125000000000100000000000B007265676578537472696E6700"
      "000001000100000002000100",
      message3);

  TcrMessageRegisterInterestRegex message4(
      new DataOutputUnderTest(), "regionName", "regexString",
      InterestResultPolicy::NONE,
      false,  // isDurable
      false,  // isCacheingEnabled
      true,   // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message4.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000004300000007FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "000000000100000003010125000000000100000000000B007265676578537472696E6700"
      "000001000000000002000000",
      message4);

  TcrMessageRegisterInterestRegex message5(
      new DataOutputUnderTest(), "regionName", "regexString",
      InterestResultPolicy::KEYS,
      true,  // isDurable
      true,  // isCacheingEnabled
      true,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message5.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000004300000007FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "000000000100000003010125010000000100010000000B007265676578537472696E6700"
      "000001000000000002000100",
      message5);

  TcrMessageRegisterInterestRegex message6(
      new DataOutputUnderTest(), "regionName", "regexString",
      InterestResultPolicy::KEYS_VALUES,
      true,  // isDurable
      true,  // isCacheingEnabled
      true,  // receiveValues
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REGISTER_INTEREST, message6.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000140000004300000007FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "000000000100000003010125020000000100010000000B007265676578537472696E6700"
      "000001000000000002000100",
      message6);
}

TEST_F(TcrMessageTest, testConstructor6WithUnregisterInterest) {
  using apache::geode::client::TcrMessageUnregisterInterestRegex;

  TcrMessageUnregisterInterestRegex message(
      new DataOutputUnderTest(), "regionName", "regexString",
      false,  // isDurable
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::UNREGISTER_INTEREST, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000160000003400000005FFFFFFFF000000000A00726567696F6E4E616D6500000004"
      "00000000010000000B007265676578537472696E67000000010000000000010000",
      message);
}

TEST_F(TcrMessageTest, testConstructorGetPdxTypeById) {
  using apache::geode::client::TcrMessageGetPdxTypeById;

  TcrMessageGetPdxTypeById message(new DataOutputUnderTest(), 42,
                                   static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_TYPE_BY_ID, message.getMessageType());

  EXPECT_MESSAGE_EQ("0000005C0000000900000001FFFFFFFF0000000004000000002A",
                    message);
}

TEST_F(TcrMessageTest, testConstructorGetPdxEnumById) {
  using apache::geode::client::TcrMessageGetPdxEnumById;

  TcrMessageGetPdxEnumById message(new DataOutputUnderTest(), 42,
                                   static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_ENUM_BY_ID, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000620000000900000001FFFFFFFF0000000004000000002A",
                    message);
}

TEST_F(TcrMessageTest, testConstructorGetPdxIdForType) {
  using apache::geode::client::TcrMessageGetPdxIdForType;

  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());
  TcrMessageGetPdxIdForType message(new DataOutputUnderTest(), myPtr,
                                    static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_ID_FOR_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ("0000005D0000000700000001FFFFFFFF0000000002010000",
                    message);
}

TEST_F(TcrMessageTest, testConstructorAddPdxType) {
  using apache::geode::client::TcrMessageAddPdxType;

  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());
  TcrMessageAddPdxType message(new DataOutputUnderTest(), myPtr,
                               static_cast<ThinClientBaseDM *>(nullptr), 42);

  EXPECT_EQ(TcrMessage::ADD_PDX_TYPE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000005E0000001000000002FFFFFFFF000000000201000000000004000000002A",
      message);
}

TEST_F(TcrMessageTest, testConstructorGetPdxIdForEnum) {
  using apache::geode::client::TcrMessageGetPdxIdForEnum;

  TcrMessageGetPdxIdForEnum message(
      new DataOutputUnderTest(),
      static_cast<std::shared_ptr<Cacheable>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::GET_PDX_ID_FOR_ENUM, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000610000000600000001FFFFFFFF00000000010129", message);
}

TEST_F(TcrMessageTest, testConstructorAddPdxEnum) {
  using apache::geode::client::TcrMessageAddPdxEnum;

  TcrMessageAddPdxEnum message(new DataOutputUnderTest(),
                               static_cast<std::shared_ptr<Cacheable>>(nullptr),
                               static_cast<ThinClientBaseDM *>(nullptr), 42);

  EXPECT_EQ(TcrMessage::ADD_PDX_ENUM, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000600000000F00000002FFFFFFFF0000000001012900000004000000002A",
      message);
}

TEST_F(TcrMessageTest, testConstructorEventId) {
  using apache::geode::client::EventId;
  using apache::geode::client::TcrMessageRequestEventValue;

  TcrMessageRequestEventValue message(
      new DataOutputUnderTest(),
      static_cast<std::shared_ptr<EventId>>(nullptr));

  EXPECT_EQ(TcrMessage::REQUEST_EVENT_VALUE, message.getMessageType());

  EXPECT_MESSAGE_EQ("000000440000000600000001FFFFFFFF00000000010129", message);
}

TEST_F(TcrMessageTest, testConstructorRemoveUserAuth) {
  using apache::geode::client::TcrMessageRemoveUserAuth;

  TcrMessageRemoveUserAuth message(new DataOutputUnderTest(), true,
                                   static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REMOVE_USER_AUTH, message.getMessageType());

  EXPECT_MESSAGE_EQ("0000004E0000000600000001FFFFFFFF00000000010001", message);

  TcrMessageRemoveUserAuth message2(new DataOutputUnderTest(), false,
                                    static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::REMOVE_USER_AUTH, message2.getMessageType());

  EXPECT_MESSAGE_EQ("0000004E0000000600000001FFFFFFFF00000000010000", message2);
}

TEST_F(TcrMessageTest, testConstructorUserCredential) {
  using apache::geode::client::Properties;
  using apache::geode::client::TcrMessageUserCredential;

  TcrMessageUserCredential message(
      new DataOutputUnderTest(),
      static_cast<std::shared_ptr<Properties>>(nullptr),
      static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::USER_CREDENTIAL_MESSAGE, message.getMessageType());
  // this message is currently blank so this should change it if the impl
  // changes
  EXPECT_MESSAGE_EQ("", message);
}

TEST_F(TcrMessageTest, testConstructorGetClientPartitionAttributes) {
  using apache::geode::client::TcrMessageGetClientPartitionAttributes;

  TcrMessageGetClientPartitionAttributes message(new DataOutputUnderTest(),
                                                 "testClientRegion");

  EXPECT_EQ(TcrMessage::GET_CLIENT_PARTITION_ATTRIBUTES,
            message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000490000001500000001FFFFFFFF00000000100074657374436C69656E7452656769"
      "6F6E",
      message);
}

TEST_F(TcrMessageTest, testConstructorGetClientPrMetadata) {
  using apache::geode::client::TcrMessageGetClientPrMetadata;

  TcrMessageGetClientPrMetadata message(new DataOutputUnderTest(),
                                        "testClientRegionPRMETA");

  EXPECT_EQ(TcrMessage::GET_CLIENT_PR_METADATA, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000470000001B00000001FFFFFFFF00000000160074657374436C69656E7452656769"
      "6F6E50524D455441",
      message);
}
TEST_F(TcrMessageTest, testConstructorSize) {
  using apache::geode::client::TcrMessageSize;

  TcrMessageSize message(new DataOutputUnderTest(), "testClientRegionSIZE");

  EXPECT_EQ(TcrMessage::SIZE, message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "000000510000001900000001FFFFFFFF00000000140074657374436C69656E7452656769"
      "6F6E53495A45",
      message);
}

TEST_F(TcrMessageTest, testConstructorExecuteRegionFunctionSingleHop) {
  using apache::geode::client::TcrMessageExecuteRegionFunctionSingleHop;

  const Region *region = nullptr;

  auto myHashCachePtr = CacheableHashSet::create();

  std::shared_ptr<Cacheable> myPtr(CacheableString::createDeserializable());

  TcrMessageExecuteRegionFunctionSingleHop message(
      new DataOutputUnderTest(), "myFuncName", region, myPtr, myHashCachePtr, 2,
      myHashCachePtr,
      false,  // allBuckets
      std::chrono::milliseconds{1}, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP,
            message.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000004F0000005E00000009FFFFFFFF00000000050002000000010000001300494E5641"
      "4C49445F524547494F4E5F4E414D450000000A006D7946756E634E616D65000000030157"
      "000000000001012900000001000000000004000000000000000004000000000000000002"
      "014200",
      message);

  EXPECT_TRUE(message.hasResult());
}

TEST_F(TcrMessageTest, testConstructorExecuteRegionFunction) {
  using apache::geode::client::TcrMessageExecuteRegionFunction;

  const Region *region = nullptr;

  auto myHashCachePtr = CacheableHashSet::create();
  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());
  auto myVectPtr = CacheableVector::create();

  TcrMessageExecuteRegionFunction testMessage(
      new DataOutputUnderTest(), "ExecuteRegion", region, myCacheablePtr,
      myVectPtr, 2, myHashCachePtr, std::chrono::milliseconds{10},
      static_cast<ThinClientBaseDM *>(nullptr), 10);

  EXPECT_EQ(TcrMessage::EXECUTE_REGION_FUNCTION, testMessage.getMessageType());
  // this message is currently blank so this should change it if the impl
  // changes

  EXPECT_MESSAGE_EQ(
      "0000003B0000006100000009FFFFFFFF000000000500020000000A0000001300494E5641"
      "4C49445F524547494F4E5F4E414D450000000D0045786563757465526567696F6E000000"
      "030157000000000001012900000001000A00000004000000000000000004000000000000"
      "000002014200",
      testMessage);

  EXPECT_TRUE(testMessage.hasResult());
}

TEST_F(TcrMessageTest, DISABLED_testConstructorExecuteFunction) {
  using apache::geode::client::TcrMessageExecuteFunction;

  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  TcrMessageExecuteFunction testMessage(
      new DataOutputUnderTest(), "ExecuteFunction", myCacheablePtr, 1,
      static_cast<ThinClientBaseDM *>(nullptr), std::chrono::milliseconds{10});

  EXPECT_EQ(TcrMessage::EXECUTE_FUNCTION, testMessage.getMessageType());

  EXPECT_TRUE(testMessage.hasResult());

  EXPECT_MESSAGE_EQ(
      "0000003E0000002600000003FFFFFFFF000000000500010000000A0000000F0045786563"
      "75746546756E6374696F6E0000000301570000",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorExecuteCq) {
  using apache::geode::client::TcrMessageExecuteCq;

  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  TcrMessageExecuteCq testMessage(
      new DataOutputUnderTest(), "ExecuteCQ", "select * from /somewhere",
      CqState::RUNNING, false, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTECQ_MSG_TYPE, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002A0000004000000005FFFFFFFF0000000009004578656375746543510000001800"
      "73656C656374202A2066726F6D202F736F6D657768657265000000040000000001000000"
      "010000000000010001",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorWithGinormousQueryExecuteCq) {
  using apache::geode::client::TcrMessageExecuteCq;

  std::ostringstream oss;
  oss << "select * from /somewhere s where s.data.id in SET(";
  // Ensure over 64KiB of query string.
  const int n = (((64 * 1024) + 11) / 12);
  for (int i = 0; i < n; ++i) {
    if (0 < i) {
      oss << ',';
    }
    oss << '\'';
    oss.fill('0');
    oss.width(9);
    oss << i;
    oss << '\'';
  }
  oss << ") and s.type in SET('AAA','BBB','CCC','DDD') limit 60000";

  TcrMessageExecuteCq testMessage(new DataOutputUnderTest(), "ExecuteCQ",
                                  oss.str(), CqState::RUNNING, false,
                                  static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTECQ_MSG_TYPE, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002A0001009900000005FFFFFFFF0000000009004578656375746543510001007100"
      "\\h{131298}000000040000000001000000010000000000010001",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorExecuteCqWithIr) {
  using apache::geode::client::TcrMessageExecuteCqWithIr;

  TcrMessageExecuteCqWithIr testMessage(
      new DataOutputUnderTest(), "ExecuteCQWithIr", "select * from /somewhere",
      CqState::RUNNING, false, static_cast<ThinClientBaseDM *>(nullptr));

  EXPECT_EQ(TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE,
            testMessage.getMessageType());

  EXPECT_MESSAGE_EQ(
      "0000002B0000004600000005FFFFFFFF000000000F004578656375746543515769746849"
      "72000000180073656C656374202A2066726F6D202F736F6D657768657265000000040000"
      "000001000000010000000000010001",
      testMessage);
}

TEST_F(TcrMessageTest, testConstructorPing) {
  using apache::geode::client::TcrMessagePing;

  TcrMessagePing testMessage(
      std::unique_ptr<DataOutput>(new DataOutputUnderTest()));

  EXPECT_EQ(TcrMessage::PING, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ("000000050000000000000000FFFFFFFF00", testMessage);
}

TEST_F(TcrMessageTest, testConstructorCloseConnection) {
  using apache::geode::client::TcrMessageCloseConnection;

  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  TcrMessageCloseConnection testMessage(
      std::unique_ptr<DataOutput>(new DataOutputUnderTest()), false);

  EXPECT_EQ(TcrMessage::CLOSE_CONNECTION, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ("000000120000000600000001FFFFFFFF00000000010000",
                    testMessage);
}

TEST_F(TcrMessageTest, testConstructorCloseConnectionKeepAlive) {
  using apache::geode::client::TcrMessageCloseConnection;

  std::shared_ptr<Cacheable> myCacheablePtr(
      CacheableString::createDeserializable());

  TcrMessageCloseConnection testMessage(
      std::unique_ptr<DataOutput>(new DataOutputUnderTest()), true);

  EXPECT_EQ(TcrMessage::CLOSE_CONNECTION, testMessage.getMessageType());

  EXPECT_MESSAGE_EQ("000000120000000600000001FFFFFFFF00000000010001",
                    testMessage);
}

}  // namespace
