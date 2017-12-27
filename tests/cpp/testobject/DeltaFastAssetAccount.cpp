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

#include "DeltaFastAssetAccount.hpp"

using namespace apache::geode::client;
using namespace testframework;
using namespace testobject;

DeltaFastAssetAccount::DeltaFastAssetAccount(int index, bool encodeTimestp,
                                             int maxVal, int asstSize,
                                             bool getbfrUpdate)
    : Delta(nullptr), encodeTimestamp(encodeTimestp), acctId(index) {
  customerName = CacheableString::create("Milton Moneybags");
  netWorth = 0.0;
  assets = CacheableHashMap::create();
  for (int i = 0; i < asstSize; i++) {
    auto asset = std::make_shared<FastAsset>(i, maxVal);
    assets->emplace(CacheableInt32::create(i), asset);
    netWorth += asset->getValue();
  }
  if (encodeTimestamp) {
    ACE_Time_Value startTime;
    startTime = ACE_OS::gettimeofday();
    ACE_UINT64 tusec = 0;
    startTime.to_usec(tusec);
    timestamp = tusec * 1000;
  }
  getBeforeUpdate = getbfrUpdate;
}

void DeltaFastAssetAccount::toData(
    apache::geode::client::DataOutput& output) const {
  output.writeInt(static_cast<int32_t>(acctId));
  output.writeObject(customerName);
  output.writeDouble(netWorth);
  output.writeObject(assets);
  output.writeInt(static_cast<int64_t>(timestamp));
}

void DeltaFastAssetAccount::fromData(apache::geode::client::DataInput& input) {
  acctId = input.readInt32();
  customerName = input.readObject<CacheableString>();
  netWorth = input.readDouble();
  assets = input.readObject<CacheableHashMap>();
  timestamp = input.readInt64();
}

void DeltaFastAssetAccount::toDelta(
    apache::geode::client::DataOutput& output) const {
  output.writeDouble(netWorth);
  if (encodeTimestamp) {
    output.writeInt(static_cast<int64_t>(timestamp));
  }
}

void DeltaFastAssetAccount::fromDelta(apache::geode::client::DataInput& input) {
  if (getBeforeUpdate) {
    netWorth = input.readDouble();
  } else {
    double netWorthTemp;
    netWorthTemp = input.readDouble();
    netWorth += netWorthTemp;
  }
  if (encodeTimestamp) {
    timestamp = input.readInt64();
  }
}
