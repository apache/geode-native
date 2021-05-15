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

#ifndef GEODE_CLIENTMETADATA_H_
#define GEODE_CLIENTMETADATA_H_

#include <map>
#include <vector>

#include <geode/PartitionResolver.hpp>

#include "BucketServerLocation.hpp"
#include "FixedPartitionAttributesImpl.hpp"
#include "ReadWriteLock.hpp"
#include "ServerLocation.hpp"
#include "util/Log.hpp"

/*Stores the information such as partition attributes and meta data details*/

namespace apache {
namespace geode {
namespace client {

class ThinClientPoolDM;
class ClientMetadata;

typedef std::vector<std::shared_ptr<BucketServerLocation>>
    BucketServerLocationsType;
// typedef std::map<int,BucketServerLocationsType >
// BucketServerLocationsListType;
typedef std::vector<BucketServerLocationsType> BucketServerLocationsListType;
typedef std::map<std::string, std::vector<int>> FixedMapType;

class ClientMetadata {
 private:
  void setPartitionNames();
  std::shared_ptr<CacheableHashSet> m_partitionNames;

  BucketServerLocationsListType m_bucketServerLocationsList;
  std::shared_ptr<ClientMetadata> m_previousOne;
  int m_totalNumBuckets;
  // std::shared_ptr<PartitionResolver> m_partitionResolver;
  std::string m_colocatedWith;
  ThinClientPoolDM* m_tcrdm;
  FixedMapType m_fpaMap;
  inline void checkBucketId(size_t bucketId) {
    if (bucketId >= m_bucketServerLocationsList.size()) {
      LOG_ERROR("ClientMetadata::getServerLocation(): BucketId out of range.");
      throw IllegalStateException(
          "ClientMetadata::getServerLocation(): BucketId out of range.");
    }
  }

 public:
  void setPreviousone(std::shared_ptr<ClientMetadata> cptr) {
    m_previousOne = cptr;
  }
  ~ClientMetadata();
  ClientMetadata();
  ClientMetadata(
      int totalNumBuckets, std::string colocatedWith, ThinClientPoolDM* tcrdm,
      std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* fpaSet);

  void getServerLocation(int bucketId, bool tryPrimary,
                         std::shared_ptr<BucketServerLocation>& serverLocation,
                         int8_t& version);
  // ServerLocation getPrimaryServerLocation(int bucketId);
  void updateBucketServerLocations(
      int bucketId, BucketServerLocationsType bucketServerLocations);
  int getTotalNumBuckets();
  // std::shared_ptr<PartitionResolver> getPartitionResolver();
  const std::string& getColocatedWith();
  int assignFixedBucketId(const char* partitionName,
                          std::shared_ptr<CacheableKey> resolvekey);
  std::shared_ptr<CacheableHashSet>& getFixedPartitionNames() {
    /* if(m_fpaMap.size() >0)
     {
      auto partitionNames = CacheableHashSet::create();
       for ( FixedMapType::iterator it=m_fpaMap.begin() ; it != m_fpaMap.end();
     it++ ) {
         partitionNames->insert(CacheableString::create(((*it).first).c_str()));
       }
       return partitionNames;
     }*/
    return m_partitionNames;
  }
  ClientMetadata(ClientMetadata& other);
  ClientMetadata& operator=(const ClientMetadata&) = delete;
  std::vector<std::shared_ptr<BucketServerLocation>> adviseServerLocations(
      int bucketId);
  std::shared_ptr<BucketServerLocation> advisePrimaryServerLocation(
      int bucketId);
  std::shared_ptr<BucketServerLocation> adviseRandomServerLocation();

  void removeBucketServerLocation(
      const std::shared_ptr<BucketServerLocation>& serverLocation);

  std::string toString();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTMETADATA_H_
