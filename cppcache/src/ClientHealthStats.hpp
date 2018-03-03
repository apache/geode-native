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

#include <geode/Serializable.hpp>
#include <geode/CacheableDate.hpp>

#include "util/Log.hpp"
#include "GeodeTypeIdsImpl.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientHealthStats
    : public DataSerializableFixedId_t<GeodeTypeIdsImpl::ClientHealthStats> {
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
  static std::shared_ptr<ClientHealthStats> create(int gets, int puts,
                                                   int misses, int listCalls,
                                                   int numThreads,
                                                   int64_t cpuTime = 0,
                                                   int cpus = 0) {
    return std::shared_ptr<ClientHealthStats>(new ClientHealthStats(
        gets, puts, misses, listCalls, numThreads, cpuTime, cpus));
  }
  ~ClientHealthStats() override = default;

 private:
  ClientHealthStats(int gets, int puts, int misses, int listCalls,
                    int numThreads, int64_t cpuTime, int cpus);
  ClientHealthStats();

  int m_numGets;                // CachePerfStats.gets
  int m_numPuts;                // CachePerfStats.puts
  int m_numMisses;              // CachePerfStats.misses
  int m_numCacheListenerCalls;  // CachePerfStats.cacheListenerCallsCompleted
  int m_numThread;              // ProcessStats.threads;
  int64_t m_processCpuTime;     //
  int m_cpus;
  std::shared_ptr<CacheableDate> m_updateTime;  // Last updateTime

  _GEODE_FRIEND_STD_SHARED_PTR(ClientHealthStats)
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTHEALTHSTATS_H_
