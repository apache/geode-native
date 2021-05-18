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

#ifndef GEODE_EVENTSOURCE_H_
#define GEODE_EVENTSOURCE_H_

#include <functional>
#include <memory>

namespace apache {
namespace geode {
namespace client {

/** @class EventSource
 *
 * EventSource is the combination of MembershipId and ThreadId from the EventId
 */
class EventSource {
  char *m_srcId;
  int32_t m_srcIdLen;
  int64_t m_thrId;

  mutable uint32_t m_hash;

  void init();

  // hide copy ctor and assignment operator
  EventSource();
  EventSource(const EventSource &);
  EventSource &operator=(const EventSource &);

 public:
  void clear();

  EventSource(const char *memId, int32_t memIdLen, int64_t thrId);
  ~EventSource();

  int32_t hashcode() const;
  bool operator==(const EventSource &rhs) const;

  // Accessors

  char *getSrcId();
  const char *getSrcId() const;
  int32_t getSrcIdLen();
  int32_t getSrcIdLen() const;
  char *getMemId();
  int32_t getMemIdLen();
  int64_t getThrId();

  struct hash {
    inline std::size_t operator()(const EventSource &val) const {
      return val.hashcode();
    }

    inline std::size_t operator()(const EventSource *val) const {
      return val->hashcode();
    }

    inline std::size_t operator()(
        const std::shared_ptr<EventSource> &val) const {
      return val->hashcode();
    }
  };

  struct equal_to {
    inline bool operator()(const EventSource &lhs,
                           const EventSource &rhs) const {
      return lhs == rhs;
    }

    inline bool operator()(const EventSource *lhs,
                           const EventSource *rhs) const {
      return (*lhs) == (*rhs);
    }

    inline bool operator()(const std::shared_ptr<EventSource> &lhs,
                           const std::shared_ptr<EventSource> &rhs) const {
      return (*lhs) == (*rhs);
    }
  };
};

}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::EventSource> {
  typedef apache::geode::client::EventSource argument_type;
  typedef size_t result_type;
  size_t operator()(const apache::geode::client::EventSource &val) const {
    return std::hash<std::string>{}(
        std::string(val.getSrcId(), val.getSrcIdLen()));
  }
};

}  // namespace std
#endif  // GEODE_EVENTSOURCE_H_
