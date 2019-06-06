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

#ifndef GEODE_EVENTID_H_
#define GEODE_EVENTID_H_

#include <string>

#include <geode/DataOutput.hpp>
#include <geode/internal/DataSerializableFixedId.hpp>
#include <geode/internal/geode_globals.hpp>

/** @file
 */

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

/**
 * EventID "token" with a Distributed Member ID, Thread ID and per-thread
 * Sequence ID
 */
class APACHE_GEODE_EXPORT EventId
    : public internal::DataSerializableFixedId_t<DSFid::EventId> {
 private:
  char m_eidMem[512];
  int32_t m_eidMemLen;
  int64_t m_eidThr;
  int64_t m_eidSeq;
  int32_t m_bucketId;
  int8_t m_breadcrumbCounter;

 public:
  /**
   *@brief Accessor methods
   **/
  const char* getMemId() const;
  int32_t getMemIdLen() const;
  int64_t getThrId() const;
  int64_t getSeqNum() const;

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  size_t objectSize() const override;

  /**
   * @brief creation function for strings.
   */
  static std::shared_ptr<Serializable> createDeserializable();

  /** Returns a pointer to a new eventid value. */
  static std::shared_ptr<EventId> create(char* memId, uint32_t memIdLen,
                                         int64_t thr, int64_t seq);

  /** Destructor. */
  ~EventId() override = default;

  int64_t getEventIdData(DataInput& input, char numberCode);

  void writeIdsData(DataOutput& output);

  /** Constructor, given the values. */
  EventId(char* memId, uint32_t memIdLen, int64_t thr, int64_t seq);
  /** Constructor, used for deserialization. */
  EventId(bool doInit = true, uint32_t reserveSize = 0,
          bool fullValueAfterDeltaFail = false);  // set init=false if we dont
                                                  // want to inc sequence (for
                                                  // de-serialization)

 protected:
  void initFromTSS();  // init from TSS and increment per-thread sequence number
  void initFromTSS_SameThreadIdAndSameSequenceId();  // for after delta message
                                                     // fail, and server is
                                                     // looking for full value.
                                                     // So eventiD should be
                                                     // same

 private:
  // never implemented.
  void operator=(const EventId& other);
  EventId(const EventId& other);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EVENTID_H_
