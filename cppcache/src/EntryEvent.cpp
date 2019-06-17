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
#include <geode/EntryEvent.hpp>

#include "CacheableToken.hpp"

namespace apache {
namespace geode {
namespace client {
EntryEvent::EntryEvent(const std::shared_ptr<Region>& region,
                       const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Cacheable>& oldValue,
                       const std::shared_ptr<Cacheable>& newValue,
                       const std::shared_ptr<Serializable>& aCallbackArgument,
                       const bool remoteOrigin)
    : m_region(region),
      m_key(key),
      m_oldValue(oldValue),
      m_newValue(newValue),
      m_callbackArgument(aCallbackArgument),
      m_remoteOrigin(remoteOrigin) {}

EntryEvent::~EntryEvent() {}

EntryEvent::EntryEvent()
    /* adongre
     * CID 28923: Uninitialized scalar field (UNINIT_CTOR)
     */
    : m_remoteOrigin(false) {}

std::shared_ptr<Region> EntryEvent::getRegion() const { return m_region; }

/** @return the key this event describes. */
std::shared_ptr<CacheableKey> EntryEvent::getKey() const { return m_key; }

/** If the prior state of the entry was invalid, or non-existent/destroyed,
 * then the old value will be nullptr.
 * @return the old value in the cache.
 */
std::shared_ptr<Cacheable> EntryEvent::getOldValue() const {
  return m_oldValue;
}

/** If the event is a destroy or invalidate operation, then the new value
 * will be nullptr.
 * @return the updated value from this event
 */
std::shared_ptr<Cacheable> EntryEvent::getNewValue() const {
  return m_newValue;
}

/**
 * Returns the callbackArgument passed to the method that generated
 * this event. See the {@link Region} interface methods that take
 * a callbackArgument parameter.
 */
std::shared_ptr<Serializable> EntryEvent::getCallbackArgument() const {
  return m_callbackArgument;
}

/** If the event originated in a remote process, returns true. */
bool EntryEvent::remoteOrigin() const { return m_remoteOrigin; }
}  // namespace client
}  // namespace geode
}  // namespace apache
