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
#include "EventId.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>

#include <geode/DataInput.hpp>

#include "ClientProxyMembershipID.hpp"
#include "Utils.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

class ThreadIdCounter {
 public:
  static std::atomic<int64_t>& instance() {
    static std::atomic<int64_t> threadId_(0);
    return threadId_;
  }

  static int64_t next() { return ++instance(); }
};

class EventIdTSS {
 public:
  static EventIdTSS& instance() {
    thread_local EventIdTSS eventId_;
    return eventId_;
  }

  int64_t nextSequenceId() {
    sequenceId_++;
    return sequenceId_;
  }

  int64_t currentSequenceId() { return sequenceId_; }

  int64_t threadId() { return threadId_; }

 private:
  EventIdTSS();

  int64_t threadId_;
  int64_t sequenceId_;
};

EventIdTSS::EventIdTSS() : threadId_(ThreadIdCounter::next()), sequenceId_(0) {
  LOG_DEBUG("EventIdTSS::EventIdTSS({}): threadId_={}, sequenceId_={}",
            static_cast<const void*>(this), threadId_, sequenceId_);
}

void EventId::toData(DataOutput& output) const {
  //  This method is always expected to write out nonstatic distributed
  //  memberid.  Note that binary representation of EventId is NOT THE
  //  SAME here as when serialized into part of a message (via the writeIdsData
  //  method).
  LOG_DEBUG("EventId::toData({}) - called", static_cast<const void*>(this));
  output.writeBytes(reinterpret_cast<const int8_t*>(clientId_),
                    clientIdLength_);
  output.writeArrayLen(18);
  char longCode = 3;
  output.write(static_cast<uint8_t>(longCode));
  output.writeInt(threadId_);
  output.write(static_cast<uint8_t>(longCode));
  output.writeInt(sequenceId_);
  output.writeInt(bucketId_);
  output.write(breadcrumbCounter_);
}

void EventId::fromData(DataInput& input) {
  LOG_DEBUG("EventId::fromData({}) - called", static_cast<void*>(this));
  clientIdLength_ = input.readArrayLength();
  input.readBytesOnly(reinterpret_cast<int8_t*>(clientId_), clientIdLength_);
  input.readArrayLength();
  threadId_ = getEventIdData(input, input.read());
  sequenceId_ = getEventIdData(input, input.read());
  bucketId_ = input.readInt32();
  breadcrumbCounter_ = input.read();
}

const char* EventId::clientId() const { return clientId_; }

int32_t EventId::clientIdLength() const { return clientIdLength_; }

int64_t EventId::threadId() const { return threadId_; }

int64_t EventId::sequenceNumber() const { return sequenceId_; }

int64_t EventId::getEventIdData(DataInput& input, char numberCode) {
  int64_t retVal = 0;
  LOG_DEBUG("EventId::getEventIdData({}) - called", static_cast<void*>(this));

  //  Read number based on numeric code written by java server.
  if (numberCode == 0) {
    return input.read();
  } else if (numberCode == 1) {
    retVal = input.readInt16();
  } else if (numberCode == 2) {
    int32_t intVal;
    intVal = input.readInt32();
    retVal = intVal;
  } else if (numberCode == 3) {
    int64_t longVal;
    longVal = input.readInt64();
    retVal = longVal;
  }

  return retVal;
}

std::shared_ptr<Serializable> EventId::createDeserializable() {
  LOG_DEBUG("EventId::createDeserializable - called");
  // use false since we dont want to inc sequence
  // (for de-serialization)
  return std::make_shared<EventId>(false);
}

EventId::EventId(char* memId, uint32_t memIdLen, int64_t thr, int64_t seq) {
  LOG_DEBUG("EventId::EventId({}) - memId={}, memIdLen={}, thr={}, seq={}",
            static_cast<void*>(this),
            Utils::convertBytesToString(memId, memIdLen).c_str(), memIdLen, thr,
            seq);
  // TODO: statics being assigned; not thread-safe??
  std::memcpy(clientId_, memId, memIdLen);
  clientIdLength_ = memIdLen;
  threadId_ = thr;
  sequenceId_ = seq;
  bucketId_ = -1;
  breadcrumbCounter_ = 0;
}

EventId::EventId(bool doInit, uint32_t reserveSize,
                 bool fullValueAfterDeltaFail)
    : /* adongre
       * CID 28934: Uninitialized scalar field (UNINIT_CTOR)
       */
      clientIdLength_(0),
      threadId_(0),
      sequenceId_(0),
      bucketId_(-1),
      breadcrumbCounter_(0) {
  LOG_DEBUG(
      "EventId::EventId({}) - doInit={}, reserveSize={}, "
      "fullValueAfterDeltaFail={}",
      static_cast<void*>(this), doInit ? "true" : "false", reserveSize,
      fullValueAfterDeltaFail ? "true" : "false");
  if (!doInit) return;

  if (fullValueAfterDeltaFail) {
    /// need to send old sequence id
    initFromTSS_SameThreadIdAndSameSequenceId();
  } else {
    initFromTSS();
  }

  for (uint32_t i = 0; i < reserveSize; i++) {
    EventIdTSS::instance().nextSequenceId();
  }
}

void EventId::initFromTSS() {
  threadId_ = EventIdTSS::instance().threadId();
  sequenceId_ = EventIdTSS::instance().nextSequenceId();
  LOG_DEBUG("EventId::initFromTSS({}) - called, tid={}, seqid={}",
            static_cast<void*>(this), threadId_, sequenceId_);
}

void EventId::initFromTSS_SameThreadIdAndSameSequenceId() {
  threadId_ = EventIdTSS::instance().threadId();
  sequenceId_ = EventIdTSS::instance().currentSequenceId();
  LOG_DEBUG(
      "EventId::initFromTSS_SameThreadIdAndSameSequenceId({}) - called, "
      "tid={}, seqid={}",
      static_cast<void*>(this), threadId_, sequenceId_);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
