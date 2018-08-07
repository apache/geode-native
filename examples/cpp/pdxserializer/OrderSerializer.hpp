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

#ifndef CUSTOMSERIALIZER_ORDERSERIALIZER_H
#define CUSTOMSERIALIZER_ORDERSERIALIZER_H

#include <geode/PdxSerializer.hpp>

using namespace apache::geode::client;

namespace customserializer {

class OrderSerializer : public PdxSerializer {
 public:
  static const std::string CLASS_NAME_;

  OrderSerializer() = default;

  ~OrderSerializer() override = default;

  std::shared_ptr<void> fromData(const std::string& className,
                                 PdxReader& pdxReader) override;

  bool toData(const std::shared_ptr<const void>& userObject,
              const std::string& className, PdxWriter& pdxWriter) override;

 private:
  static const std::string ORDER_ID_KEY_;
  static const std::string NAME_KEY_;
  static const std::string QUANTITY_KEY_;
};
}  // namespace customserializer

#endif  // CUSTOMSERIALIZER_ORDERSERIALIZER_H
