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

#ifndef GEODE_MAPENTRY_H_
#define GEODE_MAPENTRY_H_

#include <atomic>
#include <memory>
#include <utility>

#include <geode/CacheableKey.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/geode_globals.hpp>

#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

class ExpEntryProperties;
class LRUEntryProperties;
class MapEntry;
class MapEntryImpl;
class VersionStamp;
class LRUEntryProperties;

/**
 * @brief Interface class for region mapped entry value.
 */
class MapEntry {
 public:
  virtual ~MapEntry() noexcept = default;

  virtual void getKey(std::shared_ptr<CacheableKey>& result) const = 0;
  virtual void getValue(std::shared_ptr<Cacheable>& result) const = 0;
  virtual void setValue(const std::shared_ptr<Cacheable>& value) = 0;
  virtual std::shared_ptr<MapEntryImpl> getImplPtr() = 0;

  virtual LRUEntryProperties& getLRUProperties() = 0;
  virtual ExpEntryProperties& getExpProperties() = 0;
  virtual VersionStamp& getVersionStamp() = 0;

  /**
   * Adds a tracker to this MapEntry for any updates.
   * Returns the current update sequence number of this entry.
   * Returns by reference any updated MapEntry to be rebound while an
   * unchanged return value implies this MapEntry should be continued with.
   * Note that the contract says that the return value should not be
   * touched if this MapEntry is to be continued with.
   *
   * The design of tracking is thus:
   * Each entry will have two fields, one indicating the current number of
   * operations tracking this entry, and second indicating the sequence
   * number for number of updates since tracking was started. The idea is
   * to start tracking an entry before going for a long remote operation
   * and check whether the update sequence has remained unchanged upon
   * return. If the entry has been updated in the interim period then
   * the current update is not applied. When the number of operations
   * tracking an entry goes down to zero, then the update sequence number
   * is also reset to zero.
   */
  virtual int addTracker(std::shared_ptr<MapEntry>& newEntry) = 0;

  /**
   * Removes a tracker for this MapEntry and returns a pair:
   *  1) the first element of the pair is a boolean that indicates whether
   *     or not the entry should be replaced with the underlying MapEntryImpl
   *     object in the map
   *  2) the second element is the updated number of trackers for this entry
   */
  virtual std::pair<bool, int> removeTracker() = 0;

  /**
   * Increment the number of updates to this entry.
   * Returns the current update sequence number of this entry.
   * Returns by reference any updated MapEntry to be rebound while an
   * unchanged return value implies this MapEntry should be continued with.
   * Note that the contract says that the return value should not be
   * touched if this MapEntry is to be continued with.
   */
  virtual int incrementUpdateCount(std::shared_ptr<MapEntry>& newEntry) = 0;

  /**
   * Get the current tracking number of this entry. A return value of zero
   * indicates that the entry is not being tracked.
   */
  virtual int getTrackingNumber() const = 0;

  /**
   * Get the current number of updates since tracking was started for
   * this entry.
   */
  virtual int getUpdateCount() const = 0;

  /**
   * Any cleanup required (e.g. removing from LRUList) for the entry.
   */
  virtual void cleanup(const CacheEventFlags eventFlags) = 0;

 protected:
  inline MapEntry() = default;

  inline explicit MapEntry(bool) {}
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_MAPENTRY_H_
