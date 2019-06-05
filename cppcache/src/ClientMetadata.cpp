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

ClientMetadata::ClientMetadata(
    int totalNumBuckets, std::string colocatedWith, ThinClientPoolDM* tcrdm,
    std::vector<std::shared_ptr<FixedPartitionAttributesImpl>>* fpaSet)
    : m_partitionNames(nullptr),
      m_previousOne(nullptr),
      m_totalNumBuckets(totalNumBuckets),
      m_colocatedWith(std::move(colocatedWith)),
      m_tcrdm(tcrdm) {
  LOGFINE("Creating metadata with %d buckets", totalNumBuckets);
  for (int item = 0; item < totalNumBuckets; item++) {
    BucketServerLocationsType empty;
    m_bucketServerLocationsList.push_back(empty);
  }
  if (m_tcrdm == nullptr) {
    throw IllegalArgumentException(
        "ClientMetaData: ThinClientPoolDM is nullptr.");
  }
  if (fpaSet != nullptr) {
    LOGDEBUG(
        "ClientMetadata Creating metadata with %d buckets & fpaset size is %d ",
        totalNumBuckets, fpaSet->size());
    if (!fpaSet->empty()) {
      int totalFPABuckets = 0;
      for (int i = 0; i < static_cast<int>(fpaSet->size()); i++) {
        std::vector<int> attList;
        totalFPABuckets += fpaSet->at(i)->getNumBuckets();
        attList.push_back(fpaSet->at(i)->getNumBuckets());
        attList.push_back(fpaSet->at(i)->getStartingBucketID());
        m_fpaMap[fpaSet->at(i)->getPartitionName()] = attList;
      }
      setPartitionNames();
    }
    delete fpaSet;
  }
}

ClientMetadata::ClientMetadata(ClientMetadata& other) {
  m_partitionNames = nullptr;
  m_previousOne = nullptr;
  m_totalNumBuckets = other.m_totalNumBuckets;
  for (int item = 0; item < m_totalNumBuckets; item++) {
    BucketServerLocationsType empty;
    m_bucketServerLocationsList.push_back(empty);
  }
  m_colocatedWith = other.m_colocatedWith;
  m_tcrdm = other.m_tcrdm;
  for (FixedMapType::iterator iter = other.m_fpaMap.begin();
       iter != other.m_fpaMap.end(); ++iter) {
    m_fpaMap[iter->first] = iter->second;
  }
  if (!m_fpaMap.empty()) setPartitionNames();
}

ClientMetadata::ClientMetadata()
    : m_partitionNames(nullptr),
      m_previousOne(nullptr),
      m_totalNumBuckets(0),
      m_colocatedWith(nullptr),
      m_tcrdm(nullptr) {}

ClientMetadata::~ClientMetadata() {}

void ClientMetadata::setPartitionNames() {
  m_partitionNames = CacheableHashSet::create();
  for (FixedMapType::iterator it = m_fpaMap.begin(); it != m_fpaMap.end();
       it++) {
    m_partitionNames->insert(CacheableString::create(((*it).first).c_str()));
  }
}

int ClientMetadata::getTotalNumBuckets() {
  return m_totalNumBuckets;
}

const std::string& ClientMetadata::getColocatedWith() {
  return m_colocatedWith;
}

void ClientMetadata::getServerLocation(
    int bucketId, bool tryPrimary,
    std::shared_ptr<BucketServerLocation>& serverLocation, int8_t& version) {

  checkBucketId(bucketId);

  if (m_bucketServerLocationsList[bucketId].empty()) {
    return;
  } else if (tryPrimary) {
    LOGFINER("returning primary & m_bucketServerLocationsList size is %d",
             m_bucketServerLocationsList.size());
    serverLocation = m_bucketServerLocationsList[bucketId].at(0);
    if (serverLocation->isValid()) {
      if (serverLocation->isPrimary()) {
        version = serverLocation->getVersion();
      }
    } else {
      version = serverLocation->getVersion();
    }
  } else {
    serverLocation = m_bucketServerLocationsList[bucketId].at(0);
    if (serverLocation->isValid()) {
      if (serverLocation->isPrimary()) {
        version = serverLocation->getVersion();
      }
    } else {
      version = serverLocation->getVersion();
    }
    RandGen randgen;
    serverLocation = m_bucketServerLocationsList[bucketId].at(randgen(
        static_cast<int>(m_bucketServerLocationsList[bucketId].size())));
  }
}


void ClientMetadata::updateBucketServerLocations(
    int bucketId, BucketServerLocationsType bucketServerLocations) {
  checkBucketId(bucketId);

  auto&& serverGroup = m_tcrdm->getServerGroup();

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
      std::random_shuffle(primaries.begin(), primaries.end(), randGen);
    }

    if (secondaries.size() > 0) {
      std::random_shuffle(secondaries.begin(), secondaries.end(), randGen);
    }

    m_bucketServerLocationsList[bucketId].clear();
    for (BucketServerLocationsType::iterator iter = primaries.begin();
         iter != primaries.end(); ++iter) {
      LOGFINER("updating primaries with bucketId %d", bucketId);
      m_bucketServerLocationsList[bucketId].push_back(*iter);
    }

    // add secondaries to the end
    for (BucketServerLocationsType::iterator iter = secondaries.begin();
         iter != secondaries.end(); ++iter) {
      LOGFINER("updating secondaries with bucketId %d", bucketId);
      m_bucketServerLocationsList[bucketId].push_back(*iter);
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
      std::random_shuffle(primaries.begin(), primaries.end(), randGen);
    }

    if (secondaries.size() > 0) {
      std::random_shuffle(secondaries.begin(), secondaries.end(), randGen);
    }

    m_bucketServerLocationsList[bucketId].clear();

    // add primaries to the front
    for (BucketServerLocationsType::iterator iter = primaries.begin();
         iter != primaries.end(); ++iter) {
      LOGFINER("updating primaries with bucketId %d and Server = %s ", bucketId,
               (*iter)->getEpString().c_str());
      m_bucketServerLocationsList[bucketId].push_back(*iter);
    }

    // add secondaries to the end
    for (BucketServerLocationsType::iterator iter = secondaries.begin();
         iter != secondaries.end(); ++iter) {
      LOGFINER("updating secondaries with bucketId %d", bucketId);
      m_bucketServerLocationsList[bucketId].push_back(*iter);
    }
  }
}

int ClientMetadata::assignFixedBucketId(
    const char* partitionName, std::shared_ptr<CacheableKey> resolvekey) {
  LOGDEBUG(
      "FPR assignFixedBucketId partititonname = %s , m_fpaMap.size() = %d ",
      partitionName, m_fpaMap.size());
  FixedMapType::iterator iter = m_fpaMap.find(partitionName);
  if (iter != m_fpaMap.end()) {
    auto attList = iter->second;
    int32_t hc = resolvekey->hashcode();
    int bucketId = std::abs(hc % (attList.at(0)));
    int partitionBucketID = bucketId + attList.at(1);
    return partitionBucketID;
  } else {
    return -1;
  }
}

std::vector<std::shared_ptr<BucketServerLocation>>
ClientMetadata::adviseServerLocations(int bucketId) {
  checkBucketId(bucketId);
  return m_bucketServerLocationsList[bucketId];
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
  return nullptr;
}
std::shared_ptr<BucketServerLocation>
ClientMetadata::adviseRandomServerLocation() {
  if (m_bucketServerLocationsList.size() > 0) {
    RandGen randGen;
    size_t random = randGen(m_bucketServerLocationsList.size());
    checkBucketId(random);
    auto locations = m_bucketServerLocationsList[random];
    if (locations.size() == 0) return nullptr;
    return locations.at(0);
  }
  return nullptr;
}

void ClientMetadata::checkBucketId(size_t bucketId) {
    if (bucketId >= m_bucketServerLocationsList.size()) {
      LOGERROR("ClientMetadata::getServerLocation(): BucketId out of range.");
      throw IllegalStateException(
          "ClientMetadata::getServerLocation(): BucketId out of range.");
    }
  }

  void ClientMetadata::setPreviousone(std::shared_ptr<ClientMetadata> cptr) {
    m_previousOne = cptr;
  }

  std::shared_ptr<CacheableHashSet>& ClientMetadata::getFixedPartitionNames() {
    return m_partitionNames;
  }
}  // namespace client
}  // namespace geode
}  // namespace apache
