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

#include "DeliveryAddress.hpp"

namespace PdxTests {
DeliveryAddress::DeliveryAddress()
    : addressLine_{}, city_{}, country_{}, instructions_{}, phoneNumbers_{} {}

DeliveryAddress::DeliveryAddress(std::string addressLine, std::string city,
                                 std::string country, std::string instructions,
                                 std::shared_ptr<CacheableVector> numbers)
    : addressLine_{addressLine},
      city_{city},
      country_{country},
      instructions_{instructions},
      phoneNumbers_{numbers} {}

std::string DeliveryAddress::toString() const {
  return "DeliveryAddress[Version=" + std::to_string(version_) +
         "; Address=" + addressLine_ + "; City=" + city_ +
         "; Country=" + country_ + "; Instructions=" + instructions_ +
         "; PhoneNumbers.isNull=" + (!phoneNumbers_ ? "true" : "false") + ']';
}

bool DeliveryAddress::operator==(const CacheableKey& o) const {
  if (this == &o) {
    return true;
  }

  auto other = dynamic_cast<const DeliveryAddress*>(&o);
  if (other == nullptr) {
    return false;
  }

  return addressLine_ == other->addressLine_ && city_ == other->city_ &&
         country_ == other->country_ && instructions_ == other->instructions_ &&
         !phoneNumbers_ == !other->phoneNumbers_;
}

void DeliveryAddress::toData(PdxWriter& writer) const {
  writer.writeString("address", addressLine_);
  writer.writeString("city", city_);
  writer.writeString("country", country_);

  if (version_ == VERSION_2) {
    writer.writeString("instructions", instructions_);
    writer.writeObject("phoneNumbers", phoneNumbers_);
  }
}

void DeliveryAddress::fromData(PdxReader& reader) {
  addressLine_ = reader.readString("address");
  city_ = reader.readString("city");
  country_ = reader.readString("country");

  if (version_ == VERSION_2) {
    instructions_ = reader.readString("instructions");
    phoneNumbers_ = std::dynamic_pointer_cast<CacheableVector>(
        reader.readObject("phoneNumbers"));
  }
}

const std::string& DeliveryAddress::getClassName() const {
  static std::string className = "PdxTests.DeliveryAddress";
  return className;
}

}  // namespace PdxTests
