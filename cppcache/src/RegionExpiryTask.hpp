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

#ifndef GEODE_REGIONEXPIRYTASK_H_
#define GEODE_REGIONEXPIRYTASK_H_

#include <geode/ExpirationAction.hpp>
#include <geode/Region.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ExpiryTask.hpp"
#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class RegionExpiryTask RegionExpiryTask.hpp
 *
 * The task object which contains the handler which gets triggered
 * when a region expires.
 */
class RegionExpiryTask : public ExpiryTask {
 public:
  RegionExpiryTask(ExpiryTaskManager& manager,
                   std::shared_ptr<RegionInternal> region,
                   ExpirationAction action, const duration_t& duration);

 protected:
  bool on_expire() override;
  time_point_t expire_at() const;

 protected:
  std::shared_ptr<RegionInternal> region_;
  ExpiryTask::duration_t duration_;
  ExpirationAction action_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONEXPIRYTASK_H_
