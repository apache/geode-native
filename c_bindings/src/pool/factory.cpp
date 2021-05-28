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

// Standard headers
#include <cstdint>
#include <string>

// C client public headers
#include "geode/pool/factory.h"

// C client private headers
#include "pool.hpp"
#include "pool/factory.hpp"
#include "pool/manager.hpp"

apache_geode_pool_t* apache_geode_PoolFactory_CreatePool(
    apache_geode_pool_factory_t* poolFactory, const char* name) {
  PoolFactoryWrapper* poolFactoryWrapper =
      reinterpret_cast<PoolFactoryWrapper*>(poolFactory);
  return reinterpret_cast<apache_geode_pool_t*>(
      poolFactoryWrapper->CreatePool(name));
}

void apache_geode_PoolFactory_AddLocator(
    apache_geode_pool_factory_t* poolFactory, const char* hostname,
    const std::uint16_t port) {
  PoolFactoryWrapper* poolFactoryWrapper =
      reinterpret_cast<PoolFactoryWrapper*>(poolFactory);
  poolFactoryWrapper->AddLocator(hostname, port);
}

void apache_geode_DestroyPoolFactory(apache_geode_pool_factory_t* poolFactory) {
  PoolFactoryWrapper* poolFactoryWrapper =
      reinterpret_cast<PoolFactoryWrapper*>(poolFactory);
  delete poolFactoryWrapper;
}

PoolFactoryWrapper::PoolFactoryWrapper(apache::geode::client::PoolFactory poolFactory)
    : poolFactory_(poolFactory) {
  AddRecord(this, "PoolFactoryWrapper");
}

PoolFactoryWrapper::~PoolFactoryWrapper() { RemoveRecord(this); }

PoolWrapper* PoolFactoryWrapper::CreatePool(const char* name) {
  return new PoolWrapper(poolFactory_.create(name));
}

void PoolFactoryWrapper::AddLocator(const std::string& hostname,
                                    const std::uint16_t port) {
  poolFactory_.addLocator(hostname, port);
}
