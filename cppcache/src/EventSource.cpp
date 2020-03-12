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

#include "EventIdMap.hpp"

namespace apache {
namespace geode {
namespace client {

EventSource::EventSource(const char* memId, int32_t memIdLen, int64_t thrId) {
  init();

  if (memId == nullptr || memIdLen <= 0) {
    return;
  }

  m_thrId = thrId;

  m_srcIdLen = memIdLen + sizeof(thrId);  // 8; // sizeof(thrId or int64_t);
  m_srcId = new char[m_srcIdLen];
  memcpy(m_srcId, memId, memIdLen);

  // convert the int64 thrId to a byte-array and place at the end of m_srcId
  memcpy(m_srcId + memIdLen, &thrId, sizeof(thrId));
}

EventSource::~EventSource() { clear(); }

void EventSource::init() {
  m_srcId = nullptr;
  m_srcIdLen = 0;
  m_hash = 0;
  m_thrId = -1;
}

void EventSource::clear() {
  delete[] m_srcId;
  init();
}

char* EventSource::getSrcId() { return m_srcId; }
const char* EventSource::getSrcId() const { return m_srcId; }

int32_t EventSource::getSrcIdLen() { return m_srcIdLen; }
int32_t EventSource::getSrcIdLen() const { return m_srcIdLen; }

char* EventSource::getMemId() { return m_srcId; }

int32_t EventSource::getMemIdLen() { return m_srcIdLen - sizeof(m_thrId); }

int64_t EventSource::getThrId() { return m_thrId; }

int32_t EventSource::hashcode() const {
  return static_cast<int32_t>(std::hash<EventSource>{}(*this));
}

bool EventSource::operator==(const EventSource& rhs) const {
  if (this->m_srcId == nullptr || (&rhs)->m_srcId == nullptr ||
      this->m_srcIdLen != (&rhs)->m_srcIdLen) {
    return false;
  }

  return memcmp(this->m_srcId, (&rhs)->m_srcId, this->m_srcIdLen) == 0;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
