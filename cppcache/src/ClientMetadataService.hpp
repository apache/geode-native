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

#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>
#include <chrono>

#include <ace/Task.h>

#include <geode/CacheableKey.hpp>
#include <geode/Serializable.hpp>
#include <geode/Region.hpp>
#include <geode/internal/functional.hpp>

#include "ClientMetadata.hpp"
#include "ServerLocation.hpp"
#include "BucketServerLocation.hpp"
#include "Queue.hpp"
#include "DistributedSystemImpl.hpp"
#include "NonCopyable.hpp"

namespace apache {
namespace geode {
namespace client {

class ClienMetadata;

static constexpr std::chrono::milliseconds DEFAULT_MUTEX_TIMEOUT =
    std::chrono::seconds(1);

static constexpr std::chrono::milliseconds CLOSE_SERVICE_MUTEX_TIMEOUT =
    std::chrono::seconds(60);

typedef std::map<std::string, std::shared_ptr<ClientMetadata>>
    RegionMetadataMapType;

class BucketStatus {
 private:
  ACE_Time_Value m_lastTimeout;

 public:
  BucketStatus() : m_lastTimeout(ACE_Time_Value::zero) {}
  bool isTimedoutAndReset(std::chrono::milliseconds millis) {
    if (m_lastTimeout == ACE_Time_Value::zero) {
      return false;
    } else {
      ACE_Time_Value to(millis);
      to += m_lastTimeout;
      if (to > ACE_OS::gettimeofday()) {
        return true;  // timeout as buckste not recovered yet
      } else {
        // reset to zero as we waited enough to recover bucket
        m_lastTimeout = ACE_Time_Value::zero;
        return false;
      }
    }
  }

  void setTimeout() {
    if (m_lastTimeout == ACE_Time_Value::zero) {
      m_lastTimeout = ACE_OS::gettimeofday();  // set once only for timeout
    }
  }
};

class PRbuckets {
 private:
  BucketStatus* m_buckets;

 public:
  PRbuckets(int32_t nBuckets) { m_buckets = new BucketStatus[nBuckets]; }
  ~PRbuckets() { delete[] m_buckets; }

  bool isBucketTimedOut(int32_t bucketId, std::chrono::milliseconds millis) {
    return m_buckets[bucketId].isTimedoutAndReset(millis);
  }

  void setBucketTimeout(int32_t bucketId) { m_buckets[bucketId].setTimeout(); }
};

class ClientMetadataService : public ACE_Task_Base,
                              private NonCopyable,
                              private NonAssignable {
 public:
  ~ClientMetadataService();
  ClientMetadataService(Pool* pool);

  inline void start() {
    m_run = true;
    this->activate();
  }

  inline void stop() {
    m_run = false;
    m_regionQueueSema.release();
    this->wait();
  }

  int svc(void);

  void getClientPRMetadata(const char* regionFullPath);

  void getBucketServerLocation(
      const std::shared_ptr<Region>& region,
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Cacheable>& value,
      const std::shared_ptr<Serializable>& aCallbackArgument, bool isPrimary,
      std::shared_ptr<BucketServerLocation>& serverLocation, int8_t& version);

  void removeBucketServerLocation(BucketServerLocation serverLocation);

  std::shared_ptr<ClientMetadata> getClientMetadata(
      const std::string& regionFullPath);

  void populateDummyServers(const char* regionName,
                            std::shared_ptr<ClientMetadata> clientmetadata);

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
  // bool AreBucketSetsEqual(const BucketSet& currentBucketSet,
  //                        const BucketSet& bucketSet);

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
  // const std::shared_ptr<PartitionResolver>& getResolver(const
  // std::shared_ptr<Region>& region, const std::shared_ptr<CacheableKey>& key,
  // const std::shared_ptr<Serializable>& aCallbackArgument);

  // BucketServerLocation getServerLocation(std::shared_ptr<ClientMetadata>
  // cptr, int bucketId, bool isPrimary);

  std::shared_ptr<ClientMetadata> SendClientPRMetadata(
      const char* regionPath, std::shared_ptr<ClientMetadata> cptr);

  std::shared_ptr<ClientMetadata> getClientMetadata(
      const std::shared_ptr<Region>& region);

 private:
  // ACE_Recursive_Thread_Mutex m_regionMetadataLock;
  ACE_RW_Thread_Mutex m_regionMetadataLock;
  ClientMetadataService();
  ACE_Semaphore m_regionQueueSema;
  RegionMetadataMapType m_regionMetaDataMap;
  volatile bool m_run;
  Pool* m_pool;
  Queue<std::string>* m_regionQueue;

  std::map<std::string, PRbuckets*> m_bucketStatus;
  std::chrono::milliseconds m_bucketWaitTimeout;
  static const char* NC_CMDSvcThread;
  std::timed_mutex m_timedBucketStatusLock;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTMETADATASERVICE_H_
