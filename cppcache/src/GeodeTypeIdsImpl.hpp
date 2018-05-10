#pragma once

#ifndef GEODE_GEODETYPEIDSIMPL_H_
#define GEODE_GEODETYPEIDSIMPL_H_

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

namespace apache {
namespace geode {
namespace client {

class GeodeTypeIdsImpl {
 public:
  // Internal IDs here
  // User visible IDs are in GeodeTypeIds.hpp

  enum IdValues {
    // keep the following in alphabetical order, please.
    CacheableObjectPartList = 25,
    CacheableObjectPartList66 = 2121,
    CacheableToken = 14,  // because there's no equivalence in java
    CacheableUserClass = 40,
    CacheableUserData = 39,
    CacheableUserData2 = 38,
    CacheableUserData4 = 37,
    Class = 43,
    ClientConnectionRequest = -53,
    ClientConnectionResponse = -50,
    ClientHealthStats = -126,
    ClientProxyMembershipId = 38,
    ClientReplacementRequest = -48,
    CollectionTypeImpl = -59,
    DataSerializable = 45,
    DiskStoreId = 2133,
    DiskVersionTag = 2131,
    EntryEventImpl = 105,
    EventId = 36,
    FixedIDByte = 1,
    FixedIDDefault = 0,
    FixedIDInt = 3,
    FixedIDNone = 4,
    FixedIDShort = 2,
    GatewayEventCallbackArgument = -56,         // 0xC8
    GatewaySenderEventCallbackArgument = -135,  // 0xC8
    GetAllServersRequest = -43,
    GetAllServersResponse = -42,
    InterestResultPolicy = 37,
    InternalDistributedMember = 92,
    JavaSerializable = 44,
    LocatorListRequest = -54,
    LocatorListResponse = -51,
    ObjectTypeImpl = -61,
    PDX = 93,
    // PDX_ENUM = 94,
    PreferBytesCachedDeserializable = -65,
    QueueConnectionRequest = -52,
    QueueConnectionResponse = -49,
    RegionEventImpl = 108,
    StructTypeImpl = -60,
    TXCommitMessage = 110,
    VersionedObjectPartList = 7,
    VersionTag = -120,
    VmCachedDeserializable = -64
  };
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_GEODETYPEIDSIMPL_H_
