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
#include <memory>

// C++ public headers
#include "geode/Pool.hpp"

// C client public headers
#include "geode/pool.h"

// C client private headers
#include "pool.hpp"
#include "pool/factory.hpp"

void apache_geode_DestroyPool(apache_geode_pool_t* pool) {
  PoolWrapper* poolWrapper = reinterpret_cast<PoolWrapper*>(pool);
  delete poolWrapper;
}

PoolWrapper::PoolWrapper(std::shared_ptr<apache::geode::client::Pool> pool)
    : pool_(pool) {
  AddRecord(this, "PoolWrapper");
}

PoolWrapper::~PoolWrapper() { RemoveRecord(this); }