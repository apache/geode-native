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

#ifndef GEODE_TOMBSTONEEXPIRYTASK_H_
#define GEODE_TOMBSTONEEXPIRYTASK_H_

#include <geode/ExpirationAction.hpp>
#include <geode/Region.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ExpiryTask.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

class MapSegment;
class TombstoneEntry;

/**
 * @class TombstoneExpiryTask TombstoneExpiryTask.hpp
 *
 * The task which gets triggered when a tombstone expires.
 */
class TombstoneExpiryTask : public ExpiryTask {
 public:
  /**
   * Class constructor
   * @param manager A reference to the expiry manager
   * @param segment A reference to the MapSegment the entry sits in
   * @param tombstone Tombstone entry
   */
  TombstoneExpiryTask(ExpiryTaskManager& manager, MapSegment& segment,
                      std::shared_ptr<TombstoneEntry> tombstone);

 protected:
  bool on_expire() override;

 private:
  /// Member attributes

  /**
   * Reference to the map segment in which the entry is
   */
  MapSegment& segment_;

  /**
   * Tombstone entry
   */
  std::shared_ptr<TombstoneEntry> tombstone_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TOMBSTONEEXPIRYTASK_H_
