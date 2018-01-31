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

#include <geode/AttributesMutator.hpp>

#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

AttributesMutator::AttributesMutator(const std::shared_ptr<Region>& region)
    : m_region(region) {}

AttributesMutator::~AttributesMutator() { m_region = nullptr; }

std::chrono::seconds AttributesMutator::setEntryIdleTimeout(
    std::chrono::seconds idleTimeout) {
  return std::static_pointer_cast<RegionInternal>(m_region)
      ->adjustEntryExpiryDuration(idleTimeout);
}

ExpirationAction AttributesMutator::setEntryIdleTimeoutAction(
    ExpirationAction action) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustEntryExpiryAction(action);
}

std::chrono::seconds AttributesMutator::setEntryTimeToLive(
    std::chrono::seconds timeToLive) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustEntryExpiryDuration(timeToLive);
}

ExpirationAction AttributesMutator::setEntryTimeToLiveAction(
    ExpirationAction action) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustEntryExpiryAction(action);
}

std::chrono::seconds AttributesMutator::setRegionIdleTimeout(
    std::chrono::seconds idleTimeout) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustRegionExpiryDuration(idleTimeout);
}

ExpirationAction AttributesMutator::setRegionIdleTimeoutAction(
    ExpirationAction action) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustRegionExpiryAction(action);
}

std::chrono::seconds AttributesMutator::setRegionTimeToLive(
    std::chrono::seconds timeToLive) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustRegionExpiryDuration(timeToLive);
}

ExpirationAction AttributesMutator::setRegionTimeToLiveAction(
    ExpirationAction action) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustRegionExpiryAction(action);
}

uint32_t AttributesMutator::setLruEntriesLimit(uint32_t entriesLimit) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  return rImpl->adjustLruEntriesLimit(entriesLimit);
}

void AttributesMutator::setCacheListener(
    const std::shared_ptr<CacheListener>& aListener) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  rImpl->adjustCacheListener(aListener);
}

void AttributesMutator::setCacheListener(const std::string& libpath,
                                         const std::string& factoryFuncName) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  rImpl->adjustCacheListener(libpath.c_str(), factoryFuncName.c_str());
}

void AttributesMutator::setCacheLoader(
    const std::shared_ptr<CacheLoader>& aLoader) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  rImpl->adjustCacheLoader(aLoader);
}

void AttributesMutator::setCacheLoader(const std::string& libpath,
                                       const std::string& factoryFuncName) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  rImpl->adjustCacheLoader(libpath.c_str(), factoryFuncName.c_str());
}

void AttributesMutator::setCacheWriter(
    const std::shared_ptr<CacheWriter>& aWriter) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  rImpl->adjustCacheWriter(aWriter);
}

void AttributesMutator::setCacheWriter(const std::string& libpath,
                                       const std::string& factoryFuncName) {
  auto rImpl = std::static_pointer_cast<RegionInternal>(m_region);
  rImpl->adjustCacheWriter(libpath.c_str(), factoryFuncName.c_str());
}
}  // namespace client
}  // namespace geode
}  // namespace apache
