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

#ifndef NATIVECLIENT_INTERNALID_HPP
#define NATIVECLIENT_INTERNALID_HPP

namespace apache {
namespace geode {
namespace client {
namespace internal {

enum class InternalId : int8_t {
  // Do not use IDs 7 and 8 which are used by .NET
  // ManagedObject and ManagedObjectXml. If those are
  // required then change those in GeodeTypeIdsM.hpp
  CacheableManagedObject = 7,
  CacheableManagedObjectXml = 8,

};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // NATIVECLIENT_INTERNALID_HPP
