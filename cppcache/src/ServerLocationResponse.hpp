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

#ifndef GEODE_SERVERLOCATIONRESPONSE_H_
#define GEODE_SERVERLOCATIONRESPONSE_H_

#include <geode/Serializable.hpp>
#include "GeodeTypeIdsImpl.hpp"

namespace apache {
namespace geode {
namespace client {

class ServerLocationResponse : public Serializable {
 public:
  ServerLocationResponse() : Serializable() {}
  void toData(DataOutput& output) const override {}
  void fromData(DataInput& input) override = 0;
  int32_t classId() const override { return 0; }
  int8_t typeId() const override = 0;
  int8_t DSFID() const override {
    return static_cast<int8_t>(GeodeTypeIdsImpl::FixedIDByte);
  }
  size_t objectSize() const override = 0;
  ~ServerLocationResponse() override = default;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERVERLOCATIONRESPONSE_H_
