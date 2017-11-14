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

#include <geode/util/chrono/duration.hpp>

#include "fwklib/FrameworkTest.hpp"
#include "fwklib/FwkObjects.hpp"
#include "fwklib/FwkStrCvt.hpp"
#include "fwklib/FwkLog.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testframework {

using namespace apache::geode::util::chrono::duration;

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
    auto atts = m_region->getAttributesPtr();
    return regionAttributesToString(atts);
  }

  std::string regionName() { return m_region->getName(); }

  const std::string specName() { return m_spec; }

  std::string regionTag() {
    auto atts = m_region->getAttributesPtr();
    return regionTag(atts);
  }

  static std::string regionTag(std::shared_ptr<RegionAttributes> attr) {
    std::string sString;

    sString += attr->getCachingEnabled() ? "Caching" : "NoCache";
    sString += (attr->getCacheListener() == nullptr) ? "Nlstnr" : "Lstnr";
    return sString;
  }

  /** @brief Given RegionAttributes, return a string logging its configuration.
    *  @param attr Return a string describing this region.
    *  @retval A String representing aRegion.
    */
  static std::string regionAttributesToString(
      std::shared_ptr<RegionAttributes>& attr) {
    std::string sString;

    sString += "\ncaching: ";
    sString += attr->getCachingEnabled() ? "Enabled" : "Disabled";
    sString += "\nendpoints: ";
    sString += FwkStrCvt(attr->getEndpoints()).toString();
    sString += "\nclientNotification: ";
    sString += attr->getClientNotificationEnabled() ? "Enabled" : "Disabled";
    sString += "\ninitialCapacity: ";
    sString += FwkStrCvt(attr->getInitialCapacity()).toString();
    sString += "\nloadFactor: ";
    sString += FwkStrCvt(attr->getLoadFactor()).toString();
    sString += "\nconcurrencyLevel: ";
    sString += FwkStrCvt(attr->getConcurrencyLevel()).toString();
    sString += "\nlruEntriesLimit: ";
    sString += FwkStrCvt(attr->getLruEntriesLimit()).toString();
    sString += "\nlruEvictionAction: ";
    sString += ExpirationAction::fromOrdinal(attr->getLruEvictionAction());
    sString += "\nentryTimeToLive: ";
    sString += to_string(attr->getEntryTimeToLive());
    sString += "\nentryTimeToLiveAction: ";
    sString += ExpirationAction::fromOrdinal(attr->getEntryTimeToLiveAction());
    sString += "\nentryIdleTimeout: ";
    sString += to_string(attr->getEntryIdleTimeout());
    sString += "\nentryIdleTimeoutAction: ";
    sString += ExpirationAction::fromOrdinal(attr->getEntryIdleTimeoutAction());
    sString += "\nregionTimeToLive: ";
    sString += to_string(attr->getRegionTimeToLive());
    sString += "\nregionTimeToLiveAction: ";
    sString += ExpirationAction::fromOrdinal(attr->getRegionTimeToLiveAction());
    sString += "\nregionIdleTimeout: ";
    sString += to_string(attr->getRegionIdleTimeout());
    sString += "\nregionIdleTimeoutAction: ";
    sString +=
        ExpirationAction::fromOrdinal(attr->getRegionIdleTimeoutAction());
    sString += "\npoolName: ";
    sString += FwkStrCvt(attr->getPoolName()).toString();
    sString += "\nCacheLoader: ";
    sString += (attr->getCacheLoaderLibrary() != NULL &&
                attr->getCacheLoaderFactory() != NULL)
                   ? "Enabled"
                   : "Disabled";
    sString += "\nCacheWriter: ";
    sString += (attr->getCacheWriterLibrary() != NULL &&
                attr->getCacheWriterFactory() != NULL)
                   ? "Enabled"
                   : "Disabled";
    sString += "\nCacheListener: ";
    sString += (attr->getCacheListenerLibrary() != NULL &&
                attr->getCacheListenerFactory() != NULL)
                   ? "Enabled"
                   : "Disabled";
    sString += "\nConcurrencyChecksEnabled: ";
    sString += attr->getConcurrencyChecksEnabled() ? "Enabled" : "Disabled";
    sString += "\n";

    return sString;
  }
  void setRegionAttributes(RegionFactory& regionFac) {
    auto atts = m_region->getAttributesPtr();
    regionFac.setCachingEnabled(atts->getCachingEnabled());
    if (atts->getCacheListenerLibrary() != NULL &&
        atts->getCacheListenerFactory() != NULL) {
      regionFac.setCacheListener(atts->getCacheListenerLibrary(),
                                 atts->getCacheListenerFactory());
    }
    if (atts->getCacheLoaderLibrary() != NULL &&
        atts->getCacheLoaderFactory() != NULL) {
      regionFac.setCacheLoader(atts->getCacheLoaderLibrary(),
                               atts->getCacheLoaderFactory());
    }
    if (atts->getCacheWriterLibrary() != NULL &&
        atts->getCacheWriterFactory() != NULL) {
      regionFac.setCacheWriter(atts->getCacheWriterLibrary(),
                               atts->getCacheWriterFactory());
    }
    if (atts->getEntryIdleTimeout().count() != 0) {
      regionFac.setEntryIdleTimeout(atts->getEntryIdleTimeoutAction(),
                                    atts->getEntryIdleTimeout());
    }
    if (atts->getEntryTimeToLive().count() != 0) {
      regionFac.setEntryTimeToLive(atts->getEntryTimeToLiveAction(),
                                   atts->getEntryTimeToLive());
    }
    if (atts->getRegionIdleTimeout().count() != 0) {
      regionFac.setRegionIdleTimeout(atts->getRegionIdleTimeoutAction(),
                                     atts->getRegionIdleTimeout());
    }
    if (atts->getRegionTimeToLive().count() != 0) {
      regionFac.setRegionTimeToLive(atts->getRegionTimeToLiveAction(),
                                    atts->getRegionTimeToLive());
    }
    if (atts->getPartitionResolverLibrary() != NULL &&
        atts->getPartitionResolverFactory() != NULL) {
      regionFac.setPartitionResolver(atts->getPartitionResolverLibrary(),
                                     atts->getPartitionResolverFactory());
    }
    if (atts->getPersistenceLibrary() != NULL &&
        atts->getPersistenceFactory() != NULL) {
      regionFac.setPersistenceManager(atts->getPersistenceLibrary(),
                                      atts->getPersistenceFactory(),
                                      atts->getPersistenceProperties());
    }
    regionFac.setInitialCapacity(atts->getInitialCapacity());
    regionFac.setLoadFactor(atts->getLoadFactor());
    regionFac.setConcurrencyLevel(atts->getConcurrencyLevel());
    regionFac.setLruEntriesLimit(atts->getLruEntriesLimit());
    regionFac.setDiskPolicy(atts->getDiskPolicy());
    regionFac.setCloningEnabled(atts->getCloningEnabled());
    regionFac.setPoolName(atts->getPoolName());
    regionFac.setConcurrencyChecksEnabled(atts->getConcurrencyChecksEnabled());
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
    auto regionFac = cachePtr->createRegionFactory(CACHING_PROXY);
    setRegionAttributes(regionFac);
    auto atts = m_region->getAttributesPtr();
    bool withPool = (NULL != atts->getPoolName()) ? true : false;
    std::string poolName;
    if (withPool) {
      poolName = atts->getPoolName();
    } else {
      poolName = "";
    }
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
