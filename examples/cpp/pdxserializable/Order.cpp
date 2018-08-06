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
#include <sstream>

#include "Order.hpp"

#include <geode/PdxReader.hpp>
#include <geode/PdxWriter.hpp>

namespace customserializable {

void Order::fromData(PdxReader& pdxReader) {
  order_id_ = static_cast<uint32_t>(pdxReader.readLong(ORDER_ID_KEY_));
  name_ = pdxReader.readString(NAME_KEY_);
  quantity_ = static_cast<uint16_t>(pdxReader.readInt(QUANTITY_KEY_));
}

void Order::toData(PdxWriter& pdxWriter) const {
  pdxWriter.writeLong(ORDER_ID_KEY_, order_id_);
  pdxWriter.markIdentityField(ORDER_ID_KEY_);

  pdxWriter.writeString(NAME_KEY_, name_);
  pdxWriter.markIdentityField(NAME_KEY_);

  pdxWriter.writeInt(QUANTITY_KEY_, quantity_);
  pdxWriter.markIdentityField(QUANTITY_KEY_);
}

std::string Order::toString() const {
  std::stringstream strm;

  strm << "OrderID: " << order_id_ << " Product Name: " << name_ << " Quantity: " << quantity_;
  return strm.str();
}

size_t Order::objectSize() const {
  auto objectSize = sizeof(Order);
  objectSize += name_.capacity();
  return objectSize;
}

const std::string& Order::getClassName() const {
  static const std::string CLASS_NAME = "com.example.Order";
  return CLASS_NAME;
}

std::shared_ptr<PdxSerializable> Order::create() {
  return std::make_shared<Order>();
}

const std::string Order::ORDER_ID_KEY_ = "order_id";
const std::string Order::NAME_KEY_ = "name";
const std::string Order::QUANTITY_KEY_ = "quantity";

}  // namespace customserializable
