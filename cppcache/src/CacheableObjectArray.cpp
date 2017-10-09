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
#include <geode/CacheableObjectArray.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/GeodeTypeIds.hpp>
#include <GeodeTypeIdsImpl.hpp>

namespace apache {
namespace geode {
namespace client {

void CacheableObjectArray::toData(DataOutput& output) const {
  int32_t len = static_cast<int32_t>(size());
  output.writeArrayLen(len);
  output.write(static_cast<int8_t>(GeodeTypeIdsImpl::Class));
  output.write(static_cast<int8_t>(GeodeTypeIds::CacheableASCIIString));
  output.writeASCII("java.lang.Object");
  for (const auto& iter : *this) {
    output.writeObject(iter);
  }
}

void CacheableObjectArray::fromData(DataInput& input) {
  int32_t len = input.readArrayLen();
  if (len >= 0) {
    input.read();  // ignore CLASS typeid
    input.read();  // ignore string typeid
    uint16_t classLen = input.readInt16();
    input.advanceCursor(classLen);
    CacheablePtr obj;
    for (int32_t index = 0; index < len; index++) {
      input.readObject(obj);
      push_back(obj);
    }
  }
}

int32_t CacheableObjectArray::classId() const { return 0; }

int8_t CacheableObjectArray::typeId() const {
  return GeodeTypeIds::CacheableObjectArray;
}

uint32_t CacheableObjectArray::objectSize() const {
  uint32_t size = sizeof(CacheableObjectArray);
  for (const auto& iter : *this) {
    size += iter->objectSize();
  }
  return size;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
