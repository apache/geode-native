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

#ifndef GEODE_PUT_ACTION_H_
#define GEODE_PUT_ACTION_H_

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

// encapsulates actions that need to be taken for a put() operation
class PutAction {
 public:
  static const EntryEventType s_beforeEventType = BEFORE_UPDATE;
  static const EntryEventType s_afterEventType = AFTER_UPDATE;
  static const bool s_addIfAbsent = true;
  static const bool s_failIfPresent = false;

 public:
  explicit PutAction(LocalRegion& region);

  void getCallbackOldValue(bool cachingEnabled,
                           const std::shared_ptr<CacheableKey>& key,
                           std::shared_ptr<MapEntryImpl>& entry,
                           std::shared_ptr<Serializable>& oldValue) const;

  GfErrType remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                         const std::shared_ptr<Serializable>& value,
                         const std::shared_ptr<Serializable>& cbArg,
                         std::shared_ptr<Serializable>& retVal,
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
  static const char* name() { return "Region::put"; }

  static void logCacheWriterFailure(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<Serializable>& oldValue);

  static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Serializable>& value,
                             DataInput* delta = nullptr);

 protected:
  LocalRegion& region_;
  TXState* txState_;
};

// encapsulates actions that need to be taken for a put() operation. This
// implementation allows
// null values in Put during transaction. See defect #743
class PutActionTx : public PutAction {
 public:
  explicit PutActionTx(LocalRegion& region);
  static GfErrType checkArgs(const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Serializable>& value,
                             DataInput* delta = nullptr);
};

class PutIfAbsentAction : public PutAction {
 public:
  explicit PutIfAbsentAction(LocalRegion& region);

  GfErrType localUpdate(const std::shared_ptr<CacheableKey>& key,
                        const std::shared_ptr<Serializable>& value,
                        std::shared_ptr<Serializable>& oldValue,
                        bool cachingEnabled, const CacheEventFlags eventFlags,
                        int updateCount, std::shared_ptr<VersionTag> versionTag,
                        DataInput* delta = nullptr,
                        std::shared_ptr<EventId> eventId = nullptr,
                        bool afterRemote = false);

  GfErrType remoteUpdate(const std::shared_ptr<CacheableKey>& key,
                         const std::shared_ptr<Serializable>& value,
                         const std::shared_ptr<Serializable>& cbArg,
                         std::shared_ptr<Serializable>& retValue,
                         std::shared_ptr<VersionTag>& versionTag);

  static const char* name() { return "Region::putIfAbsent"; }

 protected:
  bool absent_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PUT_ACTION_H_
