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

#ifndef NATIVECLIENT_DSFIXEDID_HPP
#define NATIVECLIENT_DSFIXEDID_HPP

namespace apache {
namespace geode {
namespace client {
namespace internal {

enum class DSFid : int32_t {
  GatewaySenderEventCallbackArgument = -135,
  ClientHealthStats = -126,
  VersionTag = -120,
  CollectionTypeImpl = -59,
  LocatorListRequest = -54,
  ClientConnectionRequest = -53,
  QueueConnectionRequest = -52,
  LocatorListResponse = -51,
  ClientConnectionResponse = -50,
  QueueConnectionResponse = -49,
  ClientReplacementRequest = -48,
  GetAllServersRequest = -43,
  GetAllServersResponse = -42,
  VersionedObjectPartList = 7,
  EnumInfo = 9,
  CacheableObjectPartList = 25,
  CacheableUndefined = 31,
  Struct = 32,
  EventId = 36,
  InterestResultPolicy = 37,
  ClientProxyMembershipId = 38,
  InternalDistributedMember = 92,
  TXCommitMessage = 110,
  DiskVersionTag = 2131,
  DiskStoreId = 2133
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // NATIVECLIENT_DSFIXEDID_HPP
