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
#include <mutex>

#include <boost/thread/shared_mutex.hpp>

#include <geode/AuthenticatedView.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/Execution.hpp>
#include <geode/Region.hpp>
#include <geode/ResultCollector.hpp>

#include "ErrType.hpp"
#include "FunctionAttributes.hpp"

namespace apache {
namespace geode {
namespace client {

class ExecutionImpl {
public:
 explicit ExecutionImpl(std::shared_ptr<Region> rptr = nullptr,
                        AuthenticatedView* authenticatedView = nullptr,
                        std::shared_ptr<Pool> pp = nullptr)
     : routingObj_(nullptr),
       args_(nullptr),
       rc_(nullptr),
       region_(rptr),
       allServer_(false),
       pool_(pp),
       authenticatedView_(authenticatedView) {}
 explicit ExecutionImpl(std::shared_ptr<Pool> pool, bool allServer = false,
                        AuthenticatedView* authenticatedView = nullptr)
     : routingObj_(nullptr),
       args_(nullptr),
       rc_(nullptr),
       region_(nullptr),
       allServer_(allServer),
       pool_(pool),
       authenticatedView_(authenticatedView) {}
 virtual ~ExecutionImpl() noexcept = default;
 virtual Execution withFilter(std::shared_ptr<CacheableVector> routingObj);
 virtual Execution withArgs(std::shared_ptr<Cacheable> args);
 virtual Execution withCollector(std::shared_ptr<ResultCollector> rs);
 // java function has hasResult property. we put the hasResult argument
 // here as a kluge.
 virtual std::shared_ptr<ResultCollector> execute(
     const std::shared_ptr<CacheableVector>& routingObj,
     const std::shared_ptr<Cacheable>& args,
     const std::shared_ptr<ResultCollector>& rs, const std::string& func,
     std::chrono::milliseconds timeout);

 virtual std::shared_ptr<ResultCollector> execute(
     const std::string& func,
     std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

 static void addResults(std::shared_ptr<ResultCollector>& collector,
                        const std::shared_ptr<CacheableVector>& results);

protected:
 std::shared_ptr<CacheableVector> executeOnPool(
     const std::string& func, FunctionAttributes funcAttrs, int32_t retryAttempts,
     std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

 void executeOnAllServers(
     const std::string& func, FunctionAttributes funcAttrs,
     std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT);

 FunctionAttributes getFunctionAttributes(
     const std::string& func);
 void clearFunctionAttributes(const std::string &funcName);
 FunctionAttributes updateFunctionAttributes(const std::string &funcName);
 GfErrType getFuncAttributes(const std::string& func, FunctionAttributes& attr);

private:
 ExecutionImpl(const ExecutionImpl& rhs)
     : routingObj_(rhs.routingObj_),
       args_(rhs.args_),
       rc_(rhs.rc_),
       region_(rhs.region_),
       allServer_(rhs.allServer_),
       pool_(rhs.pool_),
       authenticatedView_(rhs.authenticatedView_) {}
 ExecutionImpl(const std::shared_ptr<CacheableVector>& routingObj,
               const std::shared_ptr<Cacheable>& args,
               const std::shared_ptr<ResultCollector>& rc,
               const std::shared_ptr<Region>& region, const bool allServer,
               const std::shared_ptr<Pool>& pool,
               AuthenticatedView* authenticatedView = nullptr)
     : routingObj_(routingObj),
       args_(args),
       rc_(rc),
       region_(region),
       allServer_(allServer),
       pool_(pool),
       authenticatedView_(authenticatedView) {}
protected:

 std::shared_ptr<CacheableVector> routingObj_;
 std::shared_ptr<Cacheable> args_;
 std::shared_ptr<ResultCollector> rc_;
 std::shared_ptr<Region> region_;
 bool allServer_;
 std::shared_ptr<Pool> pool_;
 AuthenticatedView* authenticatedView_;

 std::map<std::string, FunctionAttributes> funcAttrs_;
 boost::shared_mutex funcAttrsMutex_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXECUTIONIMPL_H_
