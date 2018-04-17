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

#ifndef GEODE_FWKLIB_REGIONHELPER_H_
#define GEODE_FWKLIB_REGIONHELPER_H_

#include <cstdlib>
#include <string>
#include <map>

#include <geode/internal/chrono/duration.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "fwklib/FrameworkTest.hpp"
#include "fwklib/FwkObjects.hpp"
#include "fwklib/FwkStrCvt.hpp"
#include "fwklib/FwkLog.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testframework {

// ----------------------------------------------------------------------------

/** @class RegionHelper
 * @brief Class used to define a valid combination of attributes and
 * specifications for a region. Since some combinations are illegal,
 * this gives a test an easier way to consider all valid combinations.
 */
class RegionHelper {
  const FwkRegion* m_region;
  std::string m_spec;

 public:
  /** Fill in this instance of RegionHelper based on the spec named by sname.
   */
  RegionHelper(const FrameworkTest* test) : m_region(NULL) {
    m_spec = test->getStringValue("regionSpec");
    if (m_spec.empty()) {
      FWKEXCEPTION("Failed to find regionSpec definition.");
    }
    m_region = test->getSnippet(m_spec);
    if (m_region == NULL) {
      FWKEXCEPTION("Failed to find region definition.");
    }
  }

  std::string regionAttributesToString() {
    auto atts = m_region->getRegionAttributes();
    return regionAttributesToString(atts);
  }

  std::string regionName() { return m_region->getName(); }

  const std::string specName() { return m_spec; }

  std::string regionTag() {
    auto atts = m_region->getRegionAttributes();
    return regionTag(atts);
  }

  static std::string regionTag(RegionAttributes regionAttributes) {
    std::string sString;

    sString += regionAttributes.getCachingEnabled() ? "Caching" : "NoCache";
    sString +=
        (regionAttributes.getCacheListener() == nullptr) ? "Nlstnr" : "Lstnr";
    return sString;
  }

  static std::string to_string(ExpirationAction expirationAction) {
    switch (expirationAction) {
      case ExpirationAction::DESTROY:
        return "DESTROY";
      case ExpirationAction::INVALIDATE:
        return "INVALIDATE";
      case ExpirationAction::LOCAL_INVALIDATE:
        return "LOCAL_INVALIDATE";
      case ExpirationAction::LOCAL_DESTROY:
        return "LOCAL_DESTROY";
      case ExpirationAction::INVALID_ACTION:
        return "INVALID_ACTION";
    }
    throw std::invalid_argument("expirationAction is uknown.");
  }

  /** @brief Given RegionAttributes, return a string logging its configuration.
   *  @param attr Return a string describing this region.
   *  @retval A String representing aRegion.
   */
  static std::string regionAttributesToString(
      RegionAttributes regionAttributes) {
    std::string sString;

    sString += "\ncaching: ";
    sString += regionAttributes.getCachingEnabled() ? "Enabled" : "Disabled";
    sString += "\nendpoints: ";
    sString += regionAttributes.getEndpoints();
    sString += "\nclientNotification: ";
    sString += regionAttributes.getClientNotificationEnabled() ? "Enabled"
                                                               : "Disabled";
    sString += "\ninitialCapacity: ";
    sString += std::to_string(regionAttributes.getInitialCapacity());
    sString += "\nloadFactor: ";
    sString += std::to_string(regionAttributes.getLoadFactor());
    sString += "\nconcurrencyLevel: ";
    sString += std::to_string(regionAttributes.getConcurrencyLevel());
    sString += "\nlruEntriesLimit: ";
    sString += std::to_string(regionAttributes.getLruEntriesLimit());
    sString += "\nlruEvictionAction: ";
    sString += to_string(regionAttributes.getLruEvictionAction());
    sString += "\nentryTimeToLive: ";
    sString += apache::geode::internal::chrono::duration::to_string(
        regionAttributes.getEntryTimeToLive());
    sString += "\nentryTimeToLiveAction: ";
    sString += to_string(regionAttributes.getEntryTimeToLiveAction());
    sString += "\nentryIdleTimeout: ";
    sString += apache::geode::internal::chrono::duration::to_string(
        regionAttributes.getEntryIdleTimeout());
    sString += "\nentryIdleTimeoutAction: ";
    sString += to_string(regionAttributes.getEntryIdleTimeoutAction());
    sString += "\nregionTimeToLive: ";
    sString += apache::geode::internal::chrono::duration::to_string(
        regionAttributes.getRegionTimeToLive());
    sString += "\nregionTimeToLiveAction: ";
    sString += to_string(regionAttributes.getRegionTimeToLiveAction());
    sString += "\nregionIdleTimeout: ";
    sString += apache::geode::internal::chrono::duration::to_string(
        regionAttributes.getRegionIdleTimeout());
    sString += "\nregionIdleTimeoutAction: ";
    sString += to_string(regionAttributes.getRegionIdleTimeoutAction());
    sString += "\npoolName: ";
    sString += regionAttributes.getPoolName();
    sString += "\nCacheLoader: ";
    sString += (regionAttributes.getCacheLoaderLibrary().empty() ||
                regionAttributes.getCacheLoaderFactory().empty())
                   ? "Disabled"
                   : "Enabled";
    sString += "\nCacheWriter: ";
    sString += (regionAttributes.getCacheWriterLibrary().empty() ||
                regionAttributes.getCacheWriterFactory().empty())
                   ? "Disabled"
                   : "Enabled";
    sString += "\nCacheListener: ";
    sString += (regionAttributes.getCacheListenerLibrary().empty() ||
                regionAttributes.getCacheListenerFactory().empty())
                   ? "Disabled"
                   : "Enabled";
    sString += "\nConcurrencyChecksEnabled: ";
    sString +=
        regionAttributes.getConcurrencyChecksEnabled() ? "Enabled" : "Disabled";
    sString += "\n";

    return sString;
  }
  void setRegionAttributes(RegionFactory& regionFac) {
    auto regionAttributes = m_region->getRegionAttributes();
    regionFac.setCachingEnabled(regionAttributes.getCachingEnabled());
    if (!(regionAttributes.getCacheListenerLibrary().empty() ||
          regionAttributes.getCacheListenerFactory().empty())) {
      regionFac.setCacheListener(regionAttributes.getCacheListenerLibrary(),
                                 regionAttributes.getCacheListenerFactory());
    }
    if (!(regionAttributes.getCacheLoaderLibrary().empty() ||
          regionAttributes.getCacheLoaderFactory().empty())) {
      regionFac.setCacheLoader(regionAttributes.getCacheLoaderLibrary(),
                               regionAttributes.getCacheLoaderFactory());
    }
    if (!(regionAttributes.getCacheWriterLibrary().empty() ||
          regionAttributes.getCacheWriterFactory().empty())) {
      regionFac.setCacheWriter(regionAttributes.getCacheWriterLibrary(),
                               regionAttributes.getCacheWriterFactory());
    }
    if (regionAttributes.getEntryIdleTimeout().count() != 0) {
      regionFac.setEntryIdleTimeout(
          regionAttributes.getEntryIdleTimeoutAction(),
          regionAttributes.getEntryIdleTimeout());
    }
    if (regionAttributes.getEntryTimeToLive().count() != 0) {
      regionFac.setEntryTimeToLive(regionAttributes.getEntryTimeToLiveAction(),
                                   regionAttributes.getEntryTimeToLive());
    }
    if (regionAttributes.getRegionIdleTimeout().count() != 0) {
      regionFac.setRegionIdleTimeout(
          regionAttributes.getRegionIdleTimeoutAction(),
          regionAttributes.getRegionIdleTimeout());
    }
    if (regionAttributes.getRegionTimeToLive().count() != 0) {
      regionFac.setRegionTimeToLive(
          regionAttributes.getRegionTimeToLiveAction(),
          regionAttributes.getRegionTimeToLive());
    }
    if (!(regionAttributes.getPartitionResolverLibrary().empty() ||
          regionAttributes.getPartitionResolverFactory().empty())) {
      regionFac.setPartitionResolver(
          regionAttributes.getPartitionResolverLibrary(),
          regionAttributes.getPartitionResolverFactory());
    }
    if (!(regionAttributes.getPersistenceLibrary().empty() ||
          regionAttributes.getPersistenceFactory().empty())) {
      regionFac.setPersistenceManager(
          regionAttributes.getPersistenceLibrary(),
          regionAttributes.getPersistenceFactory(),
          regionAttributes.getPersistenceProperties());
    }
    regionFac.setInitialCapacity(regionAttributes.getInitialCapacity());
    regionFac.setLoadFactor(regionAttributes.getLoadFactor());
    regionFac.setConcurrencyLevel(regionAttributes.getConcurrencyLevel());
    regionFac.setLruEntriesLimit(regionAttributes.getLruEntriesLimit());
    regionFac.setDiskPolicy(regionAttributes.getDiskPolicy());
    regionFac.setCloningEnabled(regionAttributes.getCloningEnabled());
    regionFac.setPoolName(regionAttributes.getPoolName());
    regionFac.setConcurrencyChecksEnabled(
        regionAttributes.getConcurrencyChecksEnabled());
  }

  std::shared_ptr<Region> createRootRegion(std::shared_ptr<Cache>& cachePtr) {
    std::shared_ptr<Region> region;
    std::string regionName = m_region->getName();
    if (regionName.empty()) {
      FWKEXCEPTION("Region name not specified.");
    }
    return createRootRegion(cachePtr, regionName);
  }

  std::shared_ptr<Region> createRootRegion(std::shared_ptr<Cache>& cachePtr,
                                           std::string regionName) {
    if (regionName.empty()) {
      regionName = m_region->getName();
      FWKINFO("region name is " << regionName);
      if (regionName.empty()) {
        FWKEXCEPTION("Region name not specified.");
      }
    }
    auto regionFac =
        cachePtr->createRegionFactory(RegionShortcut::CACHING_PROXY);
    setRegionAttributes(regionFac);
    auto regionAttributes = m_region->getRegionAttributes();
    const auto& poolName = regionAttributes.getPoolName();
    auto region = regionFac.create(regionName.c_str());
    FWKINFO("Region created with name = " << regionName + " and pool name= "
                                          << poolName);
    FWKINFO(" Region Created with following attributes :"
            << regionAttributesToString());
    return region;
  }
};

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

// ----------------------------------------------------------------------------

#endif  // GEODE_FWKLIB_REGIONHELPER_H_
