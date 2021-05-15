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
/*
 * PreservedDataExpiryTask.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: npatel
 */
#include "PreservedDataExpiryTask.hpp"

#include "ExpiryTaskManager.hpp"
#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

PreservedDataExpiryTask::PreservedDataExpiryTask(
    ExpiryTaskManager& manager, decltype(type_registry_) type_registry,
    decltype(object_) object)
    : ExpiryTask(manager), type_registry_(type_registry), object_(object) {}

bool PreservedDataExpiryTask::on_expire() {
  WriteGuard guard(type_registry_->getPreservedDataLock());
  auto& map = type_registry_->preserved_data_map();

  LOG_DEBUG(
      "Entered PreservedDataExpiryTask "
      "PdxTypeRegistry::getPreserveDataMap().size() = %zu",
      map.size());

  auto&& iter = map.find(object_);
  if (iter == map.end()) {
    return true;
  }

  auto expires_at = iter->second->expires_at();
  if (expires_at < ExpiryTask::clock_t::now()) {
    LOG_DEBUG("Re-scheduling PreservedDataExpiryTask with ID %zu", id());

    reset(expires_at);
    return false;
  }

  map.erase(iter);
  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
