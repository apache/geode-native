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

// C client public headers
#include "geode/pool/manager.h"

// C client private headers
#include "client.hpp"
#include "pool/factory.hpp"
#include "pool/manager.hpp"
#include "cache.hpp"

apache_geode_pool_factory_t* apache_geode_PoolManager_CreateFactory(
    apache_geode_pool_manager_t* poolManager) {
  PoolManagerWrapper* poolManagerWrapper =
      reinterpret_cast<PoolManagerWrapper*>(poolManager);
  return reinterpret_cast<apache_geode_pool_factory_t*>(
      poolManagerWrapper->CreatePoolFactory());
}

void apache_geode_DestroyPoolManager(apache_geode_pool_manager_t* poolManager) {
  PoolManagerWrapper* poolManagerWrapper =
      reinterpret_cast<PoolManagerWrapper*>(poolManager);
  delete poolManagerWrapper;
}

PoolManagerWrapper::PoolManagerWrapper(CacheWrapper *cache,
    apache::geode::client::PoolManager& poolManager)
    : poolManager_(poolManager) {
      AddRecord(this, "PoolManagerWrapper");
    }

PoolManagerWrapper::~PoolManagerWrapper() {
  RemoveRecord(this);
}

PoolFactoryWrapper* PoolManagerWrapper::CreatePoolFactory() {
  return new PoolFactoryWrapper(poolManager_.createFactory());
}
