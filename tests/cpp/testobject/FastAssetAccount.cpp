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

#include "FastAssetAccount.hpp"

#include <cinttypes>

#include "FastAsset.hpp"

namespace testobject {

using apache::geode::client::CacheableBytes;
using apache::geode::client::CacheableInt32;

FastAssetAccount::FastAssetAccount(int idx, bool encodeTimestp, int maxVal,
                                   int asstSize)
    : encodeTimestamp(encodeTimestp), acctId(idx) {
  // encodeTimestamp = encodeTimestp;
  // acctId = idx;
  customerName = CacheableString::create("Milton Moneybags");
  netWorth = 0.0;
  assets = CacheableHashMap::create();
  for (int i = 0; i < asstSize; i++) {
    auto asset = std::make_shared<FastAsset>(i, maxVal);
    assets->emplace(CacheableInt32::create(i), asset);
    netWorth += asset->getValue();
  }

  if (encodeTimestamp) {
    timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  }
}

void FastAssetAccount::toData(apache::geode::client::DataOutput& output) const {
  output.writeInt(static_cast<int32_t>(acctId));
  output.writeObject(customerName);
  output.writeDouble(netWorth);
  output.writeObject(assets);
  output.writeInt(static_cast<int64_t>(timestamp));
}

void FastAssetAccount::fromData(apache::geode::client::DataInput& input) {
  acctId = input.readInt32();
  customerName = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  netWorth = input.readDouble();
  assets = std::dynamic_pointer_cast<CacheableHashMap>(input.readObject());
  timestamp = input.readInt64();
}

std::string FastAssetAccount::toString() const {
  char buf[102500];
  sprintf(buf,
          "FastAssetAccount:[acctId = %d customerName = %s netWorth = %f "
          "timestamp = %" PRIu64 "]",
          acctId, customerName->toString().c_str(), netWorth, timestamp);
  return buf;
}

}  // namespace testobject
