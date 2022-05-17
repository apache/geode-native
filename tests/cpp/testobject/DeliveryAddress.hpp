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

#pragma once

#ifndef GEODE_TESTOBJECT_DELIVERYADDRESS_H_
#define GEODE_TESTOBJECT_DELIVERYADDRESS_H_

#include <sstream>
#include <util/Log.hpp>

#include <geode/CacheableEnum.hpp>
#include <geode/CacheableObjectArray.hpp>
#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace PdxTests {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableVector;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;
using apache::geode::client::Serializable;

class TESTOBJECT_EXPORT DeliveryAddress : public PdxSerializable {
 public:
  enum {
    VERSION_1 = 1,
    VERSION_2 = 2,
  };

 public:
  DeliveryAddress();

  DeliveryAddress(std::string addressLine, std::string city,
                  std::string country, std::string instructions,
                  std::shared_ptr<CacheableVector> numbers);

  std::string toString() const override;

  bool operator==(const CacheableKey& o) const override;

  void toData(PdxWriter& writer) const override;

  void fromData(PdxReader& reader) override;

  const std::string& getClassName() const override;

  const std::string& getAddressLine() const {
    return addressLine_;
  }

  const std::string& getCity() const {
    return city_;
  }

  const std::string& getCountry() const {
    return country_;
  }

  const std::string& getInstructions() const {
    return instructions_;
  }

 public:
  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<DeliveryAddress>();
  }

  static void setSerializationVersion(int32_t version) { version_ = version; }

 private:
  // Added in version 1
  std::string addressLine_;
  std::string city_;
  std::string country_;

  // Added in version 2
  std::string instructions_;
  std::shared_ptr<CacheableVector> phoneNumbers_;

 private:
  static int32_t version_;
};

}  // namespace PdxTests

#endif  // GEODE_TESTOBJECT_DELIVERYADDRESS_H_
