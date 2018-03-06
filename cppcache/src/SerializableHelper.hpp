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

#ifndef GEODE_SERIALIZABLEHELPER_H_
#define GEODE_SERIALIZABLEHELPER_H_

#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>

namespace apache {
namespace geode {
namespace client {

template <class _Serializable>
struct SerializableHelper {
  inline void serialize(DataOutput& dataOutput,
                        const _Serializable& serializable) {
    serializable.toData(dataOutput);
  };

  inline void deserialize(DataInput& dataInput, _Serializable& serializable) {
    serializable.fromData(dataInput);
  }

  inline bool metadataEqualTo(const _Serializable& lhs,
                              const _Serializable& rhs);

  inline bool equalTo(CacheImpl& cache, const _Serializable& lhs,
                      const _Serializable& rhs) {
    if (!metadataEqualTo(lhs, rhs)) {
      return false;
    }

    auto lhsOut = cache.createDataOutput();
    auto rhsOut = cache.createDataOutput();

    serialize(*lhsOut, lhs);
    serialize(*rhsOut, rhs);

    if (lhsOut->getBufferLength() != rhsOut->getBufferLength()) {
      return false;
    }

    return memcmp(lhsOut->getBuffer(), rhsOut->getBuffer(),
                  lhsOut->getBufferLength()) == 0;
  }

  inline bool equalTo(CacheImpl& cache,
                      const std::shared_ptr<_Serializable>& lhs,
                      const std::shared_ptr<_Serializable>& rhs) {
    if (rhs && lhs) {
      return equalTo(cache, *lhs, *rhs);
    }

    return false;
  }
};

template <>
inline bool SerializableHelper<DataSerializablePrimitive>::metadataEqualTo(
    const DataSerializablePrimitive& lhs,
    const DataSerializablePrimitive& rhs) {
  return lhs.getDsCode() == rhs.getDsCode();
}

template <>
inline bool SerializableHelper<DataSerializable>::metadataEqualTo(
    const DataSerializable& lhs, const DataSerializable& rhs) {
  return lhs.getClassId() == rhs.getClassId();
}

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERIALIZABLEHELPER_H_
