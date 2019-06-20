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

#include <geode/DataInput.hpp>
#include <geode/internal/geode_globals.hpp>

#include "DSMemberForVersionStamp.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

class DiskStoreId : public DSMemberForVersionStamp {
 public:
  DiskStoreId();

  /**
   * for internal testing
   */
  DiskStoreId(int64_t mostSig, int64_t leastSig);

  DiskStoreId(const DiskStoreId& rhs);

  DiskStoreId& operator=(const DiskStoreId& rhs);

  void toData(DataOutput&) const override;

  void fromData(DataInput& input) override;

  DSFid getDSFID() const override;

  int16_t compareTo(const DSMemberForVersionStamp& tagID) const override;
  static std::shared_ptr<Serializable> createDeserializable();

  std::string getHashKey() override;

  int32_t hashcode() const override;

  bool operator==(const CacheableKey& other) const override;

 private:
  std::string m_hashCode;
  int64_t m_mostSig;
  int64_t m_leastSig;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DISKSTOREID_H_
