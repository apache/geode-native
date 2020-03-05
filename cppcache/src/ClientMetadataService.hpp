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

#ifndef GEODE_CLIENTMETADATASERVICE_H_
#define GEODE_CLIENTMETADATASERVICE_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>

#include <geode/CacheableKey.hpp>
#include <geode/Region.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/functional.hpp>

#include "AppDomainContext.hpp"
#include "BucketServerLocation.hpp"
#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

class ClientMetadata;
class ThinClientPoolDM;

typedef std::map<std::string, std::shared_ptr<ClientMetadata>>
    RegionMetadataMapType;

class BucketStatus {
 private:
  using clock = std::chrono::steady_clock;
  const static clock::time_point m_noTimeout;
  clock::time_point m_lastTimeout;

 public:
  BucketStatus() = default;
  bool isTimedoutAndReset(std::chrono::milliseconds millis) {
    if (m_lastTimeout == m_noTimeout) {
      return false;
    } else {
      auto timeout = m_lastTimeout + millis;
      if (timeout > clock::now()) {
        return true;  // timeout as buckste not recovered yet
      } else {
        // reset to zero as we waited enough to recover bucket
        m_lastTimeout = m_noTimeout;
        return false;
      }
    }
  }

  void setTimeout() {
    if (m_lastTimeout == m_noTimeout) {
      m_lastTimeout = clock::now();  // set once only for timeout
    }
  }
};

class PRbuckets {
 private:
  BucketStatus* m_buckets;

 public:
  explicit PRbuckets(int32_t nBuckets) {
    m_buckets = new BucketStatus[nBuckets];
  }
  ~PRbuckets() { delete[] m_buckets; }

  bool isBucketTimedOut(int32_t bucketId, std::chrono::milliseconds millis) {
    return m_buckets[bucketId].isTimedoutAndReset(millis);
  }

  void setBucketTimeout(int32_t bucketId) { m_buckets[bucketId].setTimeout(); }
};

class ClientMetadataService {
 public:
  ClientMetadataService(const ClientMetadataService&) = delete;
  ClientMetadataService& operator=(const ClientMetadataService&) = delete;
  ClientMetadataService() = delete;
  explicit ClientMetadataService(ThinClientPoolDM* pool);
  inline ~ClientMetadataService() noexcept = default;

  void start();

  void stop();

  void svc(void);

  void getClientPRMetadata(const char* regionFullPath);

  void getBucketServerLocation(
      const std::shared_ptr<Region>& region,
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, bool isPrimary,
      std::shared_ptr<BucketServerLocation>& serverLocation, int8_t& version);

  std::shared_ptr<ClientMetadata> getClientMetadata(
      const std::string& regionFullPath);

  void enqueueForMetadataRefresh(const std::string& regionFullPath,
                                 int8_t serverGroupFlag);

  typedef std::unordered_map<
      std::shared_ptr<BucketServerLocation>,
      std::shared_ptr<std::vector<std::shared_ptr<CacheableKey>>>,
      dereference_hash<std::shared_ptr<BucketServerLocation>>,
      dereference_equal_to<std::shared_ptr<BucketServerLocation>>>
      ServerToFilterMap;
  std::shared_ptr<ServerToFilterMap> getServerToFilterMap(
      const std::vector<std::shared_ptr<CacheableKey>>& keys,
      const std::shared_ptr<Region>& region, bool isPrimary);

  void markPrimaryBucketForTimeout(
      const std::shared_ptr<Region>& region,
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, bool isPrimary,
      std::shared_ptr<BucketServerLocation>& serverLocation, int8_t& version);

  void markPrimaryBucketForTimeoutButLookSecondaryBucket(
      const std::shared_ptr<Region>& region,
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, bool isPrimary,
      std::shared_ptr<BucketServerLocation>& serverLocation, int8_t& version);

  bool isBucketMarkedForTimeout(const char* regionFullPath, int32_t bucketid);

  typedef std::unordered_set<int32_t> BucketSet;
  typedef std::unordered_map<
      std::shared_ptr<BucketServerLocation>, std::shared_ptr<BucketSet>,
      dereference_hash<std::shared_ptr<BucketServerLocation>>,
      dereference_equal_to<std::shared_ptr<BucketServerLocation>>>
      ServerToBucketsMap;

  std::shared_ptr<BucketServerLocation> findNextServer(
      const ServerToBucketsMap& serverToBucketsMap,
      const BucketSet& currentBucketSet);

  typedef std::unordered_map<int32_t, std::shared_ptr<CacheableHashSet>>
      BucketToKeysMap;
  std::shared_ptr<BucketToKeysMap> groupByBucketOnClientSide(
      const std::shared_ptr<Region>& region,
      const std::shared_ptr<CacheableVector>& keySet,
      const std::shared_ptr<ClientMetadata>& metadata);

  typedef std::unordered_map<
      std::shared_ptr<BucketServerLocation>, std::shared_ptr<CacheableHashSet>,
      dereference_hash<std::shared_ptr<BucketServerLocation>>,
      dereference_equal_to<std::shared_ptr<BucketServerLocation>>>
      ServerToKeysMap;
  std::shared_ptr<ServerToKeysMap> getServerToFilterMapFESHOP(
      const std::shared_ptr<CacheableVector>& keySet,
      const std::shared_ptr<Region>& region, bool isPrimary);

  std::shared_ptr<ClientMetadataService::ServerToBucketsMap>
  groupByServerToAllBuckets(const std::shared_ptr<Region>& region,
                            bool optimizeForWrite);

  std::shared_ptr<ClientMetadataService::ServerToBucketsMap>
  groupByServerToBuckets(const std::shared_ptr<ClientMetadata>& metadata,
                         const BucketSet& bucketSet, bool optimizeForWrite);

  std::shared_ptr<ClientMetadataService::ServerToBucketsMap> pruneNodes(
      const std::shared_ptr<ClientMetadata>& metadata,
      const BucketSet& buckets);

 private:
  std::shared_ptr<ClientMetadata> SendClientPRMetadata(
      const char* regionPath, std::shared_ptr<ClientMetadata> cptr);

  std::shared_ptr<ClientMetadata> getClientMetadata(
      const std::shared_ptr<Region>& region);

 private:
  std::thread m_thread;
  boost::shared_mutex m_regionMetadataLock;
  RegionMetadataMapType m_regionMetaDataMap;
  std::atomic<bool> m_run;
  ThinClientPoolDM* m_pool;
  CacheImpl* m_cache;
  std::deque<std::string> m_regionQueue;
  std::mutex m_regionQueueMutex;
  std::condition_variable m_regionQueueCondition;
  boost::shared_mutex m_PRbucketStatusLock;
  std::map<std::string, std::unique_ptr<PRbuckets>> m_bucketStatus;
  std::chrono::milliseconds m_bucketWaitTimeout;
  static const char* NC_CMDSvcThread;
  std::unique_ptr<AppDomainContext> m_appDomainContext;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTMETADATASERVICE_H_
