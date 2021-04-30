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

#ifndef GEODE_ADMINREGION_H_
#define GEODE_ADMINREGION_H_

#include <memory>
#include <string>

#include <geode/Serializable.hpp>

#include "ErrType.hpp"
#include "ReadWriteLock.hpp"

namespace apache {
namespace geode {

namespace statistics {

class HostStatSampler;

}  // namespace statistics

namespace client {

class CacheImpl;
class ThinClientBaseDM;
class TcrConnectionManager;
class CacheableKey;

class AdminRegion : public std::enable_shared_from_this<AdminRegion> {
  ThinClientBaseDM* dist_mgr_;
  std::string full_path_;
  TcrConnectionManager* connection_mgr_;
  boost::shared_mutex mutex_;
  bool destroy_pending_;

  GfErrType putNoThrow(const std::shared_ptr<CacheableKey>& keyPtr,
                       const std::shared_ptr<Cacheable>& valuePtr);

 public:
  AdminRegion(const AdminRegion&) = delete;
  AdminRegion& operator=(const AdminRegion&) = delete;

  AdminRegion()
      : dist_mgr_(nullptr),
        full_path_("/__ADMIN_CLIENT_HEALTH_MONITORING__"),
        connection_mgr_(nullptr),
        destroy_pending_(false) {}
  ~AdminRegion();

  static std::shared_ptr<AdminRegion> create(
      CacheImpl* cache, ThinClientBaseDM* distMan = nullptr);
  boost::shared_lock<boost::shared_mutex> make_shared_lock();
  const bool& isDestroyed();
  void close();
  void init();
  void put(const std::shared_ptr<CacheableKey>& keyPtr,
           const std::shared_ptr<Cacheable>& valuePtr);
  TcrConnectionManager* getConnectionManager();
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_ADMINREGION_H_
