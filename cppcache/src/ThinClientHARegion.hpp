#pragma once

#ifndef GEODE_THINCLIENTHAREGION_H_
#define GEODE_THINCLIENTHAREGION_H_

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

#include <geode/Pool.hpp>

#include "ThinClientRegion.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class ThinHAClientRegion ThinHAClientRegion.hpp
 *
 * This class manages the interest list functionalities related with
 * native client regions supporting java HA queues.
 *
 * It inherits from ThinClientRegion and overrides interest list
 * send and invalidate methods.
 *
 */
class APACHE_GEODE_EXPORT ThinClientHARegion : public ThinClientRegion {
 public:
  /**
   * @brief constructor/destructor
   */
  ThinClientHARegion(const std::string& name, CacheImpl* cache,
                     const std::shared_ptr<RegionInternal>& rPtr,
                     RegionAttributes attributes,
                     const std::shared_ptr<CacheStatistics>& stats,
                     bool shared = false, bool enableNotification = true);

  ~ThinClientHARegion() noexcept override = default;

  void initTCR() override;

  bool getProcessedMarker() override;

  void setProcessedMarker(bool mark = true) override {
    m_processedMarker = mark;
  }
  void addDisMessToQueue() override;

 protected:
  GfErrType getNoThrow_FullObject(
      std::shared_ptr<EventId> eventId, std::shared_ptr<Cacheable>& fullObject,
      std::shared_ptr<VersionTag>& versionTag) override;

 private:
  RegionAttributes m_attributes;
  volatile bool m_processedMarker;
  void handleMarker() override;

  // Disallow copy constructor and assignment operator.
  ThinClientHARegion(const ThinClientHARegion&);
  ThinClientHARegion& operator=(const ThinClientHARegion&);

  void acquireGlobals(bool isFailover) override;
  void releaseGlobals(bool isFailover) override;

  void destroyDM(bool keepEndpoints) override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTHAREGION_H_
