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
#include <gmock/gmock.h>

#include "TcrMessage.hpp"
#include "CacheRegionHelper.hpp"
#include "MemberListForVersionStamp.hpp"
#include "CacheImpl.hpp"

using apache::geode::client::TcrMessage;
using apache::geode::client::DataInput;
using apache::geode::client::Cache;
using apache::geode::client::CacheFactory;
using apache::geode::client::CacheImpl;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::MemberListForVersionStamp;
using apache::geode::client::DSCode;
using apache::geode::client::Exception;
using apache::geode::client::DataSerializablePrimitive;
using apache::geode::client::DataOutput;
using apache::geode::client::CacheableString;
using apache::geode::client::ThinClientBaseDM;
using apache::geode::client::TcrMessageReply;
using apache::geode::client::TcrEndpoint;
using apache::geode::client::TcrConnectionManager;
using apache::geode::client::ThinClientRegion;

namespace {
    class TcrMessageDummy : public TcrMessage {
    public:
        TcrMessageDummy() {}
        explicit TcrMessageDummy(ThinClientBaseDM &tc) {
            m_tcdm = &tc;
        }

        using TcrMessage::readOldValue;
        using TcrMessage::readIntPart;
        using TcrMessage::readLongPart;
        using TcrMessage::readCallbackObjectPart;
        using TcrMessage::readExceptionPart;
        using TcrMessage::writeBytesOnly;
    };

    class ThinClientBaseDMDummy : public ThinClientBaseDM {
    public:
        ThinClientBaseDMDummy(TcrConnectionManager& connManager,
                              ThinClientRegion* theRegion) : ThinClientBaseDM(connManager, theRegion) {}
        ~ThinClientBaseDMDummy() override {}

        GfErrType sendSyncRequest(TcrMessage&, TcrMessageReply&,
                                          bool = true,
                                          bool = false) override {
            return GF_NOERR;
        }

        GfErrType sendRequestToEP(const TcrMessage&,
                                          TcrMessageReply&,
                                          TcrEndpoint*) override {
            return GF_NOERR;
        }
    };
}

TEST(TcrMessage, readOldValue) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0,0,0,0 //int32
                              ,0 //isObj
                              ,static_cast<uint8_t>(DSCode::NullObj)

    };
    auto input = cache.createDataInput(buffer, sizeof(buffer));

    message.readOldValue(input);

    ASSERT_EQ(input.currentBufferPosition(), buffer + sizeof(buffer));
}

TEST(TcrMessage, readVersionTagPart) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0, //isObj
                              static_cast<uint8_t>(DSCode::FixedIDDefault)
    };

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    MemberListForVersionStamp memberListForVersionStamp;

    auto versionTag = message.readVersionTagPart(input, 0, memberListForVersionStamp);

    EXPECT_EQ(nullptr, versionTag.get());
}

TEST(TcrMessage, readIntPart) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0, 0, 0, 0};

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    uint32_t value;

    EXPECT_THROW(message.readIntPart(input, &value), Exception);
}

TEST(TcrMessage, readLongPart1) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0, 0, 0, 0}; //Size

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    uint64_t value;

    EXPECT_THROW(message.readLongPart(input, &value), Exception);
}

TEST(TcrMessage, readLongPart2) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0, 0, 0, 8, //Size
                              1};         //isObj

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    uint64_t value;

    EXPECT_THROW(message.readLongPart(input, &value), Exception);
}

TEST(TcrMessage, readLongPart3) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0, 0, 0, 8,                //Size
                              0,                         //isObj
                              0, 0, 0, 0, 0, 0, 0, 255}; //value

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    uint64_t value;

    message.readLongPart(input, &value);

    EXPECT_EQ(value, 255ull);
}

TEST(TcrMessage, readCallbackObjectPart1) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    const uint8_t buffer[] = {0, 0, 0, 3, //Size
                              0,         //isObj
                              'a', 'b', 'c' }; //callback

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    message.readCallbackObjectPart(input, true);

    EXPECT_STRCASEEQ("abc", message.getCallbackArgumentRef()->toString().c_str());
}

TEST(TcrMessage, readCallbackObjectPart2) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    uint8_t buffer[] = {0, 0, 0, 3, //Size
                              0,         //isObj
                              'a', 'b', 'c' }; //callback

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    message.readCallbackObjectPart(input, false);

    const auto&& dataSerializablePrimitive = std::dynamic_pointer_cast<DataSerializablePrimitive>(message.getCallbackArgumentRef());
    ASSERT_THAT(dataSerializablePrimitive.get(), ::testing::NotNull());
    auto output = cache.createDataOutput();
    dataSerializablePrimitive->toData(output);
    EXPECT_EQ(output.getBufferLength(), 4);

    uint8_t b[] = {3, 'a','b','c'};
    EXPECT_THAT(b, ::testing::ElementsAreArray(output.getBuffer(), output.getBufferLength()));
}

TEST(TcrMessage, readSecureObjectPart1) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    uint8_t buffer[] = {0, 0, 0, 1, //Size
                        1,         //isObj
                        static_cast<uint8_t>(DSCode::CacheableBytes),
                        3,
                        'a', 'b', 'c' };

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    message.readSecureObjectPart(input);

    const auto&& dataSerializablePrimitive = std::dynamic_pointer_cast<DataSerializablePrimitive>(message.getValueRef());
    ASSERT_THAT(dataSerializablePrimitive.get(), ::testing::NotNull());
    auto output = cache.createDataOutput();
    dataSerializablePrimitive->toData(output);
    EXPECT_EQ(output.getBufferLength(), 4);

    uint8_t b[] = {3, 'a','b','c'};
    EXPECT_THAT(b, ::testing::ElementsAreArray(output.getBuffer(), output.getBufferLength()));
}

TEST(TcrMessage, readSecureObjectPart2) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    uint8_t buffer[] = {0, 0, 0, 3, //Size
                        0,         //isObj
                        'a', 'b', 'c' };

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    message.readSecureObjectPart(input, true);

    const auto&& cacheableString = std::dynamic_pointer_cast<CacheableString>(message.getValueRef());
    ASSERT_THAT(cacheableString.get(), ::testing::NotNull());
    EXPECT_EQ(cacheableString->toString(), "abc");
}

TEST(TcrMessage, readSecureObjectPart3) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    uint8_t buffer[] = {0, 0, 0, 0, //Size
                        0,         //isObj
                        0 }; //extra data

    auto input = cache.createDataInput(buffer, sizeof(buffer));

    EXPECT_THROW(message.readSecureObjectPart(input), Exception);
}

TEST(TcrMEssage, getConnectionId) {
    EXPECT_EQ(TcrMessageDummy().getConnectionId(nullptr), 0);
}

TEST(TcrMessage, getUniqueId) {
    EXPECT_EQ(TcrMessageDummy().getUniqueId(nullptr), 0);
}

TEST(TcrMessage, readExceptionPart) {
    auto cache = CacheFactory().create();
    TcrMessageDummy message;
    auto input = cache.createDataInput(nullptr, 0);

    EXPECT_FALSE(message.readExceptionPart(input, 0, false));
}

TEST(TcrMessage, DISABLED_writeBytesOnly) {
    auto cache = CacheFactory().create();
    ThinClientBaseDMDummy dm(CacheRegionHelper::getCacheImpl(&cache)->tcrConnectionManager(), nullptr);
    TcrMessageDummy message(dm);
    const auto cacheableString = CacheableString::create("abc");

    MemberListForVersionStamp memberListForVersionStamp;
    message.setData(nullptr, 0, 0, *CacheRegionHelper::getCacheImpl(&cache)->getSerializationRegistry(), memberListForVersionStamp);
    message.writeBytesOnly(cacheableString);
    uint8_t b[] = {static_cast<uint8_t>(DSCode::CacheableASCIIString),0, 3, 'a','b','c'};
    EXPECT_THAT(b, ::testing::ElementsAreArray(message.getMsgData(), message.getMsgLength() + 2));
}
