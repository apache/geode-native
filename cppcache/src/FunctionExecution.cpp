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

#include "FunctionExecution.hpp"

#include <geode/ResultCollector.hpp>

#include "CacheImpl.hpp"
#include "ChunkedFunctionExecutionResponse.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrMessage.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"

namespace apache {
namespace geode {
namespace client {

void FunctionExecution::setParameters(
    const std::string& funcName, FunctionAttributes funcAttrs,
    std::chrono::milliseconds timeout, std::shared_ptr<Cacheable> args,
    TcrEndpoint* ep, ThinClientPoolDM* poolDM,
    std::shared_ptr<std::recursive_mutex> resultCollectorMutex,
    std::shared_ptr<ResultCollector> resultCollector,
    std::shared_ptr<UserAttributes> userAttr) {
  errorMsg_.clear();
  resultCollectorMutex_ = std::move(resultCollectorMutex);
  resultCollector_ = resultCollector;
  error_ = GF_NOTCON;
  funcName_ = funcName;
  funcAttrs_ = funcAttrs;
  timeout_ = timeout;
  args_ = args;
  endpoint_ = ep;
  pool_ = poolDM;
  userAttrs_ = userAttr;
}

GfErrType FunctionExecution::execute() {
  GuardUserAttributes gua;

  if (userAttrs_) {
    gua.setAuthenticatedView(userAttrs_->getAuthenticatedView());
  }

  TcrMessageExecuteFunction request(
      new DataOutput(
          pool_->getConnectionManager().getCacheImpl()->createDataOutput()),
      funcName_, args_, funcAttrs_, pool_, timeout_);
  TcrMessageReply reply(true, pool_);

  auto resultProcessor = std::unique_ptr<ChunkedFunctionExecutionResponse>(
      new ChunkedFunctionExecutionResponse(reply, funcAttrs_.hasResult(),
                                           resultCollector_,
                                           resultCollectorMutex_));

  reply.setChunkedResultHandler(resultProcessor.get());
  reply.setTimeout(timeout_);
  reply.setDM(pool_);

  LOGDEBUG(
      "ThinClientPoolDM::sendRequestToAllServer sendRequest on endpoint[%s]!",
      endpoint_->name().c_str());

  error_ = pool_->sendRequestToEP(request, reply, endpoint_);
  error_ = pool_->handleEPError(endpoint_, reply, error_);
  if (error_ != GF_NOERR) {
    if (error_ == GF_NOTCON) {
      return GF_NOERR;  // if server is unavailable its not an error for
      // functionexec OnServers() case
    }
    LOGDEBUG("FunctionExecution::execute failed on endpoint[%s]!. Error = %d ",
             endpoint_->name().c_str(), error_);
    if (reply.getMessageType() == TcrMessage::EXCEPTION) {
      errorMsg_ = reply.getException();
    }

    return error_;
  } else if (reply.getMessageType() == TcrMessage::EXCEPTION) {
    errorMsg_ = reply.getException();
    error_ = ThinClientRegion::handleServerException(
        "FunctionExecution::execute", errorMsg_);
  } else if (reply.getMessageType() == TcrMessage::EXECUTE_FUNCTION_ERROR) {
    errorMsg_ = reply.getFunctionError();
    error_ = ThinClientRegion::handleServerFunctionError(
        "FunctionExecution::execute", errorMsg_);
  }

  return error_;
}

OnRegionFunctionExecution::OnRegionFunctionExecution(
    std::string funcName, const Region* region, std::shared_ptr<Cacheable> args,
    std::shared_ptr<CacheableHashSet> routingObj, FunctionAttributes funcAttrs,
    std::chrono::milliseconds timeout, ThinClientPoolDM* poolDM,
    const std::shared_ptr<std::recursive_mutex>& collectorMutex,
    std::shared_ptr<ResultCollector> collector,
    std::shared_ptr<UserAttributes> userAttrs, bool isBGThread,
    const std::shared_ptr<BucketServerLocation>& serverLocation,
    bool allBuckets)
    : serverLocation_{serverLocation},
      backgroundThread_{isBGThread},
      pool_{poolDM},
      funcName_{funcName},
      funcAttrs_{funcAttrs},
      timeout_{timeout},
      args_{args},
      routingObj_{routingObj},
      resultCollector_{std::move(collector)},
      resultCollectorMutex_{collectorMutex},
      userAttrs_{userAttrs},
      region_{region},
      allBuckets_{allBuckets} {
  request_ = new TcrMessageExecuteRegionFunctionSingleHop(
      new DataOutput(
          pool_->getConnectionManager().getCacheImpl()->createDataOutput()),
      funcName_, region_, args_, routingObj_, funcAttrs, nullptr, allBuckets_,
      timeout, pool_);
  reply_ = new TcrMessageReply(true, pool_);
  chunkedResponse_ = new ChunkedFunctionExecutionResponse(
      *reply_, funcAttrs.hasResult(), resultCollector_, resultCollectorMutex_);
  reply_->setChunkedResultHandler(chunkedResponse_);
  reply_->setTimeout(timeout);
  reply_->setDM(pool_);
}

OnRegionFunctionExecution::~OnRegionFunctionExecution() noexcept {
  delete request_;
  delete reply_;
}

std::shared_ptr<CacheableHashSet> OnRegionFunctionExecution::getFailedNode()
    const {
  return reply_->getFailedNode();
}

GfErrType OnRegionFunctionExecution::execute() {
  GuardUserAttributes gua;

  if (userAttrs_) {
    gua.setAuthenticatedView(userAttrs_->getAuthenticatedView());
  }

  // Function failover logic is not handled in the network layer. That's why
  // attemptFailover should be always be false when calling sendSyncRequest
  return pool_->sendSyncRequest(*request_, *reply_, false, backgroundThread_,
                                serverLocation_);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
