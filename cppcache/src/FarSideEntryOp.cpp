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
/*
 * FarSideEntryOp.cpp
 *
 *  Created on: 22-Feb-2011
 *      Author: ankurs
 */

#include "FarSideEntryOp.hpp"
#include "RegionCommit.hpp"
#include "ClientProxyMembershipID.hpp"
#include "DiskVersionTag.hpp"

namespace apache {
namespace geode {
namespace client {

FarSideEntryOp::FarSideEntryOp(
    RegionCommit* region, MemberListForVersionStamp& memberListForVersionStamp)
    :  // UNUSED m_region(region),
       /* adongre
        *
        */
      m_op(0),
      m_modSerialNum(0),
      m_eventOffset(0),
      m_didDestroy(false),
      m_memberListForVersionStamp(memberListForVersionStamp)

{}

FarSideEntryOp::~FarSideEntryOp() {}

bool FarSideEntryOp::isDestroy(int8_t op) {
  return op == DESTROY || op == LOCAL_DESTROY || op == EVICT_DESTROY ||
         op == EXPIRE_DESTROY || op == EXPIRE_LOCAL_DESTROY || op == REMOVE;
}

bool FarSideEntryOp::isInvalidate(int8_t op) {
  return op == INVALIDATE || op == LOCAL_INVALIDATE ||
         op == EXPIRE_INVALIDATE || op == EXPIRE_LOCAL_INVALIDATE;
}

void FarSideEntryOp::fromData(DataInput& input, bool largeModCount,
                              uint16_t memId) {
  m_key = input.readObject<CacheableKey>();
  m_op = input.read();
  if (largeModCount) {
    m_modSerialNum = input.readInt32();
  } else {
    m_modSerialNum = input.read();
  }

  if (input.read() != GeodeTypeIds::NullObj) {
    input.rewindCursor(1);
    input.readObject(m_callbackArg);
  }

  skipFilterRoutingInfo(input);
  m_versionTag =
      TcrMessage::readVersionTagPart(input, memId, m_memberListForVersionStamp);
  // SerializablePtr sPtr;
  // input.readObject(sPtr);
  m_eventOffset = input.readInt32();
  if (!isDestroy(m_op)) {
    m_didDestroy = input.readBoolean();
    if (!isInvalidate(m_op)) {
      if (input.readBoolean()) {
        int32_t rewind = 1;
        int16_t fixedId = 0;
        if (input.read() == GeodeTypeIdsImpl::FixedIDShort) {
          fixedId = input.readInt16();
          rewind += 2;
        }

        // TOKEN_INVALID = 141;
        // TOKEN_LOCAL_INVALID = 142;
        // TOKEN_DESTROYED = 43;
        // TOKEN_REMOVED = 144;
        // TOKEN_REMOVED2 = 145;
        if (fixedId >= 141 && fixedId < 146) {
          m_value = nullptr;
        } else {
          input.rewindCursor(rewind);
          input.readObject(m_value);
        }
      } else {
        // uint8_t* buf = nullptr;
        input.readArrayLen(); // ignore len
        input.readObject(m_value);

        // input.readBytes(&buf, &len);
        // m_value = CacheableBytes::create(buf, len);
      }
    }
  }
}

void FarSideEntryOp::apply(RegionPtr& region) {
  // LocalRegion* localRegion = static_cast<LocalRegion*>(region.get());
  // localRegion->acquireReadLock();

  RegionInternalPtr ri = std::static_pointer_cast<RegionInternal>(region);
  if (isDestroy(m_op)) {
    ri->txDestroy(m_key, m_callbackArg, m_versionTag);
  } else if (isInvalidate(m_op)) {
    ri->txInvalidate(m_key, m_callbackArg, m_versionTag);
  } else {
    ri->txPut(m_key, m_value, m_callbackArg, m_versionTag);
  }
}

void FarSideEntryOp::skipFilterRoutingInfo(DataInput& input) {
  CacheablePtr tmp;
  auto structType = input.read();  // this is DataSerializable (45)

  if (structType == GeodeTypeIds::NullObj) {
    return;
  } else if (structType == GeodeTypeIdsImpl::DataSerializable) {
    input.read(); // ignore classbyte
    input.readObject(tmp);
    int32_t size = input.readInt32();
    for (int i = 0; i < size; i++) {
      ClientProxyMembershipID memId;
      // memId.fromData(input);
      memId.readEssentialData(input);

      int32_t len = input.readArrayLen();

      if (input.readBoolean()) {
        len = input.readArrayLen();
        for (int j = 0; j < len; j++) {
          input.readUnsignedVL();
          input.readUnsignedVL();
        }
      }

      len = input.readInt32();
      if (len != -1) {
        const auto isLong = input.readBoolean();

        for (int j = 0; j < len; j++) {
          if (isLong) {
            input.readInt64();
          } else {
            input.readInt32();
          }
        }
      }

      len = input.readInt32();
      if (len != -1) {
        const auto isLong = input.readBoolean();

        for (int j = 0; j < len; j++) {
          if (isLong) {
            input.readInt64();
          } else {
            input.readInt32();
          }
        }
      }
    }
  } else {
    LOGERROR(
        "FarSideEntryOp::skipFilterRoutingInfo Unexpected type id: %d while "
        "desirializing commit response",
        structType);
    GfErrTypeThrowException(
        "FarSideEntryOp::skipFilterRoutingInfo Unable to handle commit "
        "response",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }
}
/*
EntryEventPtr FarSideEntryOp::getEntryEvent(Cache* cache)
{
        return EntryEventPtr(new EntryEvent(
                        m_region->getRegion(cache),
                        m_key,
                        nullptr,
                        m_value,
                        m_callbackArg,
                        false));
}
*/
}  // namespace client
}  // namespace geode
}  // namespace apache
