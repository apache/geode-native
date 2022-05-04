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

#include "PdxUnreadDataExpiryTask.hpp"

#include "ExpiryTaskManager.hpp"
#include "PdxTypeRegistry.hpp"
#include "PdxUnreadData.hpp"

namespace apache {
namespace geode {
namespace client {

PdxUnreadDataExpiryTask::PdxUnreadDataExpiryTask(
    ExpiryTaskManager& manager, decltype(registry_) type_registry,
    decltype(object_) object)
    : ExpiryTask(manager), registry_(type_registry), object_(object) {}

bool PdxUnreadDataExpiryTask::on_expire() {

  LOGDEBUG("Entered PdxUnreadDataExpiryTask::on_expire");

  auto ud = registry_->getUnreadData(object_);
  auto expiresAt = ud->expiresAt();
  if (expiresAt < ExpiryTask::clock_t::now()) {
    LOGDEBUG("Re-scheduling PdxUnreadDataExpiryTask with ID %zu", id());
    reset(expiresAt);
    return false;
  }

  registry_->removeUnreadData(object_);
  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
