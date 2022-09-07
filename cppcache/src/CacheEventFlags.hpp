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

#ifndef GEODE_CACHE_EVENT_FLAGS_H_
#define GEODE_CACHE_EVENT_FLAGS_H_

#include <cinttypes>

namespace apache {
namespace geode {
namespace client {


/**
 * @class CacheEventFlags
 *
 * This class encapsulates the flags (e.g. notification, expiration, local)
 * for cache events for various NoThrow methods.
 *
 *
 */
class CacheEventFlags {
 public:
  static const CacheEventFlags NORMAL;
  static const CacheEventFlags LOCAL;
  static const CacheEventFlags NOTIFICATION;
  static const CacheEventFlags NOTIFICATION_UPDATE;
  static const CacheEventFlags EVICTION;
  static const CacheEventFlags EXPIRATION;
  static const CacheEventFlags CACHE_CLOSE;
  static const CacheEventFlags NOCACHEWRITER;
  static const CacheEventFlags NOCALLBACKS;

   CacheEventFlags(const CacheEventFlags& flags) = default;

  CacheEventFlags() = delete;
  CacheEventFlags& operator=(const CacheEventFlags&) = delete;

  CacheEventFlags operator|(const CacheEventFlags& other) const {
    return CacheEventFlags(flags_ | other.flags_);
  }

  uint32_t operator&(const CacheEventFlags& other) const {
    return (flags_ & other.flags_);
  }

   bool operator==(const CacheEventFlags& other) const {
    return (flags_ == other.flags_);
  }

   bool isNormal() const { return flags_ & GF_NORMAL; }

   bool isLocal() const { return flags_ & GF_LOCAL; }

   bool isNotification() const { return flags_ & GF_NOTIFICATION; }

   bool isNotificationUpdate() const { return flags_ & GF_NOTIFICATION_UPDATE; }

   bool isEviction() const { return flags_ & GF_EVICTION; }

   bool isExpiration() const { return flags_ & GF_EXPIRATION; }

   bool isCacheClose() const { return flags_ & GF_CACHE_CLOSE; }

   bool isNoCacheWriter() const { return flags_ & GF_NOCACHEWRITER; }

   bool isNoCallbacks() const { return flags_ & GF_NOCALLBACKS; }

   bool isEvictOrExpire() const {
     return flags_ & (GF_EVICTION | GF_EXPIRATION);
   }

   // special optimized method for CacheWriter invocation condition
   bool invokeCacheWriter() const {
    return (flags_ & (GF_NOTIFICATION | GF_EVICTION | GF_EXPIRATION |
                        GF_NOCACHEWRITER | GF_NOCALLBACKS)) == 0;
  }

 protected:
  enum : uint16_t {
    GF_NORMAL = 0x01,
    GF_LOCAL = 0x02,
    GF_NOTIFICATION = 0x04,
    GF_NOTIFICATION_UPDATE = 0x08,
    GF_EVICTION = 0x10,
    GF_EXPIRATION = 0x20,
    GF_CACHE_CLOSE = 0x40,
    GF_NOCACHEWRITER = 0x80,
    GF_NOCALLBACKS = 0x100
  };

 protected:
  explicit CacheEventFlags(int16_t flags) : flags_(flags) {}

 protected:
  uint16_t flags_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHE_EVENT_FLAGS_H_
