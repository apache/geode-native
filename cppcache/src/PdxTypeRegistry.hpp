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

#ifndef GEODE_PDXTYPEREGISTRY_H_
#define GEODE_PDXTYPEREGISTRY_H_

#include <unordered_map>
#include <map>

#include <ace/ACE.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <geode/utils.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/Cache.hpp>

#include "PdxRemotePreservedData.hpp"
#include "ReadWriteLock.hpp"
#include "PdxType.hpp"
#include "EnumInfo.hpp"
#include "PreservedDataExpiryHandler.hpp"
#include "ExpiryTaskManager.hpp"

namespace apache {
namespace geode {
namespace client {

struct PdxTypeLessThan {
  bool operator()(PdxTypePtr const& n1, PdxTypePtr const& n2) const {
    // call to PdxType::operator <()
    return *n1 < *n2;
  }
};

typedef std::map<int32_t, PdxTypePtr> TypeIdVsPdxType;
typedef std::map</*char**/ std::string, PdxTypePtr> TypeNameVsPdxType;
typedef std::unordered_map<PdxSerializablePtr, PdxRemotePreservedDataPtr,
                           dereference_hash<CacheableKeyPtr>,
                           dereference_equal_to<CacheableKeyPtr>>
    PreservedHashMap;
typedef std::map<PdxTypePtr, int32_t, PdxTypeLessThan> PdxTypeToTypeIdMap;

class CPPCACHE_EXPORT PdxTypeRegistry
    : public std::enable_shared_from_this<PdxTypeRegistry> {
 private:
  Cache* cache;

  TypeIdVsPdxType typeIdToPdxType;

  TypeIdVsPdxType remoteTypeIdToMergedPdxType;

  TypeNameVsPdxType localTypeToPdxType;

  PdxTypeToTypeIdMap pdxTypeToTypeIdMap;

  // TODO:: preserveData need to be of type WeakHashMap
  PreservedHashMap preserveData;

  ACE_RW_Thread_Mutex g_readerWriterLock;

  ACE_RW_Thread_Mutex g_preservedDataLock;

  bool pdxIgnoreUnreadFields;

  bool pdxReadSerialized;

  CacheableHashMapPtr enumToInt;

  CacheableHashMapPtr intToEnum;

 public:
  PdxTypeRegistry(Cache* cache);
  PdxTypeRegistry(const PdxTypeRegistry& other) = delete;

  virtual ~PdxTypeRegistry();

  // test hook
  size_t testNumberOfPreservedData() const;

  void addPdxType(int32_t typeId, PdxTypePtr pdxType);

  PdxTypePtr getPdxType(int32_t typeId);

  void addLocalPdxType(const char* localType, PdxTypePtr pdxType);

  // newly added
  PdxTypePtr getLocalPdxType(const char* localType);

  void setMergedType(int32_t remoteTypeId, PdxTypePtr mergedType);

  PdxTypePtr getMergedType(int32_t remoteTypeId);

  void setPreserveData(PdxSerializablePtr obj,
                       PdxRemotePreservedDataPtr preserveDataPtr,
                       ExpiryTaskManager& expiryTaskManager);

  PdxRemotePreservedDataPtr getPreserveData(PdxSerializablePtr obj);

  void clear();

  int32_t getPDXIdForType(const char* type, const char* poolname,
                          PdxTypePtr nType, bool checkIfThere);

  bool getPdxIgnoreUnreadFields() const { return pdxIgnoreUnreadFields; }

  void setPdxIgnoreUnreadFields(bool value) { pdxIgnoreUnreadFields = value; }

  void setPdxReadSerialized(bool value) { pdxReadSerialized = value; }

  bool getPdxReadSerialized() const { return pdxReadSerialized; }

  inline PreservedHashMap& getPreserveDataMap() { return preserveData; };

  int32_t getEnumValue(EnumInfoPtr ei);

  EnumInfoPtr getEnum(int32_t enumVal);

  int32_t getPDXIdForType(PdxTypePtr nType, const char* poolname);

  ACE_RW_Thread_Mutex& getPreservedDataLock() { return g_preservedDataLock; }
};

typedef std::shared_ptr<PdxTypeRegistry> PdxTypeRegistryPtr;

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPEREGISTRY_H_
