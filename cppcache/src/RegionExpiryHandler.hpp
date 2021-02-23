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

#ifndef GEODE_REGIONEXPIRYHANDLER_H_
#define GEODE_REGIONEXPIRYHANDLER_H_

#include <geode/ExpirationAction.hpp>
#include <geode/Region.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ExpiryTaskManager.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class RegionExpiryHandler RegionExpiryHandler.hpp
 *
 * The task object which contains the handler which gets triggered
 * when a region expires.
 *
 * TODO: cleanup region entry node and handler from expiry task
 * manager when region is destroyed
 *
 */
class RegionExpiryHandler : public ACE_Event_Handler {
 public:
  RegionExpiryHandler(std::shared_ptr<RegionInternal>& rptr,
                      ExpirationAction action, std::chrono::seconds duration);

  int handle_timeout(const ACE_Time_Value& current_time,
                     const void* arg) override;

  int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask) override;

  void setExpiryTaskId(ExpiryTaskManager::id_type expiryTaskId) {
    m_expiryTaskId = expiryTaskId;
  }

 private:
  std::shared_ptr<RegionInternal> m_regionPtr;
  ExpirationAction m_action;
  std::chrono::seconds m_duration;
  ExpiryTaskManager::id_type m_expiryTaskId;
  // perform the actual expiration action
  void DoTheExpirationAction();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONEXPIRYHANDLER_H_
