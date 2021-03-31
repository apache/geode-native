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

#ifndef GEODE_TOMBSTONELIST_H_
#define GEODE_TOMBSTONELIST_H_

#include <chrono>
#include <memory>
#include <unordered_map>

#include <geode/CacheableBuiltins.hpp>
#include <geode/internal/functional.hpp>

#include "ExpiryTask.hpp"
#include "MapEntry.hpp"

namespace apache {
namespace geode {
namespace client {

class MapSegment;
class TombstoneEntry;

class TombstoneList {
 public:
  TombstoneList(MapSegment& segment, CacheImpl& cache)
      : segment_(segment), cache_(cache) {}
  virtual ~TombstoneList() { cleanup(); }

  ExpiryTask::id_t add(const std::shared_ptr<MapEntryImpl>& entry);
  bool erase(const std::shared_ptr<CacheableKey>& key, bool cancel_task = true);
  bool exists(const std::shared_ptr<CacheableKey>& key) const;
  void cleanup();

  // Reaps the tombstones which have been gc'ed on server.
  // A map that has identifier for ClientProxyMembershipID as key
  // and server version of the tombstone with highest version as the
  // value is passed as paramter
  void reap_tombstones(std::map<uint16_t, int64_t>& gcVersions);
  void reap_tombstones(const std::shared_ptr<CacheableHashSet>& keys);

 protected:
  using tombstone_map_t =
      std::unordered_map<std::shared_ptr<CacheableKey>,
                         std::shared_ptr<TombstoneEntry>,
                         dereference_hash<std::shared_ptr<CacheableKey>>,
                         dereference_equal_to<std::shared_ptr<CacheableKey>>>;

 protected:
  tombstone_map_t tombstones_;
  MapSegment& segment_;
  CacheImpl& cache_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TOMBSTONELIST_H_
