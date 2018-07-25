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

#ifndef CUSTOMSERIALIZABLE_ORDER_H
#define CUSTOMSERIALIZABLE_ORDER_H

#include <string>

#include <geode/PdxSerializable.hpp>

namespace customserializable {

using namespace apache::geode::client;

class Order : public PdxSerializable {
 public:
  inline Order() : Order(0, "", 0) {}

  inline Order(uint32_t order_id, std::string name, uint16_t quantity)
      : order_id_(order_id), name_(std::move(name)), quantity_(quantity) {}

  ~Order() override = default;

  inline uint32_t getOrderId() const { return order_id_; }

  inline const std::string& getName() const { return name_; }

  inline uint16_t getQuantity() const { return quantity_; }

  using PdxSerializable::fromData;

  using PdxSerializable::toData;

  void fromData(PdxReader& pdxReader) override;

  void toData(PdxWriter& pdxWriter) const override;

  std::string toString() const override;

  size_t objectSize() const override;

  const std::string& getClassName() const override;

  static std::shared_ptr<PdxSerializable> create();

 private:
  static const std::string ORDER_ID_KEY_;
  static const std::string NAME_KEY_;
  static const std::string QUANTITY_KEY_;

  uint32_t order_id_;
  std::string name_;
  uint16_t quantity_;
};

}  // namespace customserializable

#endif  // CUSTOMSERIALIZABLE_ORDER_H
