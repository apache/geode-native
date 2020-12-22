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

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace DataSerializableTest {

int32_t Position::count = 0;

Position::Position()
    : volumeAverageOver20Days_(0),
      conversionRatio_(0.0),
      valueGain_(0.0),
      industry_(0),
      issuer_(0),
      marketValue_(0.0),
      quantity_(0.0),
      sharesOutstanding_(0),
      volatility_(0),
      positionId_(0) {}

Position::Position(std::string id, int32_t outstandingShares) : Position() {
  securityId_ = std::move(id);
  securityType_ = "a";
  sharesOutstanding_ = outstandingShares;
  quantity_ = outstandingShares - (count % 2 == 0 ? 1000 : 100);
  marketValue_ = quantity_ * 1.2345998;
  positionId_ = count++;
}

void Position::toData(apache::geode::client::DataOutput& output) const {
  output.writeInt(volumeAverageOver20Days_);
  output.writeString(bondRating_);
  output.writeDouble(conversionRatio_);
  output.writeString(country_);
  output.writeDouble(valueGain_);
  output.writeInt(industry_);
  output.writeInt(issuer_);
  output.writeDouble(marketValue_);
  output.writeDouble(quantity_);
  output.writeString(securityId_);
  output.writeString(securityLinks_);
  output.writeUTF(securityType_);
  output.writeInt(sharesOutstanding_);
  output.writeString(underlyingSecurity_);
  output.writeInt(volatility_);
  output.writeInt(positionId_);
}

void Position::fromData(apache::geode::client::DataInput& input) {
  volumeAverageOver20Days_ = input.readInt64();
  bondRating_ = input.readString();
  conversionRatio_ = input.readDouble();
  country_ = input.readString();
  valueGain_ = input.readDouble();
  industry_ = input.readInt64();
  issuer_ = input.readInt64();
  marketValue_ = input.readDouble();
  quantity_ = input.readDouble();
  securityId_ = input.readString();
  securityLinks_ = input.readString();
  securityType_ = input.readUTF();
  sharesOutstanding_ = input.readInt32();
  underlyingSecurity_ = input.readString();
  volatility_ = input.readInt64();
  positionId_ = input.readInt32();
}

}  // namespace DataSerializableTest
