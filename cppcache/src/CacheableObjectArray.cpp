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
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/PdxSerializable.hpp>

namespace apache {
namespace geode {
namespace client {

void CacheableObjectArray::toData(DataOutput& output) const {
  int32_t len = static_cast<int32_t>(size());
  output.writeArrayLen(len);
  output.write(static_cast<int8_t>(DSCode::Class));
  output.writeString(getClassName());

  for (const auto& iter : *this) {
    output.writeObject(iter);
  }
}

void CacheableObjectArray::fromData(DataInput& input) {
  int32_t len = input.readArrayLength();
  if (len >= 0) {
    input.read();        // ignore CLASS typeid
    input.readString();  // ignore class name
    std::shared_ptr<Cacheable> obj;
    for (int32_t index = 0; index < len; index++) {
      input.readObject(obj);
      push_back(obj);
    }
  }
}

size_t CacheableObjectArray::objectSize() const {
  auto size = sizeof(CacheableObjectArray);
  for (const auto& iter : *this) {
    size += iter->objectSize();
  }
  return size;
}

std::string CacheableObjectArray::getClassName() const {
  if (!empty()) {
    auto&& item = *begin();

    if (auto pdx = dynamic_cast<PdxSerializable*>(item.get())) {
      return pdx->getClassName();
    }
  }

  return "java.lang.Object";
}
}  // namespace client
}  // namespace geode
}  // namespace apache
