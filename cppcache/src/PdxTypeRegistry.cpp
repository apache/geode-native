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

#include "PdxTypeRegistry.hpp"

#include <boost/thread/lock_types.hpp>

#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
#include "PdxType.hpp"
#include "PdxUnreadData.hpp"
#include "PdxUnreadDataExpiryTask.hpp"
#include "SerializationRegistry.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

PdxTypeRegistry::PdxTypeRegistry(CacheImpl* cache)
    : cache_(cache),
      typeIdToPdxType_(),
      remoteTypeIdToMergedPdxType_(),
      localTypeToPdxType_(),
      pdxTypeToTypeIdMap_(),
      enumToInt_(CacheableHashMap::create()),
      intToEnum_(CacheableHashMap::create()) {}

PdxTypeRegistry::~PdxTypeRegistry() {}

size_t PdxTypeRegistry::testNumberOfPreservedData() const {
  return unreadData_.size();
}

std::shared_ptr<PdxType> PdxTypeRegistry::getPdxType(int32_t typeId,
                                                     Pool* pool) {
  // Check if PdxType ID is already present in the registry
  if (typeId != 0) {
    boost::shared_lock<decltype(typesMutex_)> guard{typesMutex_};
    auto&& iter = typeIdToPdxType_.find(typeId);
    if (iter != typeIdToPdxType_.end()) {
      return iter->second;
    }
  }

  // This is not within exclusive access to avoid inter-locks due to I/O
  auto type = cache_->getSerializationRegistry()->GetPDXTypeById(pool, typeId);

  boost::unique_lock<decltype(typesMutex_)> guard{typesMutex_};

  // Check if the PdxType was already added while fetcthing it to the cluster
  auto&& iter = typeIdToPdxType_.find(typeId);
  if (iter != typeIdToPdxType_.end()) {
    return iter->second;
  }

  auto pdxType = std::dynamic_pointer_cast<PdxType>(type);
  if (!pdxType) {
    throw IllegalStateException("Fetched PdxType=" + std::to_string(typeId) +
                                " is not of the right type");
  }

  pdxTypeToTypeIdMap_.emplace(pdxType, typeId);
  typeIdToPdxType_.emplace(typeId, pdxType);
  return pdxType;
}

void PdxTypeRegistry::registerPdxTypeIfNeeded(std::shared_ptr<PdxType> pdxType,
                                              Pool* pool) {
  auto typeId = pdxType->getTypeId();

  // Check if PdxType ID is already present in the registry
  if (typeId != 0) {
    boost::shared_lock<decltype(typesMutex_)> guard{typesMutex_};
    auto&& iter = typeIdToPdxType_.find(typeId);
    if (iter != typeIdToPdxType_.end()) {
      return;
    }
  }

  // Check if PdxType is already present in the registry but its ID was not
  // assigned
  boost::upgrade_lock<decltype(typesMutex_)> lock{typesMutex_};
  auto&& iter = pdxTypeToTypeIdMap_.find(pdxType);
  if (iter != pdxTypeToTypeIdMap_.end() && (typeId = iter->second) != 0) {
    pdxType->setTypeId(typeId);
    return;
  }

  // Fetch the PdxType ID from the cluster and add it to the registry
  boost::upgrade_to_unique_lock<decltype(typesMutex_)> uniqueLock{lock};
  typeId = cache_->getSerializationRegistry()->GetPDXIdForType(pool, pdxType);
  pdxType->setTypeId(typeId);

  pdxTypeToTypeIdMap_.emplace(pdxType, typeId);
  typeIdToPdxType_.emplace(typeId, pdxType);
}

void PdxTypeRegistry::clear() {
  {
    boost::unique_lock<decltype(typesMutex_)> guard{typesMutex_};
    typeIdToPdxType_.clear();
    pdxTypeToTypeIdMap_.clear();

    if (intToEnum_) {
      intToEnum_->clear();
    }

    if (enumToInt_) {
      enumToInt_->clear();
    }
  }
  {
    boost::unique_lock<decltype(unreadDataMutex_)> guard{unreadDataMutex_};
    unreadData_.clear();
  }
}

void PdxTypeRegistry::setUnreadData(ExpiryTaskManager& expiryTaskManager,
                                    std::shared_ptr<PdxSerializable> obj,
                                    std::shared_ptr<PdxUnreadData> data) {
  boost::unique_lock<decltype(unreadDataMutex_)> guard{unreadDataMutex_};

  auto&& iter = unreadData_.find(obj);
  if (iter != unreadData_.end()) {
    auto expireAt = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    data->taskId(iter->second->taskId());
    data->expiresAt(expireAt);
    iter->second = data;
  } else {
    constexpr auto lifespan = std::chrono::seconds(20);
    auto expireAt = std::chrono::steady_clock::now() + lifespan;
    auto task = std::make_shared<PdxUnreadDataExpiryTask>(
        expiryTaskManager, shared_from_this(), obj);
    auto id = expiryTaskManager.schedule(std::move(task), lifespan);
    data->taskId(id);
    data->expiresAt(expireAt);

    LOGDEBUG(
        "PdxTypeRegistry::setUnreadData Schedule new expiry task with id=%zu",
        id);
    unreadData_.emplace_hint(iter, std::move(obj), std::move(data));
  }

  LOGDEBUG(
      "PdxTypeRegistry::setUnreadData Successfully inserted new entry in "
      "preservedData");
}

void PdxTypeRegistry::removeUnreadData(
    std::shared_ptr<PdxSerializable> object) {
  boost::shared_lock<decltype(typesMutex_)> guard{typesMutex_};
  unreadData_.erase(object);
}

std::shared_ptr<PdxUnreadData> PdxTypeRegistry::getUnreadData(
    std::shared_ptr<PdxSerializable> object) const {
  boost::shared_lock<decltype(typesMutex_)> guard{typesMutex_};

  auto&& iter = unreadData_.find(object);
  return iter != unreadData_.end() ? iter->second : nullptr;
}

int32_t PdxTypeRegistry::getEnumValue(std::shared_ptr<EnumInfo> ei) {
  // TODO locking - naive concurrent optimization?
  std::shared_ptr<CacheableHashMap> tmp;
  tmp = enumToInt_;
  const auto& entry = tmp->find(ei);
  if (entry != tmp->end()) {
    const auto val = std::dynamic_pointer_cast<CacheableInt32>(entry->second);
    return val->value();
  }

  boost::unique_lock<decltype(typesMutex_)> guard{typesMutex_};
  tmp = enumToInt_;
  const auto& entry2 = tmp->find(ei);
  if (entry2 != tmp->end()) {
    const auto val2 = std::dynamic_pointer_cast<CacheableInt32>(entry2->second);
    return val2->value();
  }

  int val = static_cast<ThinClientPoolDM*>(
                cache_->getPoolManager().getAll().begin()->second.get())
                ->GetEnumValue(ei);

  tmp = enumToInt_;
  tmp->emplace(ei, CacheableInt32::create(val));
  enumToInt_ = tmp;
  return val;
}

std::shared_ptr<EnumInfo> PdxTypeRegistry::getEnum(int32_t enumVal) {
  // TODO locking - naive concurrent optimization?
  auto enumValPtr = CacheableInt32::create(enumVal);

  auto&& tmp = intToEnum_;
  {
    auto&& entry = tmp->find(enumValPtr);
    if (entry != tmp->end()) {
      auto&& ret = std::dynamic_pointer_cast<EnumInfo>(entry->second);
      if (ret) {
        return std::move(ret);
      }
    }
  }

  boost::unique_lock<decltype(typesMutex_)> guard{typesMutex_};
  tmp = intToEnum_;
  {
    auto&& entry = tmp->find(enumValPtr);
    if (entry != tmp->end()) {
      auto&& ret = std::dynamic_pointer_cast<EnumInfo>(entry->second);
      if (ret) {
        return std::move(ret);
      }
    }
  }

  auto&& ret = std::dynamic_pointer_cast<EnumInfo>(
      std::static_pointer_cast<ThinClientPoolDM>(
          cache_->getPoolManager().getAll().begin()->second)
          ->GetEnum(enumVal));
  tmp = intToEnum_;
  (*tmp)[enumValPtr] = ret;
  intToEnum_ = tmp;
  return std::move(ret);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
