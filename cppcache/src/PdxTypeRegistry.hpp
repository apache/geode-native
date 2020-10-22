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

#include <map>
#include <unordered_map>

#include <geode/Cache.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/internal/functional.hpp>

#include "EnumInfo.hpp"
#include "ExpiryTaskManager.hpp"
#include "PdxRemotePreservedData.hpp"
#include "PdxType.hpp"
#include "ReadWriteLock.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<int32_t, std::shared_ptr<PdxType>> TypeIdVsPdxType;
typedef std::map<std::string, std::shared_ptr<PdxType>> TypeNameVsPdxType;
typedef std::unordered_map<std::shared_ptr<PdxSerializable>,
                           std::shared_ptr<PdxRemotePreservedData>,
                           dereference_hash<std::shared_ptr<CacheableKey>>,
                           dereference_equal_to<std::shared_ptr<CacheableKey>>>
    PreservedHashMap;

typedef std::unordered_map<std::shared_ptr<PdxType>, int32_t,
                           dereference_hash<std::shared_ptr<PdxType>>,
                           dereference_equal_to<std::shared_ptr<PdxType>>>
    PdxTypeToTypeIdMap;

class PreservedDataExpiryTask;

class APACHE_GEODE_EXPORT PdxTypeRegistry
    : public std::enable_shared_from_this<PdxTypeRegistry> {
 private:
  CacheImpl* cache_;

  TypeIdVsPdxType typeIdToPdxType_;

  TypeIdVsPdxType remoteTypeIdToMergedPdxType_;

  TypeNameVsPdxType localTypeToPdxType_;

  PdxTypeToTypeIdMap pdxTypeToTypeIdMap_;

  // TODO:: preserveData need to be of type WeakHashMap
  PreservedHashMap preserved_data_;

  mutable ACE_RW_Thread_Mutex g_readerWriterLock_;

  mutable ACE_RW_Thread_Mutex g_preservedDataLock_;

  bool pdxIgnoreUnreadFields_;

  bool pdxReadSerialized_;

  std::shared_ptr<CacheableHashMap> enumToInt_;

  std::shared_ptr<CacheableHashMap> intToEnum_;

 public:
  explicit PdxTypeRegistry(CacheImpl* cache);
  PdxTypeRegistry(const PdxTypeRegistry& other) = delete;

  virtual ~PdxTypeRegistry();

  // test hook
  size_t testNumberOfPreservedData() const;

  void addPdxType(int32_t typeId, std::shared_ptr<PdxType> pdxType);

  std::shared_ptr<PdxType> getPdxType(int32_t typeId) const;

  void addLocalPdxType(const std::string& localType,
                       std::shared_ptr<PdxType> pdxType);

  // newly added
  std::shared_ptr<PdxType> getLocalPdxType(const std::string& localType) const;

  void setMergedType(int32_t remoteTypeId, std::shared_ptr<PdxType> mergedType);

  std::shared_ptr<PdxType> getMergedType(int32_t remoteTypeId) const;

  void setPreserveData(std::shared_ptr<PdxSerializable> obj,
                       std::shared_ptr<PdxRemotePreservedData> data,
                       ExpiryTaskManager& expiryTaskManager);

  std::shared_ptr<PdxRemotePreservedData> getPreserveData(
      std::shared_ptr<PdxSerializable> obj) const;

  void clear();

  int32_t getPDXIdForType(const std::string& type, Pool* pool,
                          std::shared_ptr<PdxType> nType, bool checkIfThere);

  bool getPdxIgnoreUnreadFields() const { return pdxIgnoreUnreadFields_; }

  void setPdxIgnoreUnreadFields(bool value) { pdxIgnoreUnreadFields_ = value; }

  void setPdxReadSerialized(bool value) { pdxReadSerialized_ = value; }

  bool getPdxReadSerialized() const { return pdxReadSerialized_; }

  const PreservedHashMap& getPreserveDataMap() const { return preserved_data_; }

  int32_t getEnumValue(std::shared_ptr<EnumInfo> ei);

  std::shared_ptr<EnumInfo> getEnum(int32_t enumVal);

  int32_t getPDXIdForType(std::shared_ptr<PdxType> nType, Pool* pool);

  ACE_RW_Thread_Mutex& getPreservedDataLock() const {
    return g_preservedDataLock_;
  }

 protected:
  friend class PreservedDataExpiryTask;

  PreservedHashMap& preserved_data_map() { return preserved_data_; }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPEREGISTRY_H_
