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

#include "StreamDataInput.hpp"

#include <geode/DataInput.hpp>

#include "Utils.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

const size_t BUFF_SIZE = 3000;

StreamDataInput::StreamDataInput(std::chrono::milliseconds timeout,
                                 std::unique_ptr<Connector> connector,
                                 const CacheImpl* cache, Pool* pool)
    : DataInput(nullptr, 0, cache, pool) {
  m_remainingTimeBeforeTimeout = timeout;
  m_connector = std::move(connector);
  m_buf = nullptr;
  m_bufHead = m_buf;
  m_bufLength = 0;
}

StreamDataInput::~StreamDataInput() {
  if (m_bufHead != nullptr) {
    free(const_cast<uint8_t*>(m_bufHead));
  }
}

void StreamDataInput::readDataIfNotAvailable(size_t size) {
  char buff[BUFF_SIZE];
  while ((m_bufLength - (m_buf - m_bufHead)) < size) {
    const auto start = std::chrono::system_clock::now();

    const auto receivedLength = m_connector->receive_nothrowiftimeout(
        buff, BUFF_SIZE,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            m_remainingTimeBeforeTimeout));

    const auto timeSpent = std::chrono::system_clock::now() - start;

    m_remainingTimeBeforeTimeout -=
        std::chrono::duration_cast<decltype(m_remainingTimeBeforeTimeout)>(
            timeSpent);

    LOGDEBUG(
        "received %d bytes from %s: %s, time spent: "
        "%ld microsecs, time remaining before timeout: %ld microsecs",
        receivedLength, m_connector->getRemoteEndpoint().c_str(),
        Utils::convertBytesToString(reinterpret_cast<uint8_t*>(buff),
                                    receivedLength)
            .c_str(),
        std::chrono::duration_cast<std::chrono::microseconds>(timeSpent)
            .count(),
        std::chrono::duration_cast<std::chrono::microseconds>(
            m_remainingTimeBeforeTimeout)
            .count());

    if (m_remainingTimeBeforeTimeout <= std::chrono::microseconds ::zero()) {
      throw(TimeoutException(std::string("Timeout when receiving from ")
                                 .append(m_connector->getRemoteEndpoint())));
    }

    size_t newLength = m_bufLength + receivedLength;
    size_t currentPosition = m_buf - m_bufHead;
    if ((m_bufHead) == nullptr) {
      m_bufHead = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * newLength));
    } else {
      m_bufHead = static_cast<uint8_t*>(
          realloc(const_cast<uint8_t*>(m_bufHead), newLength));
    }
    memcpy(const_cast<uint8_t*>(m_bufHead + m_bufLength), buff, receivedLength);
    m_buf = m_bufHead + currentPosition;
    m_bufLength += receivedLength;
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
