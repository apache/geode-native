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

#include "GatewaySenderEventCallbackArgument.hpp"

#include <geode/internal/DSFixedId.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

void GatewaySenderEventCallbackArgument::fromData(DataInput &input) {
  originatingDSId = input.readInt32();
  // Trying to get the recipientDSIds in these ways:
  //
  // int numElements = input.readArrayLength();
  //   or
  // recipientDSIds = input.readIntArray();
  //   caused the same exception
  //   apache::geode::client::Exception: int length should have been 4
  //
  int numElements = input.readNativeInt32();
  for (int i = 0; i < numElements; i++) {
    recipientDSIds.push_back(input.readInt32());
  }
}

DSFid GatewaySenderEventCallbackArgument::getDSFID() const {
  return DSFid::GatewaySenderEventCallbackArgument;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
