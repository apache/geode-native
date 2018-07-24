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

#include "Order.hpp"

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace customserializable {

void Order::fromData(DataInput& dataInput) {
  order_id_ = static_cast<uint32_t>(dataInput.readInt32());
  name_ = dataInput.readString();
  quantity_ = static_cast<uint16_t>(dataInput.readInt16());
}

void Order::toData(DataOutput& dataOutput) const {
  dataOutput.writeInt(order_id_);
  dataOutput.writeString(name_);
  dataOutput.writeInt(quantity_);
}

std::string Order::toString() const {
  return "OrderID: " + std::to_string(order_id_) + " Product Name: " + name_ +
         " Quantity: " + std::to_string(quantity_);
}

size_t Order::objectSize() const {
  auto objectSize = sizeof(Order);
  objectSize += name_.capacity();
  return objectSize;
}

std::shared_ptr<DataSerializable> Order::createDeserializable() {
  return std::make_shared<Order>();
}

int32_t Order::getClassId() const {
  // No longer used by the interface
  return 7;
}

}  // namespace customserializable
