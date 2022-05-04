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

#include <boost/thread/shared_mutex.hpp>

#include <geode/Cache.hpp>
#include <geode/internal/functional.hpp>

#include "EnumInfo.hpp"
#include "ExpiryTaskManager.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxSerializable;
class PdxType;
class PdxUnreadData;

typedef std::map<int32_t, std::shared_ptr<PdxType>> TypeIdVsPdxType;
typedef std::map<std::string, std::shared_ptr<PdxType>> TypeNameVsPdxType;

typedef std::unordered_map<std::shared_ptr<PdxType>, int32_t,
                           dereference_hash<std::shared_ptr<PdxType>>,
                           dereference_equal_to<std::shared_ptr<PdxType>>>
    PdxTypeToTypeIdMap;

class PdxUnreadDataExpiryTask;

class APACHE_GEODE_EXPORT PdxTypeRegistry
    : public std::enable_shared_from_this<PdxTypeRegistry> {
 private:
  using UnreadDataMap =
      std::unordered_map<std::shared_ptr<PdxSerializable>,
                         std::shared_ptr<PdxUnreadData>,
                         dereference_hash<std::shared_ptr<CacheableKey>>,
                         dereference_equal_to<std::shared_ptr<CacheableKey>>>;

 public:
  explicit PdxTypeRegistry(CacheImpl* cache);
  PdxTypeRegistry(const PdxTypeRegistry& other) = delete;

  virtual ~PdxTypeRegistry();

  // test hook
  size_t testNumberOfPreservedData() const;

  void setUnreadData(ExpiryTaskManager& expiryTaskManager,
                     std::shared_ptr<PdxSerializable> obj,
                     std::shared_ptr<PdxUnreadData> data);

  void removeUnreadData(std::shared_ptr<PdxSerializable> obj);

  std::shared_ptr<PdxUnreadData> getUnreadData(
      std::shared_ptr<PdxSerializable> obj) const;

  void clear();

  /**
   * Returns the PdxType corresponding the given ID.
   * If not present on the cache, it fetches it
   * from the cluster and add it to the cache
   * @param typeId PdxType ID to get
   * @param pool Pointer to the pool to be used when
   * fetching the PdxType from the cluster, if needed.
   * @return PdxType corresponding to the given ID
   */
  std::shared_ptr<PdxType> getPdxType(int32_t typeId, Pool* pool);

  /**
   * Registers a PdxType if not present on the registry
   * @param pdxType PdxType to verify
   * @param pool Pointer to the pool to use while registering the PdxType, if
   * needed
   * @return true if the PdxType needed to be registered or if PdxType ID was
   * needed to be updated, and false if it was already present
   */
  bool registerPdxTypeIfNeeded(std::shared_ptr<PdxType> pdxType, Pool* pool);

  bool getPdxIgnoreUnreadFields() const { return pdxIgnoreUnreadFields_; }

  void setPdxIgnoreUnreadFields(bool value) { pdxIgnoreUnreadFields_ = value; }

  void setPdxReadSerialized(bool value) { pdxReadSerialized_ = value; }

  bool getPdxReadSerialized() const { return pdxReadSerialized_; }

  int32_t getEnumValue(std::shared_ptr<EnumInfo> ei);

  std::shared_ptr<EnumInfo> getEnum(int32_t enumVal);

 private:
  CacheImpl* cache_;

  TypeIdVsPdxType typeIdToPdxType_;

  TypeIdVsPdxType remoteTypeIdToMergedPdxType_;

  TypeNameVsPdxType localTypeToPdxType_;

  PdxTypeToTypeIdMap pdxTypeToTypeIdMap_;

  UnreadDataMap unreadData_;

  mutable boost::shared_mutex typesMutex_;

  mutable boost::shared_mutex unreadDataMutex_;

  bool pdxIgnoreUnreadFields_;

  bool pdxReadSerialized_;

  std::shared_ptr<CacheableHashMap> enumToInt_;

  std::shared_ptr<CacheableHashMap> intToEnum_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPEREGISTRY_H_
