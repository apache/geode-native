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

#ifndef GEODE_THINCLIENTPOOLREGION_H_
#define GEODE_THINCLIENTPOOLREGION_H_

#include "ThinClientHARegion.hpp"

namespace apache {
namespace geode {
namespace client {
class ThinClientPoolRegion : public ThinClientRegion {
 public:
  ThinClientPoolRegion(const std::string& name, CacheImpl* cache,
                       const std::shared_ptr<RegionInternal>& rPtr,
                       RegionAttributes attributes,
                       const std::shared_ptr<CacheStatistics>& stats,
                       bool shared = false);

  ThinClientPoolRegion(const ThinClientPoolRegion&) = delete;
  ThinClientPoolRegion& operator=(const ThinClientPoolRegion&) = delete;

  void initTCR() override;
  ~ThinClientPoolRegion() noexcept override = default;

 private:
  void destroyDM(bool keepEndpoints) override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTPOOLREGION_H_
