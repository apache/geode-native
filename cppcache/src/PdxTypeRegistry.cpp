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

#include <geode/PoolManager.hpp>

#include "PdxTypeRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

PdxTypeRegistry::PdxTypeRegistry(Cache* cache)
    : cache(cache),
      typeIdToPdxType(),
      remoteTypeIdToMergedPdxType(),
      localTypeToPdxType(),
      pdxTypeToTypeIdMap(),
      enumToInt(CacheableHashMap::create()),
      intToEnum(CacheableHashMap::create()) {}

PdxTypeRegistry::~PdxTypeRegistry() {}

size_t PdxTypeRegistry::testNumberOfPreservedData() const {
  return preserveData.size();
}

int32_t PdxTypeRegistry::getPDXIdForType(const char* type, const char* poolname,
                                         PdxTypePtr nType, bool checkIfThere) {
  // WriteGuard guard(g_readerWriterLock);
  if (checkIfThere) {
    PdxTypePtr lpdx = getLocalPdxType(type);
    if (lpdx != nullptr) {
      int id = lpdx->getTypeId();
      if (id != 0) {
        return id;
      }
    }
  }

  int typeId =
      CacheRegionHelper::getCacheImpl(cache)
          ->getSerializationRegistry()
          ->GetPDXIdForType(cache->getPoolManager().find(poolname), nType);
  nType->setTypeId(typeId);

  PdxTypeRegistry::addPdxType(typeId, nType);
  return typeId;
}

int32_t PdxTypeRegistry::getPDXIdForType(PdxTypePtr nType,
                                         const char* poolname) {
  int32_t typeId = 0;
  {
    ReadGuard read(g_readerWriterLock);
    PdxTypeToTypeIdMap::iterator iter = pdxTypeToTypeIdMap.find(nType);
    if (iter != pdxTypeToTypeIdMap.end()) {
      typeId = iter->second;
      if (typeId != 0) {
        return typeId;
      }
    }
  }

  WriteGuard write(g_readerWriterLock);

  PdxTypeToTypeIdMap::iterator iter = pdxTypeToTypeIdMap.find(nType);
  if (iter != pdxTypeToTypeIdMap.end()) {
    typeId = iter->second;
    if (typeId != 0) {
      return typeId;
    }
  }

  typeId = CacheRegionHelper::getCacheImpl(cache)
               ->getSerializationRegistry()
               ->GetPDXIdForType(cache->getPoolManager().find(poolname), nType);
  nType->setTypeId(typeId);
  pdxTypeToTypeIdMap.insert(std::make_pair(nType, typeId));
  addPdxType(typeId, nType);
  return typeId;
}

void PdxTypeRegistry::clear() {
  {
    WriteGuard guard(g_readerWriterLock);
    typeIdToPdxType.clear();

    remoteTypeIdToMergedPdxType.clear();

    localTypeToPdxType.clear();

    if (intToEnum) intToEnum->clear();

    if (enumToInt) enumToInt->clear();

    pdxTypeToTypeIdMap.clear();
  }
  {
    WriteGuard guard(getPreservedDataLock());
    preserveData.clear();
  }
}

void PdxTypeRegistry::addPdxType(int32_t typeId, PdxTypePtr pdxType) {
  WriteGuard guard(g_readerWriterLock);
  std::pair<int32_t, PdxTypePtr> pc(typeId, pdxType);
  typeIdToPdxType.insert(pc);
}

PdxTypePtr PdxTypeRegistry::getPdxType(int32_t typeId) {
  ReadGuard guard(g_readerWriterLock);
  PdxTypePtr retValue = nullptr;
  TypeIdVsPdxType::iterator iter;
  iter = typeIdToPdxType.find(typeId);
  if (iter != typeIdToPdxType.end()) {
    retValue = (*iter).second;
    return retValue;
  }
  return nullptr;
}

void PdxTypeRegistry::addLocalPdxType(const char* localType,
                                      PdxTypePtr pdxType) {
  WriteGuard guard(g_readerWriterLock);
  localTypeToPdxType.insert(
      std::pair<std::string, PdxTypePtr>(localType, pdxType));
}

PdxTypePtr PdxTypeRegistry::getLocalPdxType(const char* localType) {
  ReadGuard guard(g_readerWriterLock);
  PdxTypePtr localTypePtr = nullptr;
  TypeNameVsPdxType::iterator it;
  it = localTypeToPdxType.find(localType);
  if (it != localTypeToPdxType.end()) {
    localTypePtr = (*it).second;
    return localTypePtr;
  }
  return nullptr;
}

void PdxTypeRegistry::setMergedType(int32_t remoteTypeId,
                                    PdxTypePtr mergedType) {
  WriteGuard guard(g_readerWriterLock);
  std::pair<int32_t, PdxTypePtr> mergedTypePair(remoteTypeId, mergedType);
  remoteTypeIdToMergedPdxType.insert(mergedTypePair);
}

PdxTypePtr PdxTypeRegistry::getMergedType(int32_t remoteTypeId) {
  PdxTypePtr retVal = nullptr;
  TypeIdVsPdxType::iterator it;
  it = remoteTypeIdToMergedPdxType.find(remoteTypeId);
  if (it != remoteTypeIdToMergedPdxType.end()) {
    retVal = (*it).second;
    return retVal;
  }
  return retVal;
}

void PdxTypeRegistry::setPreserveData(PdxSerializablePtr obj,
                                      PdxRemotePreservedDataPtr pData,
                                      ExpiryTaskManager& expiryTaskManager) {
  WriteGuard guard(getPreservedDataLock());
  pData->setOwner(obj);
  if (preserveData.find(obj) != preserveData.end()) {
    // reset expiry task
    // TODO: check value for nullptr
    auto expTaskId = preserveData[obj]->getPreservedDataExpiryTaskId();
    expiryTaskManager.resetTask(expTaskId, 5);
    LOGDEBUG("PdxTypeRegistry::setPreserveData Reset expiry task Done");
    pData->setPreservedDataExpiryTaskId(expTaskId);
    preserveData[obj] = pData;
  } else {
    // schedule new expiry task
    auto handler = new PreservedDataExpiryHandler(shared_from_this(), obj, 20);
    long id = expiryTaskManager.scheduleExpiryTask(handler, 20, 0, false);
    pData->setPreservedDataExpiryTaskId(id);
    LOGDEBUG(
        "PdxTypeRegistry::setPreserveData Schedule new expirt task with id=%ld",
        id);
    preserveData.emplace(obj, pData);
  }

  LOGDEBUG(
      "PdxTypeRegistry::setPreserveData Successfully inserted new entry in "
      "preservedData");
}

PdxRemotePreservedDataPtr PdxTypeRegistry::getPreserveData(
    PdxSerializablePtr pdxobj) {
  ReadGuard guard(getPreservedDataLock());
  const auto& iter = preserveData.find((pdxobj));
  if (iter != preserveData.end()) {
    return iter->second;
  }
  return nullptr;
}

int32_t PdxTypeRegistry::getEnumValue(EnumInfoPtr ei) {
  // TODO locking - naive concurrent optimization?
  CacheableHashMapPtr tmp;
  tmp = enumToInt;
  const auto& entry = tmp->find(ei);
  if (entry != tmp->end()) {
    const auto val = std::static_pointer_cast<CacheableInt32>(entry->second);
    return val->value();
  }

  WriteGuard guard(g_readerWriterLock);
  tmp = enumToInt;
  const auto& entry2 = tmp->find(ei);
  if (entry2 != tmp->end()) {
    const auto val2 = std::static_pointer_cast<CacheableInt32>(entry2->second);
    return val2->value();
  }

  int val = static_cast<ThinClientPoolDM*>(
                cache->getPoolManager().getAll().begin()->second.get())
                ->GetEnumValue(ei);

  tmp = enumToInt;
  tmp->emplace(ei, CacheableInt32::create(val));
  enumToInt = tmp;
  return val;
}

EnumInfoPtr PdxTypeRegistry::getEnum(int32_t enumVal) {
  // TODO locking - naive concurrent optimization?
  EnumInfoPtr ret;
  CacheableHashMapPtr tmp;
  auto enumValPtr = CacheableInt32::create(enumVal);

  tmp = intToEnum;
  {
    const auto& entry = tmp->find(enumValPtr);
    if (entry != tmp->end()) {
      ret = std::static_pointer_cast<EnumInfo>(entry->second);
    }

    if (ret) {
      return ret;
    }
  }

  WriteGuard guard(g_readerWriterLock);
  tmp = intToEnum;
  {
    const auto& entry = tmp->find(enumValPtr);
    if (entry != tmp->end()) {
      ret = std::static_pointer_cast<EnumInfo>(entry->second);
    }

    if (ret) {
      return ret;
    }
  }

  ret = std::static_pointer_cast<EnumInfo>(
      static_cast<ThinClientPoolDM*>(
          cache->getPoolManager().getAll().begin()->second.get())
          ->GetEnum(enumVal));
  tmp = intToEnum;
  (*tmp)[enumValPtr] = ret;
  intToEnum = tmp;
  return ret;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
