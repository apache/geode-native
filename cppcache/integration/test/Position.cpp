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
#include "Position.hpp"

#include <wchar.h>

#include <cwchar>

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace DataSerializableTest {

int32_t Position::cnt = 0;

Position::Position()
    : avg20DaysVol(0),
      convRatio(0.0),
      valueGain(0.0),
      industry(0),
      issuer(0),
      mktValue(0.0),
      qty(0.0),
      sharesOutstanding(0),
      volatility(0),
      pid(0) {}

Position::Position(std::string id, int32_t outstandingShares) : Position() {
  secId = std::move(id);
  secType = "a";
  sharesOutstanding = outstandingShares;
  qty = outstandingShares - (cnt % 2 == 0 ? 1000 : 100);
  mktValue = qty * 1.2345998;
  pid = cnt++;
}

void Position::toData(apache::geode::client::DataOutput& output) const {
  output.writeInt(avg20DaysVol);
  output.writeString(bondRating);
  output.writeDouble(convRatio);
  output.writeString(country);
  output.writeDouble(valueGain);
  output.writeInt(industry);
  output.writeInt(issuer);
  output.writeDouble(mktValue);
  output.writeDouble(qty);
  output.writeString(secId);
  output.writeString(secLinks);
  output.writeUTF(secType);
  output.writeInt(sharesOutstanding);
  output.writeString(underlyer);
  output.writeInt(volatility);
  output.writeInt(pid);
}

void Position::fromData(apache::geode::client::DataInput& input) {
  avg20DaysVol = input.readInt64();
  bondRating = input.readString();
  convRatio = input.readDouble();
  country = input.readString();
  valueGain = input.readDouble();
  industry = input.readInt64();
  issuer = input.readInt64();
  mktValue = input.readDouble();
  qty = input.readDouble();
  secId = input.readString();
  secLinks = input.readString();
  secType = input.readUTF();
  sharesOutstanding = input.readInt32();
  underlyer = input.readString();
  volatility = input.readInt64();
  pid = input.readInt32();
}

}  // namespace DataSerializableTest
