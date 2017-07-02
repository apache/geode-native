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

#include "PdxRemotePreservedData.hpp"
#include "ReadWriteLock.hpp"
#include "PdxType.hpp"
#include "EnumInfo.hpp"
#include "PreservedDataExpiryHandler.hpp"

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

class CPPCACHE_EXPORT PdxTypeRegistry {
 private:
  static TypeIdVsPdxType* typeIdToPdxType;

  static TypeIdVsPdxType* remoteTypeIdToMergedPdxType;

  static TypeNameVsPdxType* localTypeToPdxType;

  // TODO:: preserveData need to be of type WeakHashMap
  // static std::map<PdxSerializablePtr , PdxRemotePreservedDataPtr>
  // *preserveData;
  // static CacheableHashMapPtr preserveData;
  static PreservedHashMap preserveData;

  static ACE_RW_Thread_Mutex g_readerWriterLock;

  static ACE_RW_Thread_Mutex g_preservedDataLock;

  static bool pdxIgnoreUnreadFields;

  static bool pdxReadSerialized;

  static CacheableHashMapPtr enumToInt;

  static CacheableHashMapPtr intToEnum;

 public:
  PdxTypeRegistry();

  virtual ~PdxTypeRegistry();

  static void init();

  static void cleanup();

  // test hook;
  static size_t testGetNumberOfPdxIds();

  // test hook
  static size_t testNumberOfPreservedData();

  static void addPdxType(int32_t typeId, PdxTypePtr pdxType);

  static PdxTypePtr getPdxType(int32_t typeId);

  static void addLocalPdxType(const char* localType, PdxTypePtr pdxType);

  // newly added
  static PdxTypePtr getLocalPdxType(const char* localType);

  static void setMergedType(int32_t remoteTypeId, PdxTypePtr mergedType);

  static PdxTypePtr getMergedType(int32_t remoteTypeId);

  static void setPreserveData(PdxSerializablePtr obj,
                              PdxRemotePreservedDataPtr preserveDataPtr);

  static PdxRemotePreservedDataPtr getPreserveData(PdxSerializablePtr obj);

  static void clear();

  static int32_t getPDXIdForType(const char* type, const char* poolname,
                                 PdxTypePtr nType, bool checkIfThere);

  static bool getPdxIgnoreUnreadFields() { return pdxIgnoreUnreadFields; }

  static void setPdxIgnoreUnreadFields(bool value) {
    pdxIgnoreUnreadFields = value;
  }

  static void setPdxReadSerialized(bool value) { pdxReadSerialized = value; }

  static bool getPdxReadSerialized() { return pdxReadSerialized; }

  static inline PreservedHashMap& getPreserveDataMap() { return preserveData; };

  static int32_t getEnumValue(EnumInfoPtr ei);

  static EnumInfoPtr getEnum(int32_t enumVal);

  static int32_t getPDXIdForType(PdxTypePtr nType, const char* poolname);

  static ACE_RW_Thread_Mutex& getPreservedDataLock() {
    return g_preservedDataLock;
  }

 private:
  static PdxTypeToTypeIdMap* pdxTypeToTypeIdMap;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPEREGISTRY_H_
