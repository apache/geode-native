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

#ifndef GEODE_EVENTIDMAP_H_
#define GEODE_EVENTIDMAP_H_

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <geode/internal/functional.hpp>

#include "EventId.hpp"
#include "EventSource.hpp"

namespace apache {
namespace geode {
namespace client {

class EventSequence;
class EventIdMap;

typedef std::pair<std::shared_ptr<EventSource>, std::shared_ptr<EventSequence>>
    EventIdMapEntry;
typedef std::vector<EventIdMapEntry> EventIdMapEntryList;

/** @class EventIdMap EventIdMap.hpp
 *
 * This is the class that encapsulates a HashMap and
 * provides the operations for duplicate checking and
 * expiry of idle event IDs from notifications.
 */
class EventIdMap {
 private:
  typedef std::unordered_map<std::shared_ptr<EventSource>,
                             std::shared_ptr<EventSequence>,
                             dereference_hash<std::shared_ptr<EventSource>>,
                             dereference_equal_to<std::shared_ptr<EventSource>>>
      map_type;

  std::chrono::milliseconds m_expiry;
  map_type m_map;
  std::recursive_mutex m_lock;

  // hidden
  EventIdMap(const EventIdMap &);
  EventIdMap &operator=(const EventIdMap &);

 public:
  EventIdMap() : m_expiry(0) {}

  void clear();

  /** Initialize with preset expiration time in seconds */
  void init(std::chrono::milliseconds expirySecs);

  ~EventIdMap();

  /** Find out if entry is duplicate
   * @return true if the entry exists else false
   */
  bool isDuplicate(std::shared_ptr<EventSource> key,
                   std::shared_ptr<EventSequence> value);

  /** Construct an EventIdMapEntry from an std::shared_ptr<EventId> */
  static EventIdMapEntry make(std::shared_ptr<EventId> eventid);

  /** Put an item and return true if it is new or false if it existed and was
   * updated
   * @param onlynew Only put if the sequence id does not exist or is higher
   * @return true if the entry was updated or inserted otherwise false
   */
  bool put(std::shared_ptr<EventSource> key,
           std::shared_ptr<EventSequence> value, bool onlynew = false);

  /** Update the deadline for the entry
   * @return true if the entry exists else false
   */
  bool touch(std::shared_ptr<EventSource> key);

  /** Remove an item from the map
   *  @return true if the entry was found and removed else return false
   */
  bool remove(std::shared_ptr<EventSource> key);

  /** Collect all map entries who acked flag is false and set their acked flags
   * to true */
  EventIdMapEntryList getUnAcked();

  /** Clear all acked flags in the list and return the number of entries cleared
   * @param entries List of entries whos flags are to be cleared
   * @return The number of entries whos flags were cleared
   */
  uint32_t clearAckedFlags(EventIdMapEntryList &entries);

  /** Remove entries whos deadlines have passed and return the number of entries
   * removed
   * @param onlyacked Either check only entries whos acked flag is true
   * otherwise check all entries
   * @return The number of entries removed
   */
  uint32_t expire(bool onlyacked);
};

/** @class EventSequence
 *
 * EventSequence is the combination of SequenceNum from EventId, a timestamp and
 * a flag indicating whether or not it is ACKed
 */
class EventSequence {
 public:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;

 private:
  int64_t m_seqNum;
  bool m_acked;
  time_point m_deadline;  // current time plus the expiration delay (age)

  void init();

 public:
  void clear();

  EventSequence();
  explicit EventSequence(int64_t seqNum);
  ~EventSequence();

  // update deadline
  void touch(std::chrono::milliseconds ageSecs);
  // update deadline, clear acked flag and set seqNum
  void touch(int64_t seqNum, std::chrono::milliseconds ageSecs);

  // Accessors:

  int64_t getSeqNum();
  void setSeqNum(int64_t seqNum);

  bool getAcked();
  void setAcked(bool acked);

  time_point getDeadline();
  void setDeadline(time_point deadline);

  bool operator<=(const EventSequence &rhs) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache
#endif  // GEODE_EVENTIDMAP_H_
