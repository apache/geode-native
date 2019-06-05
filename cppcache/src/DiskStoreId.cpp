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

#include "DiskStoreId.hpp"

namespace apache {
namespace geode {
namespace client {

std::string DiskStoreId::getHashKey() {
  if (m_hashCode.empty()) {
    m_hashCode.append(std::to_string(m_mostSig))
        .append("_")
        .append(std::to_string(m_leastSig));
  }
  return m_hashCode;
}

DiskStoreId::DiskStoreId() : m_hashCode(""), m_mostSig(0), m_leastSig(0) {}

  DiskStoreId::DiskStoreId(int64_t mostSig, int64_t leastSig)
      : m_hashCode(""), m_mostSig(mostSig), m_leastSig(leastSig) {}

  DiskStoreId::DiskStoreId(const DiskStoreId& rhs)
      : m_mostSig(rhs.m_mostSig), m_leastSig(rhs.m_leastSig) {}

  DiskStoreId& DiskStoreId::operator=(const DiskStoreId& rhs) {
    if (this == &rhs) return *this;
    m_leastSig = rhs.m_leastSig;
    m_mostSig = rhs.m_mostSig;
    return *this;
  }

  void DiskStoreId::toData(DataOutput&) const {
    throw IllegalStateException("DiskStoreId::toData not implemented");
  }

  void DiskStoreId::fromData(DataInput& input) {
    m_mostSig = input.readInt64();
    m_leastSig = input.readInt64();
  }

  DSFid DiskStoreId::getDSFID() const { return DSFid::DiskStoreId; }

  int16_t DiskStoreId::compareTo(const DSMemberForVersionStamp& tagID) const {
    const DiskStoreId& otherDiskStoreId =
        static_cast<const DiskStoreId&>(tagID);
    int64_t result = m_mostSig - otherDiskStoreId.m_mostSig;
    if (result == 0) {
      result = m_leastSig - otherDiskStoreId.m_leastSig;
    }
    if (result < 0) {
      return -1;
    } else if (result > 0) {
      return 1;
    } else {
      return 0;
    }
  }
  std::shared_ptr<Serializable> DiskStoreId::createDeserializable() {
    return std::make_shared<DiskStoreId>();
  }

  int32_t DiskStoreId::hashcode() const {
    static uint32_t prime = 31;
    uint32_t result = 1;
    result =
        prime * result + static_cast<uint32_t>(m_leastSig ^ (m_leastSig >> 32));
    result =
        prime * result + static_cast<uint32_t>(m_mostSig ^ (m_mostSig >> 32));
    return result;
  }

  bool DiskStoreId::operator==(const CacheableKey& other) const {
    return (compareTo(
                dynamic_cast<const DSMemberForVersionStamp&>(other)) == 0);
  }

}  // namespace client
}  // namespace geode
}  // namespace apache
