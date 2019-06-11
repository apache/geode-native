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

#include "PdxRemotePreservedData.hpp"

namespace apache {
namespace geode {
namespace client {
PdxRemotePreservedData::PdxRemotePreservedData()
    : m_typeId(0), m_mergedTypeId(0), m_currentIndex(0), m_expiryTakId(0) {}

PdxRemotePreservedData::~PdxRemotePreservedData() {}

PdxRemotePreservedData::PdxRemotePreservedData(
    int32_t typeId, int32_t mergedTypeId, std::shared_ptr<Serializable> owner) {
  m_typeId = typeId;
  m_mergedTypeId = mergedTypeId;
  m_currentIndex = 0;
  m_owner = owner;
  m_expiryTakId = 0;
}

void PdxRemotePreservedData::initialize(int32_t typeId, int32_t mergedTypeId,
                                        std::shared_ptr<Serializable> owner) {
  m_typeId = typeId;
  m_mergedTypeId = mergedTypeId;
  m_currentIndex = 0;
  m_owner = owner;
  m_expiryTakId = 0;
}

int32_t PdxRemotePreservedData::getMergedTypeId() { return m_mergedTypeId; }

void PdxRemotePreservedData::setPreservedDataExpiryTaskId(
    ExpiryTaskManager::id_type expId) {
  m_expiryTakId = expId;
}

ExpiryTaskManager::id_type
PdxRemotePreservedData::getPreservedDataExpiryTaskId() {
  return m_expiryTakId;
}

std::shared_ptr<Serializable> PdxRemotePreservedData::getOwner() {
  return m_owner;
}

void PdxRemotePreservedData::setOwner(std::shared_ptr<Serializable> val) {
  m_owner = val;
}

std::vector<int8_t> PdxRemotePreservedData::getPreservedData(int32_t idx) {
  return m_preservedData[idx];
}

void PdxRemotePreservedData::setPreservedData(std::vector<int8_t> inputVector) {
  m_preservedData.push_back(inputVector);
}

bool PdxRemotePreservedData::equals(std::shared_ptr<Serializable> otherObject) {
  if (otherObject == nullptr) return false;

  if (m_owner == nullptr) return false;

  return m_owner == otherObject;
}

int PdxRemotePreservedData::GetHashCode() {
  if (m_owner != nullptr) {
    // TODO
    return 1;  // m_owner->GetHashCode();
  }
  return 0;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
