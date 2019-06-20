#pragma once

#ifndef GEODE_PDXREMOTEPRESERVEDDATA_H_
#define GEODE_PDXREMOTEPRESERVEDDATA_H_

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

#include <vector>

#include <geode/PdxUnreadFields.hpp>
#include <geode/Serializable.hpp>

#include "ExpiryTaskManager.hpp"

namespace apache {
namespace geode {
namespace client {
class PdxRemotePreservedData;

class PdxRemotePreservedData : public PdxUnreadFields {
 private:
  std::vector<std::vector<int8_t> > m_preservedData;
  int32_t m_typeId;
  int32_t m_mergedTypeId;
  int32_t m_currentIndex;
  std::shared_ptr<Serializable> /*Object^*/ m_owner;
  ExpiryTaskManager::id_type m_expiryTakId;

 public:
  PdxRemotePreservedData();

  virtual ~PdxRemotePreservedData() = default;

  PdxRemotePreservedData(int32_t typeId, int32_t mergedTypeId,
                         std::shared_ptr<Serializable> owner);

  void initialize(int32_t typeId, int32_t mergedTypeId,
                  std::shared_ptr<Serializable> owner);

  int32_t getMergedTypeId();

  void setPreservedDataExpiryTaskId(ExpiryTaskManager::id_type expId);

  ExpiryTaskManager::id_type getPreservedDataExpiryTaskId();

  std::shared_ptr<Serializable> getOwner();

  void setOwner(std::shared_ptr<Serializable> val);

  std::vector<int8_t> getPreservedData(int32_t idx);

  void setPreservedData(std::vector<int8_t> inputVector);

  virtual bool equals(std::shared_ptr<Serializable> otherObject);

  virtual int GetHashCode();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREMOTEPRESERVEDDATA_H_
