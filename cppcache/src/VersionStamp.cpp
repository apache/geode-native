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

#include "VersionStamp.hpp"

#include <string>

#include "CacheImpl.hpp"
#include "MemberListForVersionStamp.hpp"
#include "RegionInternal.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"

namespace apache {
namespace geode {
namespace client {

void VersionStamp::setVersions(std::shared_ptr<VersionTag> versionTag) {
  int32_t eVersion = versionTag->getEntryVersion();
  m_entryVersionLowBytes = static_cast<uint16_t>(eVersion & 0xffff);
  m_entryVersionHighByte = static_cast<uint8_t>((eVersion & 0xff0000) >> 16);
  m_regionVersionHighBytes = versionTag->getRegionVersionHighBytes();
  m_regionVersionLowBytes = versionTag->getRegionVersionLowBytes();
  m_memberID = versionTag->getInternalMemID();
}

void VersionStamp::setVersions(VersionStamp& versionStamp) {
  m_entryVersionLowBytes = versionStamp.m_entryVersionLowBytes;
  m_entryVersionHighByte = versionStamp.m_entryVersionHighByte;
  m_regionVersionHighBytes = versionStamp.m_regionVersionHighBytes;
  m_regionVersionLowBytes = versionStamp.m_regionVersionLowBytes;
  m_memberID = versionStamp.m_memberID;
}
int32_t VersionStamp::getEntryVersion() const {
  return (m_entryVersionHighByte << 16) | m_entryVersionLowBytes;
}

int64_t VersionStamp::getRegionVersion() const {
  return ((static_cast<int64_t>(m_regionVersionHighBytes)) << 32) |
         m_regionVersionLowBytes;
}

uint16_t VersionStamp::getMemberId() const { return m_memberID; }

// Processes version tag. Checks if there is a conflict with the existing
// version.
// Also checks if it is a delta update than it is based on the existing version.
// This is based on the basicprocessVersionTag function of
// AbstractRegionEntry.java
// Any change to the java function should be reflected here as well.
GfErrType VersionStamp::processVersionTag(
    const RegionInternal* region, const std::shared_ptr<CacheableKey>& keyPtr,
    const std::shared_ptr<VersionTag>& tag, const bool deltaCheck) const {
  if (nullptr == tag) {
    LOG_ERROR("Cannot process version tag as it is nullptr.");
    return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
  }

  return checkForConflict(region, keyPtr->toString().c_str(), tag, deltaCheck);
}

GfErrType VersionStamp::checkForConflict(const RegionInternal* region,
                                         const std::string& keystr,
                                         const std::shared_ptr<VersionTag>& tag,
                                         const bool deltaCheck) const {
  if (getEntryVersion() == 0 && getRegionVersion() == 0 && getMemberId() == 0) {
    LOG_DEBUG(
        "Version stamp on existing entry not found. applying change: key=%s",
        keystr.c_str());
    return GF_NOERR;
  }

  if (tag->getEntryVersion() == 0 && tag->getRegionVersionHighBytes() == 0 &&
      tag->getRegionVersionLowBytes() == 0 && tag->getInternalMemID() == 0) {
    LOG_DEBUG("Version Tag not available. applying change: key=%s",
              keystr.c_str());
    return GF_NOERR;
  }
  int64_t stampVersion = getEntryVersion() & 0xffffffffL;
  int64_t tagVersion = tag->getEntryVersion() & 0xffffffffL;
  auto memberList = region->getCacheImpl()->getMemberListForVersionStamp();
  bool apply = false;
  if (stampVersion != 0) {
    // check for int wraparound on the version number
    int64_t difference = tagVersion - stampVersion;
    if (0x10000 < difference || difference < -0x10000) {
      LOG_DEBUG("version rollover detected: key=%s tag=%lld stamp=%lld",
                keystr.c_str(), tagVersion, stampVersion);
      int64_t temp = 0x100000000LL;
      if (difference < 0) {
        tagVersion += temp;
      } else {
        stampVersion += temp;
      }
    }
  }

  if (deltaCheck) {
    auto err =
        checkForDeltaConflict(region, keystr, stampVersion, tagVersion, tag);
    if (err != GF_NOERR) return err;
  }

  if (stampVersion == 0 || stampVersion < tagVersion) {
    LOG_DEBUG("applying change: key=%s", keystr.c_str());
    apply = true;
  } else if (stampVersion > tagVersion) {
    LOG_DEBUG("disallowing change: key=%s", keystr.c_str());
  } else {
    // compare member IDs
    auto stampID = memberList->getDSMember(getMemberId());
    if (nullptr == stampID) {
      // This scenario is not possible. But added for just in case
      LOG_ERROR(
          "MemberId of the version stamp could not be found. Disallowing a "
          "possible inconsistent change: key=%s",
          keystr.c_str());
      // throw error
      return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
    }
    auto tagID = memberList->getDSMember(tag->getInternalMemID());
    if (nullptr == tagID) {
      // This scenario is not possible. But added for just in case
      LOG_ERROR(
          "MemberId of the version tag could not be found. Disallowing a "
          "possible inconsistent change. key=%s",
          keystr.c_str());
      // throw error
      return GF_CACHE_ILLEGAL_STATE_EXCEPTION;
    }
    if (!apply) {
      LOG_DEBUG(
          "comparing tagID %s with stampId %s for version comparison of key %s",
          tagID->getHashKey().c_str(), stampID->getHashKey().c_str(),
          keystr.c_str());
      int compare = stampID->compareTo(*tagID);
      if (compare < 0) {
        LOG_DEBUG("applying change: key=%s", keystr.c_str());
        apply = true;
      } else if (compare > 0) {
        LOG_DEBUG("disallowing change: key=%s", keystr.c_str());
      } else {
        LOG_DEBUG(
            "allowing the change as both the version tag and version stamp are "
            "same: key=%s",
            keystr.c_str());
        // This is required for local ops to succeed.
        apply = true;
      }
    }
  }

  if (!apply) {
    region->getCacheImpl()->getCachePerfStats().incConflatedEvents();
    return GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION;
  }
  return GF_NOERR;
}

GfErrType VersionStamp::checkForDeltaConflict(
    const RegionInternal* region, const std::string& keystr,
    const int64_t stampVersion, const int64_t tagVersion,
    const std::shared_ptr<VersionTag>& tag) const {
  auto memberList = region->getCacheImpl()->getMemberListForVersionStamp();
  auto tcRegion = dynamic_cast<const ThinClientRegion*>(region);
  ThinClientPoolDM* poolDM = nullptr;
  if (tcRegion) {
    poolDM = dynamic_cast<ThinClientPoolDM*>(tcRegion->getDistMgr());
  }

  if (tagVersion != stampVersion + 1) {
    LOG_DEBUG(
        "delta requires full value due to version mismatch. key=%s tagVersion "
        "%lld stampVersion %lld ",
        keystr.c_str(), tagVersion, stampVersion);
    if (poolDM) {
      poolDM->updateNotificationStats(false, std::chrono::nanoseconds(0));
    }
    return GF_INVALID_DELTA;

  } else {
    // make sure the tag was based on the value in this entry by checking the
    // tag's previous-changer ID against this stamp's current ID
    auto stampID = memberList->getDSMember(getMemberId());
    if (nullptr == stampID) {
      LOG_ERROR(
          "MemberId of the version stamp could not be found. Requesting full "
          "delta value. key=%s",
          keystr.c_str());
      if (poolDM) {
        poolDM->updateNotificationStats(false, std::chrono::nanoseconds(0));
      }
      return GF_INVALID_DELTA;
    }

    auto tagID = memberList->getDSMember(tag->getPreviousMemID());
    if (nullptr == tagID) {
      LOG_ERROR(
          "Previous MemberId of the version tag could not be found. Requesting "
          "full delta value. key=%s",
          keystr.c_str());
      if (poolDM) {
        poolDM->updateNotificationStats(false, std::chrono::nanoseconds(0));
      }
      return GF_INVALID_DELTA;
    }

    if (tagID->compareTo(*stampID) != 0) {
      LOG_DEBUG(
          "delta requires full value due to version mismatch. key=%s. \
        tag.previous=%s but stamp.current=%s",
          keystr.c_str(), tagID->getHashKey().c_str(),
          stampID->getHashKey().c_str());

      if (poolDM) {
        poolDM->updateNotificationStats(false, std::chrono::nanoseconds(0));
      }
      return GF_INVALID_DELTA;
    }
    return GF_NOERR;
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
