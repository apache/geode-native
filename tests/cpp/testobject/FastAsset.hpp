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

#ifndef GEODE_TESTOBJECT_FASTASSET_H_
#define GEODE_TESTOBJECT_FASTASSET_H_

/*
 * @brief User class for testing the query functionality.
 */

#include <string>

#include "TimestampedObject.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;

class TESTOBJECT_EXPORT FastAsset : public TimestampedObject {
 private:
  int32_t assetId;
  double value;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  FastAsset() : assetId(0), value(0) {}
  FastAsset(int size, int maxVal);
  ~FastAsset() override = default;
  void toData(DataOutput& output) const override;
  void fromData(DataInput& input) override;

  size_t objectSize() const override {
    auto objectSize = sizeof(FastAsset);
    return objectSize;
  }

  /**
   * Returns the id of the asset.
   */
  int getAssetId() { return assetId; }

  /**
   * Returns the asset value.
   */
  double getValue() { return value; }

  /**
   * Sets the asset value.
   */
  void setValue(double d) { value = d; }

  int getIndex() { return assetId; }
  /**
   * Makes a copy of this asset.
   */
  std::shared_ptr<FastAsset> copy() {
    auto asset = std::make_shared<FastAsset>();
    asset->setAssetId(getAssetId());
    asset->setValue(getValue());
    return asset;
  }
  /**
   * Sets the id of the asset.
   */
  void setAssetId(int i) { assetId = i; }

  std::string toString() const override {
    return std::string("FastAsset:[assetId = ") + std::to_string(assetId) +
           " value = " + std::to_string(value) + "]";
  }

  static apache::geode::client::Serializable* createDeserializable() {
    return new FastAsset();
  }
};

}  // namespace testobject

#endif  // GEODE_TESTOBJECT_FASTASSET_H_
