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

#ifndef GEODE_VERSIONSTAMP_H_
#define GEODE_VERSIONSTAMP_H_

#include <memory>

#include <geode/internal/geode_globals.hpp>

#include "ClientProxyMembershipID.hpp"
#include "ErrType.hpp"
#include "VersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @brief This class encapsulates Version Stamp for map entries.
 */
class VersionStamp {
 public:
  VersionStamp()
      : m_memberID(0),
        m_entryVersionHighByte(0),
        m_entryVersionLowBytes(0),
        m_regionVersionHighBytes(0),
        m_regionVersionLowBytes(0) {}

  VersionStamp(const VersionStamp& rhs)
      : m_memberID(rhs.m_memberID),
        m_entryVersionHighByte(rhs.m_entryVersionHighByte),
        m_entryVersionLowBytes(rhs.m_entryVersionLowBytes),
        m_regionVersionHighBytes(rhs.m_regionVersionHighBytes),
        m_regionVersionLowBytes(rhs.m_regionVersionLowBytes) {}

  virtual ~VersionStamp() noexcept = default;
  void setVersions(std::shared_ptr<VersionTag> versionTag);
  void setVersions(VersionStamp& versionStamp);
  int32_t getEntryVersion() const;
  int64_t getRegionVersion() const;
  uint16_t getMemberId() const;

  VersionStamp& operator=(const VersionStamp& rhs) {
    if (this == &rhs) return *this;
    this->m_memberID = rhs.m_memberID;
    this->m_entryVersionHighByte = rhs.m_entryVersionHighByte;
    this->m_entryVersionLowBytes = rhs.m_entryVersionLowBytes;
    this->m_regionVersionHighBytes = rhs.m_regionVersionHighBytes;
    this->m_regionVersionLowBytes = rhs.m_regionVersionLowBytes;
    return *this;
  }
  GfErrType processVersionTag(const RegionInternal* region,
                              const std::shared_ptr<CacheableKey>& keyPtr,
                              const std::shared_ptr<VersionTag>& tag,
                              const bool deltaCheck) const;

 private:
  uint16_t m_memberID;
  uint8_t m_entryVersionHighByte;
  uint16_t m_entryVersionLowBytes;
  uint16_t m_regionVersionHighBytes;
  uint32_t m_regionVersionLowBytes;
  GfErrType checkForConflict(const RegionInternal* region,
                             const std::string& keystr,
                             const std::shared_ptr<VersionTag>& tag,
                             const bool deltaCheck) const;
  GfErrType checkForDeltaConflict(const RegionInternal* region,
                                  const std::string& keystr,
                                  const int64_t stampVersion,
                                  const int64_t tagVersion,
                                  const std::shared_ptr<VersionTag>& tag) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_VERSIONSTAMP_H_
