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

#include <ace/RW_Thread_Mutex.h>

#include <geode/CacheableKey.hpp>
#include <geode/Serializable.hpp>

#include "ErrType.hpp"

namespace apache {
namespace geode {

namespace statistics {

class HostStatSampler;

}  // namespace statistics

namespace client {

class CacheImpl;
class ThinClientBaseDM;
class TcrConnectionManager;

class AdminRegion : public std::enable_shared_from_this<AdminRegion> {
  friend class apache::geode::statistics::HostStatSampler;

  ThinClientBaseDM* m_distMngr;
  std::string m_fullPath;
  TcrConnectionManager* m_connectionMgr;
  ACE_RW_Thread_Mutex m_rwLock;
  bool m_destroyPending;

  AdminRegion(const AdminRegion&) = delete;
  AdminRegion& operator=(const AdminRegion&) = delete;

  GfErrType putNoThrow(const std::shared_ptr<CacheableKey>& keyPtr,
                       const std::shared_ptr<Cacheable>& valuePtr);
  TcrConnectionManager* getConnectionManager();

 public:
  AdminRegion();
  ~AdminRegion();

  static std::shared_ptr<AdminRegion> create(
      CacheImpl* cache, ThinClientBaseDM* distMan = nullptr);
  ACE_RW_Thread_Mutex& getRWLock();
  const bool& isDestroyed();
  void close();
  void init();
  void put(const std::shared_ptr<CacheableKey>& keyPtr,
           const std::shared_ptr<Cacheable>& valuePtr);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_ADMINREGION_H_
