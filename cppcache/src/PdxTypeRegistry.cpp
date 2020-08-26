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

#include <geode/PoolManager.hpp>

#include "CacheImpl.hpp"
#include "CacheRegionHelper.hpp"
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
  return preserveData_.size();
}

int32_t PdxTypeRegistry::getPDXIdForType(const std::string& type, Pool* pool,
                                         std::shared_ptr<PdxType> nType,
                                         bool checkIfThere) {
  if (checkIfThere) {
    auto lpdx = getLocalPdxType(type);
    if (lpdx != nullptr) {
      int id = lpdx->getTypeId();
      if (id != 0) {
        return id;
      }
    }
  }

  int typeId = cache_->getSerializationRegistry()->GetPDXIdForType(pool, nType);
  nType->setTypeId(typeId);

  addPdxType(typeId, nType);
  return typeId;
}

int32_t PdxTypeRegistry::getPDXIdForType(std::shared_ptr<PdxType> nType,
                                         Pool* pool) {
  int32_t typeId = 0;
  {
    ReadGuard read(g_readerWriterLock_);
    auto&& iter = pdxTypeToTypeIdMap_.find(nType);
    if (iter != pdxTypeToTypeIdMap_.end()) {
      typeId = iter->second;
      if (typeId != 0) {
        return typeId;
      }
    }
  }

  {
    WriteGuard write(g_readerWriterLock_);
    auto&& iter = pdxTypeToTypeIdMap_.find(nType);
    if (iter != pdxTypeToTypeIdMap_.end()) {
      typeId = iter->second;
      if (typeId != 0) {
        return typeId;
      }
    }

    typeId = cache_->getSerializationRegistry()->GetPDXIdForType(pool, nType);
    nType->setTypeId(typeId);
    pdxTypeToTypeIdMap_.emplace(nType, typeId);
    typeIdToPdxType_.emplace(typeId, nType);
  }
  return typeId;
}

void PdxTypeRegistry::clear() {
  {
    WriteGuard guard(g_readerWriterLock_);
    typeIdToPdxType_.clear();

    remoteTypeIdToMergedPdxType_.clear();

    localTypeToPdxType_.clear();

    if (intToEnum_) intToEnum_->clear();

    if (enumToInt_) enumToInt_->clear();

    pdxTypeToTypeIdMap_.clear();
  }
  {
    WriteGuard guard(getPreservedDataLock());
    preserveData_.clear();
  }
}

void PdxTypeRegistry::addPdxType(int32_t typeId,
                                 std::shared_ptr<PdxType> pdxType) {
  WriteGuard guard(g_readerWriterLock_);
  typeIdToPdxType_.emplace(typeId, pdxType);
}

std::shared_ptr<PdxType> PdxTypeRegistry::getPdxType(int32_t typeId) const {
  ReadGuard guard(g_readerWriterLock_);
  auto&& iter = typeIdToPdxType_.find(typeId);
  if (iter != typeIdToPdxType_.end()) {
    return iter->second;
  }
  return nullptr;
}

void PdxTypeRegistry::addLocalPdxType(const std::string& localType,
                                      std::shared_ptr<PdxType> pdxType) {
  WriteGuard guard(g_readerWriterLock_);
  localTypeToPdxType_.emplace(localType, pdxType);
}

std::shared_ptr<PdxType> PdxTypeRegistry::getLocalPdxType(
    const std::string& localType) const {
  ReadGuard guard(g_readerWriterLock_);
  auto&& it = localTypeToPdxType_.find(localType);
  if (it != localTypeToPdxType_.end()) {
    return it->second;
  }
  return nullptr;
}

void PdxTypeRegistry::setMergedType(int32_t remoteTypeId,
                                    std::shared_ptr<PdxType> mergedType) {
  WriteGuard guard(g_readerWriterLock_);
  remoteTypeIdToMergedPdxType_.emplace(remoteTypeId, mergedType);
}

std::shared_ptr<PdxType> PdxTypeRegistry::getMergedType(
    int32_t remoteTypeId) const {
  auto&& it = remoteTypeIdToMergedPdxType_.find(remoteTypeId);
  if (it != remoteTypeIdToMergedPdxType_.end()) {
    return it->second;
  }
  return nullptr;
}

void PdxTypeRegistry::setPreserveData(
    std::shared_ptr<PdxSerializable> obj,
    std::shared_ptr<PdxRemotePreservedData> pData,
    ExpiryTaskManager& expiryTaskManager) {
  WriteGuard guard(getPreservedDataLock());
  pData->setOwner(obj);
  if (preserveData_.find(obj) != preserveData_.end()) {
    // reset expiry task
    // TODO: check value for nullptr
    auto expTaskId = preserveData_[obj]->getPreservedDataExpiryTaskId();
    expiryTaskManager.resetTask(expTaskId, 5);
    LOGDEBUG("PdxTypeRegistry::setPreserveData Reset expiry task Done");
    pData->setPreservedDataExpiryTaskId(expTaskId);
    preserveData_[obj] = pData;
  } else {
    // schedule new expiry task
    auto handler = new PreservedDataExpiryHandler(shared_from_this(), obj);
    auto id = expiryTaskManager.scheduleExpiryTask(
        handler, std::chrono::seconds(20), std::chrono::seconds::zero(), false);
    pData->setPreservedDataExpiryTaskId(id);
    LOGDEBUG(
        "PdxTypeRegistry::setPreserveData Schedule new expirt task with id=%ld",
        id);
    preserveData_.emplace(obj, pData);
  }

  LOGDEBUG(
      "PdxTypeRegistry::setPreserveData Successfully inserted new entry in "
      "preservedData");
}
std::shared_ptr<PdxRemotePreservedData> PdxTypeRegistry::getPreserveData(
    std::shared_ptr<PdxSerializable> pdxobj) const {
  ReadGuard guard(getPreservedDataLock());
  const auto& iter = preserveData_.find((pdxobj));
  if (iter != preserveData_.end()) {
    return iter->second;
  }
  return nullptr;
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

  WriteGuard guard(g_readerWriterLock_);
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

  WriteGuard guard(g_readerWriterLock_);
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
