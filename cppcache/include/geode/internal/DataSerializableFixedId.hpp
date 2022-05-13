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

#ifndef GEODE_DATASERIALIZABLEFIXEDID_H_
#define GEODE_DATASERIALIZABLEFIXEDID_H_

#include "../Serializable.hpp"
#include "DSFixedId.hpp"

namespace apache {
namespace geode {
namespace client {

class DataOutput;
class DataInput;

namespace internal {

class APACHE_GEODE_EXPORT DataSerializableFixedId
    : public virtual Serializable {
 public:
  ~DataSerializableFixedId() noexcept override = default;

  virtual void toData(DataOutput& dataOutput) const = 0;

  virtual void fromData(DataInput& dataInput) = 0;

  virtual DSFid getDSFID() const = 0;
};

template <DSFid _DSFID>
class APACHE_GEODE_EXPORT DataSerializableFixedId_t
    : public DataSerializableFixedId {
 public:
  ~DataSerializableFixedId_t() noexcept override = default;

  DSFid getDSFID() const final { return _DSFID; }
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DATASERIALIZABLEFIXEDID_H_
