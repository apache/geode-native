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

/** Sets the idleTimeout duration for region entries.
 * @param idleTimeout the idleTimeout in seconds for entries in this region.
 * @return the previous value.
 * @throw IllegalStateException if the new idleTimeout changes entry expiration
 * from
 *   disabled to enabled or enabled to disabled.
 */
int32_t AttributesMutator::setEntryIdleTimeout(int32_t idleTimeout) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryDuration(idleTimeout);
}

/** Set the idleTimeout Action for region entries.
 * @param action the idleTimeout ExpirationAction::Action for entries in this
 * region.
 * @return the previous value.
 */
ExpirationAction::Action AttributesMutator::setEntryIdleTimeoutAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryAction(action);
}

/** Sets the timeToLive duration for region entries.
 * @param timeToLive the timeToLive in seconds for entries in this region.
 * @return the previous value.
 * @throw IllegalStateException if the new timeToLive changes entry expiration
 * from
 *   disabled to enabled or enabled to disabled.
 */
int32_t AttributesMutator::setEntryTimeToLive(int32_t timeToLive) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryDuration(timeToLive);
}

/** Set the timeToLive Action for region entries.
 * @param action the timeToLive ExpirationAction::Action for entries in this
 * region.
 * @return the previous value.
 */
ExpirationAction::Action AttributesMutator::setEntryTimeToLiveAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustEntryExpiryAction(action);
}

/** Sets the idleTimeout duration for the region itself.
 * @param idleTimeout the ExpirationAttributes for this region idleTimeout
 * @return the previous value.
 * @throw IllegalStateException if the new idleTimeout changes region expiration
 * from
 *   disabled to enabled or enabled to disabled.
 */
int32_t AttributesMutator::setRegionIdleTimeout(int32_t idleTimeout) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryDuration(idleTimeout);
}

/** Set the idleTimeout Action for the region itself.
 * @param action the idleTimeout ExpirationAction::Action for this region.
 * @return the previous value.
 */
ExpirationAction::Action AttributesMutator::setRegionIdleTimeoutAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryAction(action);
}

/** Sets the timeToLive duration for the region itself.
 * @param timeToLive the ExpirationAttributes for this region timeToLive
 * @return the previous value.
 * @throw IllegalStateException if the new timeToLive changes region expiration
 * from
 *   disabled to enabled or enabled to disabled.
 */
int32_t AttributesMutator::setRegionTimeToLive(int32_t timeToLive) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryDuration(timeToLive);
}

/** Set the timeToLive Action for the region itself.
 * @param action the timeToLive ExpirationAction::Action for this region.
 * @return the previous value.
 */
ExpirationAction::Action AttributesMutator::setRegionTimeToLiveAction(
    ExpirationAction::Action action) {
  RegionInternal* rImpl = dynamic_cast<RegionInternal*>(m_region.get());
  return rImpl->adjustRegionExpiryAction(action);
}

/** Sets the Maximum entry count in the region before LRU eviction.
 * @param entriesLimit the number of entries to allow.
 * @return the previous value.
 * @throw IllegalStateException if the new entriesLimit changes LRU from
 *   disabled to enabled or enabled to disabled.
 */
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
