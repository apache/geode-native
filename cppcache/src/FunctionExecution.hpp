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

#ifndef GEODE_FUNCTIONEXECUTION_H_
#define GEODE_FUNCTIONEXECUTION_H_

#include <memory>
#include <string>

#include <geode/Serializable.hpp>

#include "ErrType.hpp"
#include "FunctionAttributes.hpp"
#include "ThreadPool.hpp"

namespace apache {
namespace geode {
namespace client {

class BucketServerLocation;
class CacheableHashSet;
class CacheableString;
class ChunkedFunctionExecutionResponse;
class Region;
class ResultCollector;
class TcrChunkedResult;
class TcrEndpoint;
class TcrMessage;
class TcrMessageReply;
class ThinClientPoolDM;
class UserAttributes;

class FunctionExecution : public PooledWork<GfErrType> {
public:
 FunctionExecution() : pool_{nullptr}, endpoint_{nullptr}, error_{GF_NOERR} {}

 ~FunctionExecution() noexcept override = default;

 const std::string& getException() { return errorMsg_; }

 void setParameters(const std::string& funcName, FunctionAttributes funcAttrs,
                    std::chrono::milliseconds timeout,
                    std::shared_ptr<Cacheable> args, TcrEndpoint* ep,
                    ThinClientPoolDM* poolDM,
                    std::shared_ptr<std::recursive_mutex> resultCollectorMutex,
                    std::shared_ptr<ResultCollector> resultCollector,
                    std::shared_ptr<UserAttributes> userAttr);

 GfErrType execute(void) override;

protected:
 ThinClientPoolDM* pool_;
 TcrEndpoint* endpoint_;
 std::string funcName_;
 FunctionAttributes funcAttrs_;
 std::chrono::milliseconds timeout_;
 std::shared_ptr<Cacheable> args_;
 GfErrType error_;
 std::shared_ptr<ResultCollector> resultCollector_;
 std::shared_ptr<std::recursive_mutex> resultCollectorMutex_;
 std::string errorMsg_;
 std::shared_ptr<UserAttributes> userAttrs_;
};

class OnRegionFunctionExecution : public PooledWork<GfErrType> {
public:
 OnRegionFunctionExecution(
     std::string funcName, const Region* region,
     std::shared_ptr<Cacheable> args,
     std::shared_ptr<CacheableHashSet> routingObj,
     FunctionAttributes funcAttrs, std::chrono::milliseconds timeout,
     ThinClientPoolDM* poolDM,
     const std::shared_ptr<std::recursive_mutex>& collectorMutex,
     std::shared_ptr<ResultCollector> collector,
     std::shared_ptr<UserAttributes> userAttrs, bool isBGThread,
     const std::shared_ptr<BucketServerLocation>& serverLocation,
     bool allBuckets);

 ~OnRegionFunctionExecution() noexcept override;

 TcrMessage* getReply() { return reinterpret_cast<TcrMessage*>(reply_); }

 std::shared_ptr<CacheableHashSet> getFailedNode() const;

 ChunkedFunctionExecutionResponse* getResultCollector() {
   return reinterpret_cast<ChunkedFunctionExecutionResponse*>(
       resultCollector_.get());
 }

 GfErrType execute() override;

protected:
 std::shared_ptr<BucketServerLocation> serverLocation_;
 TcrMessage* request_;
 TcrMessageReply* reply_;
 bool backgroundThread_;
 ThinClientPoolDM* pool_;
 std::string funcName_;
 FunctionAttributes funcAttrs_;
 std::chrono::milliseconds timeout_;
 std::shared_ptr<Cacheable> args_;
 std::shared_ptr<CacheableHashSet> routingObj_;
 std::shared_ptr<ResultCollector> resultCollector_;
 TcrChunkedResult* chunkedResponse_;
 std::shared_ptr<std::recursive_mutex> resultCollectorMutex_;
 std::shared_ptr<UserAttributes> userAttrs_;
 const Region* region_;
 bool allBuckets_;
};  // class FunctionAttributes

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FUNCTIONEXECUTION_H_
