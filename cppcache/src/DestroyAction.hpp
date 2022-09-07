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

#ifndef GEODE_DESTROY_ACTION_H_
#define GEODE_DESTROY_ACTION_H_

#include <memory>

#include "CacheEventFlags.hpp"
#include "ErrType.hpp"
#include "EventType.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheableKey;
class DataInput;
class EventId;
class LocalRegion;
class MapEntryImpl;
class Serializable;
class TXState;
class VersionTag;

// encapsulates actions that need to be taken for a destroy() operation
class DestroyAction {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_DESTROY;
  static const EntryEventType s_afterEventType = AFTER_DESTROY;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = false;

  explicit DestroyAction(LocalRegion& region);

  void getCallbackOldValue(bool cachingEnabled,
                           const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<MapEntryImpl>& entry,
                           std::shared_ptr<Serializable>& oldValue) const;

  GfErrType remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                         const std::shared_ptr<Serializable>& value,
                         const std::shared_ptr<Serializable>& cbArg,
                         std::shared_ptr<Serializable>& retValue,
                         std::shared_ptr<VersionTag>& versionTag);

  GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Serializable>& value,
                        std::shared_ptr<Serializable>& oldValue,
                        bool cachingEnabled, const CacheEventFlags eventFlags,
                        int updateCount, std::shared_ptr<VersionTag> versionTag,
                        DataInput* delta = nullptr,
                        std::shared_ptr<EventId> eventId = nullptr,
                        bool afterRemote = false);

  TXState* getTxState() const { return txState_; }

 public:
  static const char* name() { return "Region::destroy"; }

  static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Serializable>& value,
                             DataInput* delta);

  static void logCacheWriterFailure(const std::shared_ptr<CacheableKey>& key,
                                    const std::shared_ptr<Serializable>& oldValue);

 protected:
  LocalRegion& region_;
  TXState* txState_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DESTROY_ACTION_H_
