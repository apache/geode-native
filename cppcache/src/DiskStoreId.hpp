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

#ifndef GEODE_DISKSTOREID_H_
#define GEODE_DISKSTOREID_H_

#include <geode/geode_globals.hpp>
#include <geode/DataInput.hpp>
#include "DSMemberForVersionStamp.hpp"
#include "GeodeTypeIdsImpl.hpp"

namespace apache {
namespace geode {
namespace client {

class DiskStoreId : public DSMemberForVersionStamp {
 public:
  DiskStoreId() : m_hashCode(""), m_mostSig(0), m_leastSig(0) {}

  /**
   * for internal testing
   */
  DiskStoreId(int64_t mostSig, int64_t leastSig)
      : m_hashCode(""), m_mostSig(mostSig), m_leastSig(leastSig) {}

  DiskStoreId(const DiskStoreId& rhs)
      : m_mostSig(rhs.m_mostSig), m_leastSig(rhs.m_leastSig) {}

  DiskStoreId& operator=(const DiskStoreId& rhs) {
    if (this == &rhs) return *this;
    this->m_leastSig = rhs.m_leastSig;
    this->m_mostSig = rhs.m_mostSig;
    return *this;
  }

  virtual void toData(DataOutput& output) const {
    throw IllegalStateException("DiskStoreId::toData not implemented");
  }
  virtual void fromData(DataInput& input) {
    m_mostSig = input.readInt64();
    m_leastSig = input.readInt64();
  }
  virtual int32_t classId() const { return 0; }

  virtual int8_t typeId() const {
    return static_cast<int8_t>(GeodeTypeIdsImpl::DiskStoreId);
  }

  virtual int16_t compareTo(const DSMemberForVersionStamp& tagID) const {
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
  static Serializable* createDeserializable() { return new DiskStoreId(); }
  std::string getHashKey();

  virtual int32_t hashcode() const {
    static uint32_t prime = 31;
    uint32_t result = 1;
    result =
        prime * result + static_cast<uint32_t>(m_leastSig ^ (m_leastSig >> 32));
    result =
        prime * result + static_cast<uint32_t>(m_mostSig ^ (m_mostSig >> 32));
    return result;
  }

  virtual bool operator==(const CacheableKey& other) const {
    return (this->compareTo(
                dynamic_cast<const DSMemberForVersionStamp&>(other)) == 0);
  }

 private:
  std::string m_hashCode;
  int64_t m_mostSig;
  int64_t m_leastSig;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DISKSTOREID_H_
