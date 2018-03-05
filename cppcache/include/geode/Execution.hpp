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

#ifndef GEODE_EXECUTIONIMPL_H_
#define GEODE_EXECUTIONIMPL_H_

#include <map>

#include <ace/Condition_Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <geode/CacheableBuiltins.hpp>
#include <geode/ResultCollector.hpp>
#include <geode/Region.hpp>

#include "../../src/ProxyCache.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<std::string, std::vector<int8_t>*> FunctionToFunctionAttributes;

class Execution {
 public:
  Execution(std::shared_ptr<Region> rptr = nullptr,
                std::shared_ptr<ProxyCache> proxyCache = nullptr,
                std::shared_ptr<Pool> pp = nullptr)
      : m_routingObj(nullptr),
        m_args(nullptr),
        m_rc(nullptr),
        m_region(rptr),
        m_allServer(false),
        m_pool(pp),
        m_proxyCache(proxyCache) {}
  Execution(std::shared_ptr<Pool> pool, bool allServer = false,
                std::shared_ptr<ProxyCache> proxyCache = nullptr)
      : m_routingObj(nullptr),
        m_args(nullptr),
        m_rc(nullptr),
        m_region(nullptr),
        m_allServer(allServer),
        m_pool(pool),
        m_proxyCache(proxyCache) {}
  Execution& withFilter(
          std::shared_ptr<CacheableVector> routingObj);
  Execution& withArgs(
          std::shared_ptr<Cacheable> args);
  Execution& withCollector(
          std::shared_ptr<ResultCollector> rs);
  // java function has hasResult property. we put the hasResult argument
  // here as a kluge.
  std::shared_ptr<ResultCollector> execute(
      const std::shared_ptr<CacheableVector>& routingObj,
      const std::shared_ptr<Cacheable>& args,
      const std::shared_ptr<ResultCollector>& rs, const std::string& func,
      std::chrono::milliseconds timeout);

  std::shared_ptr<ResultCollector> execute(
      const std::string& func, std::chrono::milliseconds timeout =
                                   DEFAULT_QUERY_RESPONSE_TIMEOUT);

  static void addResults(std::shared_ptr<ResultCollector>& collector,
                         const std::shared_ptr<CacheableVector>& results);

 private:
  Execution(const Execution& rhs)
      : m_routingObj(rhs.m_routingObj),
        m_args(rhs.m_args),
        m_rc(rhs.m_rc),
        m_region(rhs.m_region),
        m_allServer(rhs.m_allServer),
        m_pool(rhs.m_pool),
        m_proxyCache(rhs.m_proxyCache) {}
  Execution(const std::shared_ptr<CacheableVector>& routingObj,
                const std::shared_ptr<Cacheable>& args,
                const std::shared_ptr<ResultCollector>& rc,
                const std::shared_ptr<Region>& region, const bool allServer,
                const std::shared_ptr<Pool>& pool,
                std::shared_ptr<ProxyCache> proxyCache = nullptr)
      : m_routingObj(routingObj),
        m_args(args),
        m_rc(rc),
        m_region(region),
        m_allServer(allServer),
        m_pool(pool),
        m_proxyCache(proxyCache) {}
  // ACE_Recursive_Thread_Mutex m_lock;
  std::shared_ptr<CacheableVector> m_routingObj;
  std::shared_ptr<Cacheable> m_args;
  std::shared_ptr<ResultCollector> m_rc;
  std::shared_ptr<Region> m_region;
  bool m_allServer;
  std::shared_ptr<Pool> m_pool;
  std::shared_ptr<ProxyCache> m_proxyCache;
  static ACE_Recursive_Thread_Mutex m_func_attrs_lock;
  static FunctionToFunctionAttributes m_func_attrs;
  //  std::vector<int8_t> m_attributes;

  std::shared_ptr<CacheableVector> executeOnPool(
      const std::string& func, uint8_t getResult, int32_t retryAttempts,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

  void executeOnAllServers(
      const std::string& func, uint8_t getResult,
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

  std::vector<int8_t>* getFunctionAttributes(const std::string& func);
  GfErrType getFuncAttributes(const std::string& func,
                              std::vector<int8_t>** attr);

  _GEODE_FRIEND_STD_SHARED_PTR(Execution)
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXECUTIONIMPL_H_
