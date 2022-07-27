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
typedef std::vector<BucketServerLocationsType> BucketServerLocationsListType;
typedef std::map<std::string, std::vector<int>> FixedMapType;

class ClientMetadata {
 public:
  virtual ~ClientMetadata();

  ClientMetadata();
  ClientMetadata(
      int totalNumBuckets, std::string colocatedWith, ThinClientPoolDM* tcrdm,
      std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* fpaSet);

  ClientMetadata(ClientMetadata& other);
  ClientMetadata& operator=(const ClientMetadata&) = delete;

  int getTotalNumBuckets() const { return totalBucketsCount_; }

  const std::string& getColocatedWith() const { return colocatedWith_; }

  const std::shared_ptr<CacheableHashSet>& getFixedPartitionNames() const {
    return partitionNames_;
  }

  void getServerLocation(int bucketId, bool tryPrimary,
                         std::shared_ptr<BucketServerLocation>& serverLocation,
                         int8_t& version);

  void setPreviousMetadata(std::shared_ptr<ClientMetadata> cptr);

  void updateBucketServerLocations(
      int bucketId, BucketServerLocationsType bucketServerLocations);

  int assignFixedBucketId(const char* partitionName,
                          std::shared_ptr<CacheableKey> key);

  std::shared_ptr<BucketServerLocation> advisePrimaryServerLocation(
      int bucketId);

  void removeBucketServerLocation(
      const std::shared_ptr<BucketServerLocation>& serverLocation);

  std::string toString();

  virtual BucketServerLocationsType adviseServerLocations(int bucketId);

 private:
  void setPartitionNames();

  void checkBucketId(int bucketId);

 private:
  std::shared_ptr<CacheableHashSet> partitionNames_;

  BucketServerLocationsListType locationsList_;

  std::shared_ptr<ClientMetadata> previous_;

  int totalBucketsCount_;
  std::string colocatedWith_;

  ThinClientPoolDM* tcrdm_;

  FixedMapType fpaMap_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CLIENTMETADATA_H_
