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

#include <geode/CacheableDate.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/DataSerializableFixedId.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientHealthStats : public internal::DataSerializableFixedId_t<
                              internal::DSFid::ClientHealthStats> {
 public:
  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  /**
   * @brief creation function for dates.
   */
  static std::shared_ptr<Serializable> createDeserializable();

  /** @return the size of the object in bytes */
  size_t objectSize() const override { return sizeof(ClientHealthStats); }

  /**
   * Factory method for creating an instance of ClientHealthStats
   */
  static std::shared_ptr<ClientHealthStats> create(
      int64_t gets, int64_t puts, int64_t misses, int32_t listCalls,
      int32_t numThreads, int64_t cpuTime = 0, int32_t cpus = 0) {
    return std::shared_ptr<ClientHealthStats>(new ClientHealthStats(
        gets, puts, misses, listCalls, numThreads, cpuTime, cpus));
  }

  ~ClientHealthStats() override = default;

  ClientHealthStats();

 private:
  ClientHealthStats(int64_t gets, int64_t puts, int64_t misses,
                    int32_t listCalls, int32_t numThreads, int64_t cpuTime,
                    int32_t cpus);

  int64_t m_numGets;    // CachePerfStats.gets
  int64_t m_numPuts;    // CachePerfStats.puts
  int64_t m_numMisses;  // CachePerfStats.misses
  int32_t
      m_numCacheListenerCalls;  // CachePerfStats.cacheListenerCallsCompleted
  int32_t m_numThread;          // ProcessStats.threads;
  int64_t m_processCpuTime;
  int32_t m_cpus;
  std::shared_ptr<CacheableDate> m_updateTime;  // Last updateTime
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTHEALTHSTATS_H_
