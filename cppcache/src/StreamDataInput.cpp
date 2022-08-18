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

constexpr size_t kBufferSize = 3000;

StreamDataInput::StreamDataInput(std::chrono::milliseconds timeout,
                                 std::unique_ptr<Connector> connector,
                                 const CacheImpl* cache, Pool* pool)
    : DataInput(nullptr, 0, cache, pool),
      connector_(std::move(connector)),
      remainingTimeBeforeTimeout_(timeout) {}

void StreamDataInput::readDataIfNotAvailable(size_t size) {
  char buff[kBufferSize];
  while (getBytesRemaining() < size) {
    const auto start = std::chrono::system_clock::now();

    const auto receivedLength = connector_->receive_nothrowiftimeout(
        buff, kBufferSize, remainingTimeBeforeTimeout_);

    const auto timeSpent = std::chrono::system_clock::now() - start;

    remainingTimeBeforeTimeout_ -=
        std::chrono::duration_cast<decltype(remainingTimeBeforeTimeout_)>(
            timeSpent);

    LOGDEBUG(
        "received %d bytes from %s: %s, time spent: "
        "%ld millisecs, time remaining before timeout: %ld millisecs",
        receivedLength, connector_->getRemoteEndpoint().c_str(),
        Utils::convertBytesToString(reinterpret_cast<uint8_t*>(buff),
                                    receivedLength)
            .c_str(),
        std::chrono::duration_cast<std::chrono::milliseconds>(timeSpent)
            .count(),
        remainingTimeBeforeTimeout_.count());

    if (remainingTimeBeforeTimeout_ <= std::chrono::milliseconds::zero()) {
      throw(TimeoutException(std::string("Timeout when receiving from ")
                                 .append(connector_->getRemoteEndpoint())));
    }

    auto newLength = bufferLength_ + receivedLength;
    auto currentPosition = getBytesRead();
    streamBuf_.resize(newLength);
    memcpy(streamBuf_.data() + bufferLength_, buff, receivedLength);

    bufferHead_ = streamBuf_.data();
    buffer_ = bufferHead_ + currentPosition;
    bufferLength_ = newLength;
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
