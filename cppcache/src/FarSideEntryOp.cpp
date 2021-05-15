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

#include "FarSideEntryOp.hpp"

#include "ClientProxyMembershipID.hpp"
#include "DiskVersionTag.hpp"
#include "TcrMessage.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

FarSideEntryOp::FarSideEntryOp(
    MemberListForVersionStamp& memberListForVersionStamp)
    : m_op(Operation::MARKER),
      m_modSerialNum(0),
      m_eventOffset(0),
      m_didDestroy(false),
      m_memberListForVersionStamp(memberListForVersionStamp)

{}

bool FarSideEntryOp::isDestroy(Operation op) {
  return op == Operation::DESTROY || op == Operation::LOCAL_DESTROY ||
         op == Operation::EVICT_DESTROY || op == Operation::EXPIRE_DESTROY ||
         op == Operation::EXPIRE_LOCAL_DESTROY || op == Operation::REMOVE;
}

bool FarSideEntryOp::isInvalidate(Operation op) {
  return op == Operation::INVALIDATE || op == Operation::LOCAL_INVALIDATE ||
         op == Operation::EXPIRE_INVALIDATE ||
         op == Operation::EXPIRE_LOCAL_INVALIDATE;
}

void FarSideEntryOp::fromData(DataInput& input, bool largeModCount,
                              uint16_t memId) {
  m_key = std::dynamic_pointer_cast<CacheableKey>(input.readObject());
  m_op = static_cast<Operation>(input.read());
  if (largeModCount) {
    m_modSerialNum = input.readInt32();
  } else {
    m_modSerialNum = input.read();
  }

  m_callbackArg = input.readObject();

  skipFilterRoutingInfo(input);

  m_versionTag =
      TcrMessage::readVersionTagPart(input, memId, m_memberListForVersionStamp);
  // std::shared_ptr<Serializable> sPtr;
  // input.readObject(sPtr);
  m_eventOffset = input.readInt32();
  if (!isDestroy(m_op)) {
    m_didDestroy = input.readBoolean();
    if (!isInvalidate(m_op)) {
      if (input.readBoolean()) {
        int32_t rewind = 1;
        int16_t fixedId = 0;
        if (input.read() == static_cast<int8_t>(DSCode::FixedIDShort)) {
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
        input.readArrayLength();  // ignore len
        input.readObject(m_value);

        // input.readBytes(&buf, &len);
        // m_value = CacheableBytes::create(buf, len);
      }
    }
  }
}

void FarSideEntryOp::apply(std::shared_ptr<Region>& region) {
  // LocalRegion* localRegion = static_cast<LocalRegion*>(region.get());
  // localRegion->acquireReadLock();

  auto ri = std::static_pointer_cast<RegionInternal>(region);
  if (isDestroy(m_op)) {
    ri->txDestroy(m_key, m_callbackArg, m_versionTag);
  } else if (isInvalidate(m_op)) {
    ri->txInvalidate(m_key, m_callbackArg, m_versionTag);
  } else {
    ri->txPut(m_key, m_value, m_callbackArg, m_versionTag);
  }
}

void FarSideEntryOp::skipFilterRoutingInfo(DataInput& input) {
  std::shared_ptr<Cacheable> tmp;
  auto structType =
      static_cast<DSCode>(input.read());  // this is DataSerializable (45)

  if (structType == DSCode::NullObj) {
    return;
  } else if (structType == DSCode::DataSerializable) {
    input.read();        // ignore classbyte
    input.readObject();  // ignore object
    int32_t size = input.readInt32();
    for (int i = 0; i < size; i++) {
      // ignore ClientProxyMembershipID
      ClientProxyMembershipID memId;
      memId.readEssentialData(input);

      // Ignore filter info
      if (input.readBoolean()) {
        auto len = input.readArrayLength();
        for (int j = 0; j < len; j++) {
          input.readUnsignedVL();
          input.readUnsignedVL();
        }
      }

      // ignore interestedClients
      auto len = input.readInt32();
      if (len != -1) {
        const auto isLong = input.readBoolean();
        input.advanceCursor(len * (isLong ? sizeof(int64_t) : sizeof(int32_t)));
      }

      // ignore interestedClientsInv
      len = input.readInt32();
      if (len != -1) {
        const auto isLong = input.readBoolean();
        input.advanceCursor(len * (isLong ? sizeof(int64_t) : sizeof(int32_t)));
      }
    }
  } else {
    LOG_ERROR(
        "FarSideEntryOp::skipFilterRoutingInfo Unexpected type id: %d while "
        "desirializing commit response",
        static_cast<int32_t>(structType));
    GfErrTypeThrowException(
        "FarSideEntryOp::skipFilterRoutingInfo Unable to handle commit "
        "response",
        GF_CACHE_ILLEGAL_STATE_EXCEPTION);
  }
}
/* std::shared_ptr<EntryEvent> FarSideEntryOp::getEntryEvent(Cache* cache)
{
        return std::shared_ptr<EntryEvent>(new EntryEvent(
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
