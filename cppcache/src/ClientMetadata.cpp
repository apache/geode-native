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

#include "ClientMetadata.hpp"

#include <climits>
#include <cstdlib>

#include "ThinClientPoolDM.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

ClientMetadata::ClientMetadata()
    : partitionNames_{},
      previous_{},
      totalBucketsCount_{0},
      colocatedWith_{},
      tcrdm_{nullptr} {}

ClientMetadata::ClientMetadata(
    int totalNumBuckets, std::string colocatedWith, ThinClientPoolDM* tcrdm,
    std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* fpaSet)
    : partitionNames_{},
      previous_{},
      totalBucketsCount_{totalNumBuckets},
      colocatedWith_{std::move(colocatedWith)},
      tcrdm_{tcrdm} {
  LOGFINE("Creating metadata with %d buckets", totalNumBuckets);
  if(totalNumBuckets > 0) {
    locationsList_.assign(totalNumBuckets, BucketServerLocationsType{});
  }

  if (tcrdm_ == nullptr) {
    throw IllegalArgumentException(
        "ClientMetaData: ThinClientPoolDM is nullptr.");
  }

  if (fpaSet != nullptr) {
    LOGDEBUG(
        "ClientMetadata Creating metadata with %d buckets & fpaset size is "
        "%zu ",
        totalNumBuckets, fpaSet->size());

    if (!fpaSet->empty()) {

      for (auto&& fpaAttrs : *fpaSet) {
        std::vector<int> attList;
        attList.push_back(fpaAttrs->getNumBuckets());
        attList.push_back(fpaAttrs->getStartingBucketID());
        fpaMap_[fpaAttrs->getPartitionName()] = attList;
      }

      setPartitionNames();
    }

    delete fpaSet;
  }
}

ClientMetadata::ClientMetadata(ClientMetadata& other)
    : partitionNames_{},
      previous_{},
      totalBucketsCount_{other.totalBucketsCount_},
      colocatedWith_{other.colocatedWith_},
      tcrdm_{other.tcrdm_} {
  if(totalBucketsCount_ > 0) {
    locationsList_.assign(totalBucketsCount_, BucketServerLocationsType{});
  }

  for (FixedMapType::iterator iter = other.fpaMap_.begin();
       iter != other.fpaMap_.end(); ++iter) {
    fpaMap_[iter->first] = iter->second;
  }

  if (!fpaMap_.empty()) {
    setPartitionNames();
  }
}

ClientMetadata::~ClientMetadata() = default;

void ClientMetadata::setPreviousMetadata(std::shared_ptr<ClientMetadata> cptr) {
  previous_ = std::move(cptr);
}

void ClientMetadata::removeBucketServerLocation(
    const std::shared_ptr<BucketServerLocation>& serverLocation) {
  for (auto&& locations : locationsList_) {
    for (unsigned int i = 0; i < locations.size(); i++) {
      if (locations[i]->getEpString() == (serverLocation->getEpString())) {
        locations.erase(locations.begin() + i);
        break;
      }
    }
  }
}

void ClientMetadata::getServerLocation(
    int bucketId, bool tryPrimary,
    std::shared_ptr<BucketServerLocation>& serverLocation, int8_t& version) {
  checkBucketId(bucketId);
  const auto& locations = locationsList_[bucketId];
  if (locations.empty()) {
    LOGFINER("locationsList_[%d] size is zero", bucketId);
    return;
  } else if (tryPrimary) {
    LOGFINER("returning primary & locationsList_ size is %zu",
             locationsList_.size());
    serverLocation = locations.at(0);
    if (serverLocation->isValid()) {
      if (serverLocation->isPrimary()) {
        version = serverLocation->getVersion();
      }
    } else {
      version = serverLocation->getVersion();
    }
  } else {
    serverLocation = locations.at(0);
    if (serverLocation->isValid()) {
      if (serverLocation->isPrimary()) {
        version = serverLocation->getVersion();
      }
    } else {
      version = serverLocation->getVersion();
    }
    LOGFINER("returning random & locationsList_ size is: %zu",
             locationsList_.size());
    RandGen randgen;

    serverLocation = locations.at(randgen(static_cast<int>(locations.size())));
  }
}

void ClientMetadata::updateBucketServerLocations(
    int bucketId, BucketServerLocationsType bucketServerLocations) {
  checkBucketId(bucketId);

  auto&& serverGroup = tcrdm_->getServerGroup();

  // This is for pruning according to server groups, only applicable when client
  // is configured with
  // server-group.
  if (serverGroup.length() != 0) {
    BucketServerLocationsType primaries;
    BucketServerLocationsType secondaries;
    for (BucketServerLocationsType::iterator iter =
             bucketServerLocations.begin();
         iter != bucketServerLocations.end(); ++iter) {
      auto groups = (*iter)->getServerGroups();
      if ((groups != nullptr) && (groups->length() > 0)) {
        bool added = false;
        for (int i = 0; i < groups->length(); i++) {
          auto cs = (*groups)[i];
          if (cs->length() > 0) {
            auto&& str = cs->toString();
            if (str == serverGroup) {
              added = true;
              if ((*iter)->isPrimary()) {
                primaries.push_back(*iter);
                break;
              } else {
                secondaries.push_back(*iter);
                break;
              }
            }
          } else {
            added = true;
            if ((*iter)->isPrimary()) {
              primaries.push_back(*iter);
            } else {
              secondaries.push_back(*iter);
            }
          }
        }
        if (!added) {
          if ((*iter)->isPrimary()) {
            primaries.push_back(*iter);
          } else {
            secondaries.push_back(*iter);
          }
        }
      }
    }

    // shuffle the deck

    RandGen randGen;

    if (primaries.size() > 0) {
      std::shuffle(primaries.begin(), primaries.end(), randGen);
    }

    if (secondaries.size() > 0) {
      std::shuffle(secondaries.begin(), secondaries.end(), randGen);
    }

    locationsList_[bucketId].clear();
    for (BucketServerLocationsType::iterator iter = primaries.begin();
         iter != primaries.end(); ++iter) {
      LOGFINER("updating primaries with bucketId %d", bucketId);
      locationsList_[bucketId].push_back(*iter);
    }

    // add secondaries to the end
    for (BucketServerLocationsType::iterator iter = secondaries.begin();
         iter != secondaries.end(); ++iter) {
      LOGFINER("updating secondaries with bucketId %d", bucketId);
      locationsList_[bucketId].push_back(*iter);
    }
  } else {
    BucketServerLocationsType primaries;
    BucketServerLocationsType secondaries;

    // separate out the primaries from the secondaries

    for (BucketServerLocationsType::iterator iter =
             bucketServerLocations.begin();
         iter != bucketServerLocations.end(); ++iter) {
      if ((*iter)->isPrimary()) {
        primaries.push_back(*iter);
      } else {
        secondaries.push_back(*iter);
      }
    }

    // shuffle the deck

    RandGen randGen;

    if (primaries.size() > 0) {
      std::shuffle(primaries.begin(), primaries.end(), randGen);
    }

    if (secondaries.size() > 0) {
      std::shuffle(secondaries.begin(), secondaries.end(), randGen);
    }

    locationsList_[bucketId].clear();

    // add primaries to the front
    for (BucketServerLocationsType::iterator iter = primaries.begin();
         iter != primaries.end(); ++iter) {
      LOGFINER("updating primaries with bucketId %d and Server = %s ", bucketId,
               (*iter)->getEpString().c_str());
      locationsList_[bucketId].push_back(*iter);
    }

    // add secondaries to the end
    for (BucketServerLocationsType::iterator iter = secondaries.begin();
         iter != secondaries.end(); ++iter) {
      LOGFINER("updating secondaries with bucketId %d", bucketId);
      locationsList_[bucketId].push_back(*iter);
    }
  }
}

int ClientMetadata::assignFixedBucketId(
    const char* partitionName, std::shared_ptr<CacheableKey> resolvekey) {
  LOGDEBUG(
      "FPR assignFixedBucketId partititonname = %s , fpaMap_.size() = %zu ",
      partitionName, fpaMap_.size());
  FixedMapType::iterator iter = fpaMap_.find(partitionName);
  if (iter != fpaMap_.end()) {
    auto attList = iter->second;
    int32_t hc = resolvekey->hashcode();
    int bucketId = std::abs(hc % (attList.at(0)));
    int partitionBucketID = bucketId + attList.at(1);
    return partitionBucketID;
  } else {
    return -1;
  }
}

BucketServerLocationsType ClientMetadata::adviseServerLocations(int bucketId) {
  checkBucketId(bucketId);
  return locationsList_[bucketId];
}

std::shared_ptr<BucketServerLocation>
ClientMetadata::advisePrimaryServerLocation(int bucketId) {
  auto locations = adviseServerLocations(bucketId);
  for (std::vector<std::shared_ptr<BucketServerLocation>>::iterator iter =
           locations.begin();
       iter != locations.end(); ++iter) {
    auto location = *iter;
    if (location->isPrimary()) {
      return location;
    }
  }

  return std::shared_ptr<BucketServerLocation>{};
}

std::string ClientMetadata::toString() {
  std::string out = "";
  for (auto&& locations : locationsList_) {
    for (auto&& location : locations) {
      out += location->toString() + "|";
    }
    out += "$";
  }
  return out;
}

void ClientMetadata::setPartitionNames() {
  partitionNames_ = CacheableHashSet::create();
  for (FixedMapType::iterator it = fpaMap_.begin(); it != fpaMap_.end(); it++) {
    partitionNames_->insert(CacheableString::create(((*it).first).c_str()));
  }
}

void ClientMetadata::checkBucketId(int bucketId) {
  if (bucketId >= totalBucketsCount_) {
    LOGERROR("ClientMetadata::getServerLocation(): BucketId out of range.");
    throw IllegalStateException(
        "ClientMetadata::getServerLocation(): BucketId out of range.");
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
