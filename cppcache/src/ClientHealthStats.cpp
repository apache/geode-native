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

#include "ClientHealthStats.hpp"

#include <geode/CacheableDate.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace apache {
namespace geode {
namespace client {

void ClientHealthStats::toData(DataOutput& output) const {
  output.writeInt(static_cast<int64_t>(gets_));
  output.writeInt(static_cast<int64_t>(puts_));
  output.writeInt(static_cast<int64_t>(misses_));
  output.writeInt(static_cast<int32_t>(cacheListenerCallsCompleted_));
  output.writeInt(static_cast<int32_t>(threads_));
  output.writeInt(static_cast<int32_t>(cpus_));
  output.writeInt(static_cast<int64_t>(processCpuTime_));
  updateTime_->toData(output);
}

void ClientHealthStats::fromData(DataInput& input) {
  gets_ = input.readInt64();
  puts_ = input.readInt64();
  misses_ = input.readInt64();
  cacheListenerCallsCompleted_ = input.readInt32();
  threads_ = input.readInt32();
  processCpuTime_ = input.readInt64();
  cpus_ = input.readInt32();
  updateTime_->fromData(input);
}

std::shared_ptr<Serializable> ClientHealthStats::createDeserializable() {
  return std::make_shared<ClientHealthStats>();
}

ClientHealthStats::ClientHealthStats()
    : ClientHealthStats(0, 0, 0, 0, 0, 0, 0) {}

ClientHealthStats::ClientHealthStats(int64_t gets, int64_t puts, int64_t misses,
                                     int32_t cacheListenerCallsCompleted,
                                     int32_t threads, int64_t processCpuTime,
                                     int32_t cpus)
    : gets_(gets),
      puts_(puts),
      misses_(misses),
      cacheListenerCallsCompleted_(cacheListenerCallsCompleted),
      threads_(threads),
      processCpuTime_(processCpuTime),
      cpus_(cpus) {
  updateTime_ = CacheableDate::create();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
