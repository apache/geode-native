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

#include "ExecutionImpl.hpp"

#include <boost/thread/lock_types.hpp>

#include <geode/DefaultResultCollector.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheImpl.hpp"
#include "ChunkedFunctionExecutionResponse.hpp"
#include "NoResult.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"
#include "UserAttributes.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

Execution ExecutionImpl::withFilter(
    std::shared_ptr<CacheableVector> routingObj) {
  if (routingObj == nullptr) {
    throw IllegalArgumentException("Execution::withFilter: filter is null");
  }
  if (region_ == nullptr) {
    throw UnsupportedOperationException(
        "Execution::withFilter: FunctionService::onRegion needs to be called "
        "first before calling this function");
  }
  //      m_routingObj = routingObj;
  return Execution(std::unique_ptr<ExecutionImpl>(new ExecutionImpl(
      routingObj, args_, rc_, region_, allServer_, pool_, authenticatedView_)));
}

Execution ExecutionImpl::withArgs(std::shared_ptr<Cacheable> args) {
  if (args == nullptr) {
    throw IllegalArgumentException("Execution::withArgs: args is null");
  }
  //  m_args = args;
  return Execution(std::unique_ptr<ExecutionImpl>(new ExecutionImpl(
      routingObj_, args, rc_, region_, allServer_, pool_, authenticatedView_)));
}

Execution ExecutionImpl::withCollector(std::shared_ptr<ResultCollector> rs) {
  if (rs == nullptr) {
    throw IllegalArgumentException(
        "Execution::withCollector: collector is null");
  }
  //	m_rc = rs;
  return Execution(std::unique_ptr<ExecutionImpl>(new ExecutionImpl(
      routingObj_, args_, rs, region_, allServer_, pool_, authenticatedView_)));
}

std::shared_ptr<ResultCollector> ExecutionImpl::execute(
    const std::shared_ptr<CacheableVector>& routingObj,
    const std::shared_ptr<Cacheable>& args,
    const std::shared_ptr<ResultCollector>& rs, const std::string& func,
    std::chrono::milliseconds timeout) {
  routingObj_ = routingObj;
  args_ = args;
  rc_ = rs;
  return execute(func, timeout);
}

FunctionAttributes ExecutionImpl::getFunctionAttributes(
    const std::string& func) {
  boost::shared_lock<boost::shared_mutex> lock(funcAttrsMutex_);
  auto&& itr = funcAttrs_.find(func);

  if (itr != funcAttrs_.end()) {
    return itr->second;
  }

  return {};
}

FunctionAttributes ExecutionImpl::updateFunctionAttributes(
    const std::string& funcName) {
  GfErrType err = GF_NOERR;
  boost::upgrade_lock<boost::shared_mutex> lock{funcAttrsMutex_};

  auto&& iter = funcAttrs_.find(funcName);
  if (iter != funcAttrs_.end()) {
    return iter->second;
  }

  FunctionAttributes attrs;
  boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock{lock};
  if (region_) {
    err = dynamic_cast<ThinClientRegion*>(region_.get())
              ->getFuncAttributes(funcName, attrs);
  } else if (pool_) {
    err = getFuncAttributes(funcName, attrs);
  }
  if (err != GF_NOERR) {
    throwExceptionIfError("Execute::GET_FUNCTION_ATTRIBUTES", err);
  }
  if (attrs && err == GF_NOERR) {
    funcAttrs_[funcName] = attrs;
  }

  return attrs;
}

void ExecutionImpl::clearFunctionAttributes(const std::string& funcName) {
  boost::unique_lock<decltype(funcAttrsMutex_)> guard{funcAttrsMutex_};
  funcAttrs_.erase(funcName);
}

std::shared_ptr<ResultCollector> ExecutionImpl::execute(
    const std::string& func, std::chrono::milliseconds timeout) {
  LOGDEBUG("ExecutionImpl::execute: ");
  GuardUserAttributes gua;
  if (authenticatedView_ != nullptr) {
    LOGDEBUG("ExecutionImpl::execute function on authenticated cache");
    gua.setAuthenticatedView(authenticatedView_);
  }

  auto&& attrs = getFunctionAttributes(func);
  if (!attrs) {
    attrs = updateFunctionAttributes(func);
  }

  LOGDEBUG(
      "ExecutionImpl::execute got functionAttributes from server for function "
      "= %s serverHasResult = %d serverIsHA = %d serverOptimizeForWrite = %d ",
      func.c_str(), attrs.hasResult(), attrs.isHA(),
      attrs.isOptimizedForWrite());

  if (!attrs.hasResult()) {
    rc_ = std::make_shared<NoResult>();
  } else if (!rc_) {
    rc_ = std::make_shared<DefaultResultCollector>();
  }

  LOGDEBUG("ExecutionImpl::execute: function state = %d", attrs.getFlags());
  TXState* txState = TSSTXStateWrapper::get().getTXState();

  if (txState != nullptr && allServer_ == true) {
    throw UnsupportedOperationException(
        "Execution::execute: Transaction function execution on all servers is "
        "not supported");
  }

  try {
    if (region_ != nullptr) {
      int32_t retryAttempts = 3;
      if (pool_ != nullptr) {
        retryAttempts = pool_->getRetryAttempts();
      }

      if (pool_ != nullptr && pool_->getPRSingleHopEnabled()) {
        auto tcrdm = std::dynamic_pointer_cast<ThinClientPoolDM>(pool_);
        if (!tcrdm) {
          throw IllegalArgumentException(
              "Execute: pool cast to ThinClientPoolDM failed");
        }
        auto cms = tcrdm->getClientMetaDataService();
        auto failedNodes = CacheableHashSet::create();
        if ((!routingObj_ || routingObj_->empty()) &&
            txState == nullptr) {  // For transactions we should not create
                                   // multiple threads
          LOGDEBUG("ExecutionImpl::execute: m_routingObj is empty");
          auto serverToBucketsMap = cms->groupByServerToAllBuckets(
              region_, attrs.isOptimizedForWrite());
          if (!serverToBucketsMap || serverToBucketsMap->empty()) {
            LOGDEBUG(
                "ExecutionImpl::execute: m_routingObj is empty and locationMap "
                "is also empty so use old FE onRegion");
            std::dynamic_pointer_cast<ThinClientRegion>(region_)
                ->executeFunction(func, args_, routingObj_, attrs, rc_,
                                  attrs.isHA() ? retryAttempts : 0, timeout);
            dynamic_cast<ThinClientRegion*>(region_.get())
                ->setMetaDataRefreshed(false);
            cms->enqueueForMetadataRefresh(region_->getFullPath(), 0);
          } else {
            // convert server to bucket map to server to key map where bucket id
            // is key.
            auto serverToKeysMap =
                std::make_shared<ClientMetadataService::ServerToKeysMap>(
                    serverToBucketsMap->size());
            for (const auto& entry : *serverToBucketsMap) {
              auto keys = std::make_shared<CacheableHashSet>(
                  static_cast<int32_t>(entry.second->size()));
              for (const auto& bucket : *(entry.second)) {
                keys->insert(CacheableInt32::create(bucket));
              }
              serverToKeysMap->emplace(entry.first, keys);
            }
            LOGDEBUG(
                "ExecutionImpl::execute: withoutFilter and locationMap is not "
                "empty");
            bool reExecute =
                std::dynamic_pointer_cast<ThinClientRegion>(region_)
                    ->executeFunctionSH(func, args_, attrs, rc_,
                                        serverToKeysMap, failedNodes, timeout,
                                        /*allBuckets*/ true);
            if (reExecute) {  // Fallback to old FE onREgion
              if (attrs.isHA()) {
                rc_->clearResults();
                auto rs = std::dynamic_pointer_cast<ThinClientRegion>(region_)
                              ->reExecuteFunction(
                                  func, args_, routingObj_, attrs, rc_,
                                  attrs.isHA() ? retryAttempts : 0, failedNodes,
                                  timeout);
              }
            }
          }
        } else if (routingObj_ != nullptr && routingObj_->size() == 1) {
          LOGDEBUG("executeFunction onRegion WithFilter size equal to 1 ");
          dynamic_cast<ThinClientRegion*>(region_.get())
              ->executeFunction(func, args_, routingObj_, attrs, rc_,
                                attrs.isHA() ? retryAttempts : 0, timeout);
        } else {
          if (txState == nullptr) {
            auto serverToKeysMap = cms->getServerToFilterMapFESHOP(
                routingObj_, region_, attrs.isOptimizedForWrite());
            if (!serverToKeysMap || serverToKeysMap->empty()) {
              LOGDEBUG(
                  "ExecutionImpl::execute: withFilter but locationMap is empty "
                  "so use old FE onRegion");
              dynamic_cast<ThinClientRegion*>(region_.get())
                  ->executeFunction(func, args_, routingObj_, attrs, rc_,
                                    attrs.isHA() ? retryAttempts : 0, timeout);
              cms->enqueueForMetadataRefresh(region_->getFullPath(), 0);
            } else {
              LOGDEBUG(
                  "ExecutionImpl::execute: withFilter and locationMap is not "
                  "empty");
              bool reExecute =
                  dynamic_cast<ThinClientRegion*>(region_.get())
                      ->executeFunctionSH(func, args_, attrs, rc_,
                                          serverToKeysMap, failedNodes, timeout,
                                          /*allBuckets*/ false);
              if (reExecute) {       // Fallback to old FE onREgion
                if (attrs.isHA()) {  // isHA = true
                  rc_->clearResults();
                  auto rs = dynamic_cast<ThinClientRegion*>(region_.get())
                                ->reExecuteFunction(
                                    func, args_, routingObj_, attrs, rc_,
                                    attrs.isHA() ? retryAttempts : 0,
                                    failedNodes, timeout);
                }
              }
            }
          } else {  // For transactions use old way
            dynamic_cast<ThinClientRegion*>(region_.get())
                ->executeFunction(func, args_, routingObj_, attrs, rc_,
                                  attrs.isHA() ? retryAttempts : 0, timeout);
          }
        }
      } else {  // w/o single hop, Fallback to old FE onREgion
        dynamic_cast<ThinClientRegion*>(region_.get())
            ->executeFunction(func, args_, routingObj_, attrs, rc_,
                              attrs.isHA() ? retryAttempts : 0, timeout);
      }

      if (attrs.hasResult()) {
        rc_->endResults();
      }

      return rc_;
    } else if (pool_ != nullptr) {
      if (txState != nullptr) {
        throw UnsupportedOperationException(
            "Execution::execute: Transaction function execution on pool is not "
            "supported");
      }
      if (!allServer_) {
        executeOnPool(func, attrs, attrs.isHA() ? pool_->getRetryAttempts() : 0,
                      timeout);
        if (attrs.hasResult()) {
          rc_->endResults();
        }
        return rc_;
      }
      executeOnAllServers(func, attrs, timeout);
    } else {
      throw IllegalStateException("Execution::execute: should not be here");
    }
  } catch (FunctionAttributesMismatchException&) {
    LOGERROR("Execution::execute: function '%s' attributes mismatch detected",
             func.c_str());
    clearFunctionAttributes(func);
    throw;
  }

  return rc_;
}

GfErrType ExecutionImpl::getFuncAttributes(const std::string& func,
                                           FunctionAttributes& attr) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(pool_.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }

  GfErrType err = GF_NOERR;

  // do TCR GET_FUNCTION_ATTRIBUTES
  LOGDEBUG("Tcrmessage request GET_FUNCTION_ATTRIBUTES ");
  TcrMessageGetFunctionAttributes request(
      new DataOutput(
          tcrdm->getConnectionManager().getCacheImpl()->createDataOutput()),
      func, tcrdm);
  TcrMessageReply reply(true, tcrdm);
  err = tcrdm->sendSyncRequest(request, reply);
  if (err != GF_NOERR) {
    return err;
  }
  switch (reply.getMessageType()) {
    case TcrMessage::RESPONSE: {
      attr = reply.getFunctionAttributes();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = dynamic_cast<ThinClientRegion*>(region_.get())
                ->handleServerException("Region::GET_FUNCTION_ATTRIBUTES",
                                        reply.getException());
      break;
    }
    case TcrMessage::REQUEST_DATA_ERROR: {
      LOGERROR("Error message from server: " + reply.getValue()->toString());
      throw FunctionException(reply.getValue()->toString());
    }
    default: {
      LOGERROR("Unknown message type %d while getting function attributes.",
               reply.getMessageType());
      err = GF_MSG;
      break;
    }
  }
  return err;
}

void ExecutionImpl::addResults(
    std::shared_ptr<ResultCollector>& collector,
    const std::shared_ptr<CacheableVector>& results) {
  if (results == nullptr || collector == nullptr) {
    return;
  }

  for (const auto& result : *results) {
    collector->addResult(result);
  }
}

void ExecutionImpl::executeOnAllServers(const std::string& func,
                                        FunctionAttributes funcAttrs,
                                        std::chrono::milliseconds timeout) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(pool_.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }

  std::string exceptionMsg;
  GfErrType err = tcrdm->sendRequestToAllServers(
      func.c_str(), funcAttrs, timeout, args_, rc_, exceptionMsg);

  if (err != GF_NOERR) {
    LOGDEBUG("Execute failed: %d", err);
    if (err == GF_CACHESERVER_EXCEPTION) {
      std::string message;
      if (!exceptionMsg.empty()) {
        message = std::string("Execute: exception at the server side: ") +
                  exceptionMsg.c_str();
      } else {
        message = "Execute: failed to execute function with server.";
      }
      throw FunctionException(message);
    } else {
      throwExceptionIfError("Execute", err);
    }
  }
}
std::shared_ptr<CacheableVector> ExecutionImpl::executeOnPool(
    const std::string& func, FunctionAttributes funcAttrs,
    int32_t retryAttempts, std::chrono::milliseconds timeout) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(pool_.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }
  int32_t attempt = 0;

  // if pools retry attempts are not set then retry once on all available
  // endpoints
  if (retryAttempts == -1) {
    retryAttempts = static_cast<int32_t>(tcrdm->getNumberOfEndPoints());
  }

  while (attempt <= retryAttempts) {
    std::string funcName(func);
    TcrMessageExecuteFunction msg(
        new DataOutput(
            tcrdm->getConnectionManager().getCacheImpl()->createDataOutput()),
        funcName, args_, funcAttrs, tcrdm, timeout);
    TcrMessageReply reply(true, tcrdm);
    auto resultCollector = std::unique_ptr<ChunkedFunctionExecutionResponse>(
        new ChunkedFunctionExecutionResponse(reply, funcAttrs.hasResult(),
                                             rc_));
    reply.setChunkedResultHandler(resultCollector.get());
    reply.setTimeout(timeout);

    GfErrType err = GF_NOERR;
    // Function failover logic is not handled in the network layer. That's why
    // attemptFailover should be always be false when calling sendSyncRequest
    err = tcrdm->sendSyncRequest(msg, reply, false);
    LOGFINE("executeOnPool %d attempt = %d retryAttempts = %d", err, attempt,
            retryAttempts);

    if (err == GF_NOERR) {
      auto replyType = reply.getMessageType();
      if (replyType == TcrMessage::EXCEPTION) {
        err = ThinClientRegion::handleServerException(
            "ExecutionImpl::executeOnPool", reply.getException());
      } else if (replyType == TcrMessage::EXECUTE_FUNCTION_ERROR) {
        err = ThinClientRegion::handleServerFunctionError(
            "ExecutionImpl::executeOnPool", reply.getFunctionError());
      }
    }

    if (ThinClientBaseDM::isFatalClientError(err)) {
      throwExceptionIfError("ExecuteOnPool:", err);
    } else if (err != GF_NOERR) {
      if (funcAttrs.isHA()) {
        resultCollector->reset();
        rc_->clearResults();
        attempt++;
        if (attempt > 0) {
          funcAttrs.markRetry();
        }
        continue;
      } else {
        throwExceptionIfError("ExecuteOnPool:", err);
      }
    }
    return {};
  }
  return {};
}

}  // namespace client
}  // namespace geode
}  // namespace apache
