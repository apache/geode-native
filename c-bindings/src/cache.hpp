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

#include "geode/Cache.hpp"
#include "client.hpp"


namespace apache {
  namespace geode {
    namespace client {
      enum class RegionShortcut;
    }
  }
}

class PoolManagerWrapper;
class RegionFactoryWrapper;
class CacheFactoryWrapper;

class CacheWrapper: public ClientKeeper {
  apache::geode::client::Cache cache_;

 public:
  explicit CacheWrapper(apache::geode::client::Cache);
  ~CacheWrapper();

  bool getPdxIgnoreUnreadFields();

  bool getPdxReadSerialized();

  PoolManagerWrapper* getPoolManager();

  RegionFactoryWrapper* createRegionFactory(
      apache::geode::client::RegionShortcut regionShortcut);

  const char* getName();

  void close(bool keepalive);

  bool isClosed();
};
