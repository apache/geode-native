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

#ifndef GEODE_ENTRYEVENT_H_
#define GEODE_ENTRYEVENT_H_

#include <memory>

#include "CacheableKey.hpp"
#include "Region.hpp"
#include "geode/Serializable.hpp"
#include "internal/geode_globals.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

class CacheableKey;
class Region;
class Serializable;

/** Represents an entry event affecting an entry, including its identity and the
 * the circumstances of the event. */

class APACHE_GEODE_EXPORT EntryEvent {
 protected:
  std::shared_ptr<Region> m_region;      /**< Region */
  std::shared_ptr<CacheableKey> m_key;   /**< Cacheable key */
  std::shared_ptr<Cacheable> m_oldValue; /**< Old value */
  std::shared_ptr<Cacheable> m_newValue; /**< New value */
  std::shared_ptr<Serializable>
      m_callbackArgument; /**< Callback argument for this event, if any. */
  bool m_remoteOrigin;    /**< True if from a remote (non-local) process */

 public:
  /** Constructor, given all values. */
  EntryEvent(const std::shared_ptr<Region>& region,
             const std::shared_ptr<CacheableKey>& key,
             const std::shared_ptr<Cacheable>& oldValue,
             const std::shared_ptr<Cacheable>& newValue,
             const std::shared_ptr<Serializable>& aCallbackArgument,
             const bool remoteOrigin);

  /** Destructor. */
  virtual ~EntryEvent();

  /** Constructor. */
  EntryEvent();

  /** @return the region this event occurred in. */
  inline std::shared_ptr<Region> getRegion() const { return m_region; }

  /** @return the key this event describes. */
  inline std::shared_ptr<CacheableKey> getKey() const { return m_key; }

  /** If the prior state of the entry was invalid, or non-existent/destroyed,
   * then the old value will be nullptr.
   * @return the old value in the cache.
   */
  inline std::shared_ptr<Cacheable> getOldValue() const { return m_oldValue; }

  /** If the event is a destroy or invalidate operation, then the new value
   * will be nullptr.
   * @return the updated value from this event
   */
  inline std::shared_ptr<Cacheable> getNewValue() const { return m_newValue; }

  /**
   * Returns the callbackArgument passed to the method that generated
   * this event. See the {@link Region} interface methods that take
   * a callbackArgument parameter.
   */
  inline std::shared_ptr<Serializable> getCallbackArgument() const {
    return m_callbackArgument;
  }

  /** If the event originated in a remote process, returns true. */
  inline bool remoteOrigin() const { return m_remoteOrigin; }

 private:
  // never implemented.
  EntryEvent(const EntryEvent& other);
  void operator=(const EntryEvent& other);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_ENTRYEVENT_H_
