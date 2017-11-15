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
#include <RegionInternal.hpp>

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

ExpirationAction::Action AttributesMutator::setEntryIdleTimeoutAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryAction(action);
}

std::chrono::seconds AttributesMutator::setEntryTimeToLive(
    std::chrono::seconds timeToLive) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryDuration(timeToLive);
}

ExpirationAction::Action AttributesMutator::setEntryTimeToLiveAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryAction(action);
}

std::chrono::seconds AttributesMutator::setRegionIdleTimeout(
    std::chrono::seconds idleTimeout) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryDuration(idleTimeout);
}

ExpirationAction::Action AttributesMutator::setRegionIdleTimeoutAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryAction(action);
}

std::chrono::seconds AttributesMutator::setRegionTimeToLive(
    std::chrono::seconds timeToLive) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryDuration(timeToLive);
}

ExpirationAction::Action AttributesMutator::setRegionTimeToLiveAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryAction(action);
}

uint32_t AttributesMutator::setLruEntriesLimit(uint32_t entriesLimit) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustLruEntriesLimit(entriesLimit);
}

void AttributesMutator::setCacheListener(
    const std::shared_ptr<CacheListener>& aListener) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  rImpl->adjustCacheListener(aListener);
}

void AttributesMutator::setCacheListener(const char* libpath,
                                         const char* factoryFuncName) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  rImpl->adjustCacheListener(libpath, factoryFuncName);
}

void AttributesMutator::setCacheLoader(
    const std::shared_ptr<CacheLoader>& aLoader) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  rImpl->adjustCacheLoader(aLoader);
}

void AttributesMutator::setCacheLoader(const char* libpath,
                                       const char* factoryFuncName) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  rImpl->adjustCacheLoader(libpath, factoryFuncName);
}

void AttributesMutator::setCacheWriter(
    const std::shared_ptr<CacheWriter>& aWriter) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  rImpl->adjustCacheWriter(aWriter);
}

void AttributesMutator::setCacheWriter(const char* libpath,
                                       const char* factoryFuncName) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  rImpl->adjustCacheWriter(libpath, factoryFuncName);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
