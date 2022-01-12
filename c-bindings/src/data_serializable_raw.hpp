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

#include <vector>

namespace apache {
namespace geode {
namespace client {
namespace internal {

class DataSerializableRaw : public DataSerializablePrimitive , CacheableKey {
 public:
  DataSerializableRaw(const int8_t* data, size_t size);
  ~DataSerializableRaw() noexcept override = default;

  static std::shared_ptr<DataSerializableRaw> create(
      const int8_t* data, size_t size);


  virtual void toData(DataOutput& dataOutput) const;
  virtual void fromData(DataInput& dataInput);
  virtual DSCode getDsCode() const;

  bool operator==(const CacheableKey& other) const;

  int32_t hashcode() const;

  private:
   std::vector<int8_t> bytes_;
   static constexpr int dsCodeSize_ = 1;
   mutable int32_t hashCode_;
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache
