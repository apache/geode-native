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

#ifndef GEODE_CLIENTHEALTHSTATS_H_
#define GEODE_CLIENTHEALTHSTATS_H_

#include <geode/internal/DataSerializableFixedId.hpp>

namespace apache {
namespace geode {
namespace client {

class CacheableDate;

class ClientHealthStats : public internal::DataSerializableFixedId_t<
                              internal::DSFid::ClientHealthStats> {
 public:
  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  static std::shared_ptr<Serializable> createDeserializable();

  size_t objectSize() const override { return sizeof(ClientHealthStats); }

  /**
   * Factory method for creating an instance of ClientHealthStats
   */
  static std::shared_ptr<ClientHealthStats> create(
      int64_t gets, int64_t puts, int64_t misses,
      int32_t cacheListenerCallsCompleted, int32_t threads,
      int64_t processCpuTime = 0, int32_t cpus = 0) {
    return std::make_shared<ClientHealthStats>(gets, puts, misses,
                                               cacheListenerCallsCompleted,
                                               threads, processCpuTime, cpus);
  }

  ~ClientHealthStats() noexcept override = default;

  ClientHealthStats();

  ClientHealthStats(int64_t gets, int64_t puts, int64_t misses,
                    int32_t cacheListenerCallsCompleted, int32_t threads,
                    int64_t processCpuTime, int32_t cpus);

 private:
  int64_t gets_;
  int64_t puts_;
  int64_t misses_;
  int32_t cacheListenerCallsCompleted_;
  int32_t threads_;
  int64_t processCpuTime_;
  int32_t cpus_;
  std::shared_ptr<CacheableDate> updateTime_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTHEALTHSTATS_H_
