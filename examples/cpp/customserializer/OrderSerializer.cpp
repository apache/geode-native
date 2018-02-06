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

#include "OrderSerializer.hpp"
#include "Order.hpp"

namespace customserializer {

std::shared_ptr<void> OrderSerializer::fromData(const std::string& className,
                                                PdxReader& pdxReader) {
  if (className != OrderSerializer::CLASS_NAME_) {
    return nullptr;
  }

  try {
    auto myOrder =
        std::make_shared<Order>((uint32_t)pdxReader.readLong(ORDER_ID_KEY_),
                                pdxReader.readString(NAME_KEY_),
                                (uint16_t)pdxReader.readInt(QUANTITY_KEY_));
    return myOrder;
  } catch (std::exception& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
    return nullptr;
  }
}

bool OrderSerializer::toData(const std::shared_ptr<const void>& userObject,
                             const std::string& className,
                             PdxWriter& pdxWriter) {
  if (className != OrderSerializer::CLASS_NAME_) {
    return false;
  }

  try {
    auto myOrder = std::static_pointer_cast<const Order>(userObject);
    pdxWriter.writeLong(ORDER_ID_KEY_, myOrder->getOrderID());
    pdxWriter.writeString(NAME_KEY_, myOrder->getName());
    pdxWriter.writeInt(QUANTITY_KEY_, myOrder->getQuantity());
  } catch (std::exception& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
    return false;
  }
  return true;
}

const std::string OrderSerializer::CLASS_NAME_ = "com.example.Order";
const std::string OrderSerializer::ORDER_ID_KEY_ = "orderid";
const std::string OrderSerializer::NAME_KEY_ = "name";
const std::string OrderSerializer::QUANTITY_KEY_ = "quantity";
}  // namespace customserializer
