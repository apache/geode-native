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

#ifndef GEODE_PRESERVEDDATAEXPIRYTASK_H_
#define GEODE_PRESERVEDDATAEXPIRYTASK_H_

#include <geode/Cache.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheImpl.hpp"
#include "ExpiryTask.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class PdxUnreadDataExpiryTask
 *
 * The expiry task which gets triggered when a preserved data expires.
 */
class PdxUnreadDataExpiryTask : public ExpiryTask {
 public:
  /**
   * Constructor
   */
  PdxUnreadDataExpiryTask(ExpiryTaskManager& manager,
                          std::shared_ptr<PdxTypeRegistry> type_registry,
                          std::shared_ptr<PdxSerializable> object);

  bool on_expire() override;

 private:
  std::shared_ptr<PdxTypeRegistry> registry_;
  std::shared_ptr<PdxSerializable> object_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PRESERVEDDATAEXPIRYTASK_H_
