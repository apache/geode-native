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
  if (m_region == nullptr) {
    throw UnsupportedOperationException(
        "Execution::withFilter: FunctionService::onRegion needs to be called "
        "first before calling this function");
  }
  //      m_routingObj = routingObj;
  return Execution(std::unique_ptr<ExecutionImpl>(
      new ExecutionImpl(routingObj, m_args, m_rc, m_region, m_allServer, m_pool,
                        m_authenticatedView)));
}

Execution ExecutionImpl::withArgs(std::shared_ptr<Cacheable> args) {
  if (args == nullptr) {
    throw IllegalArgumentException("Execution::withArgs: args is null");
  }
  //  m_args = args;
  return Execution(std::unique_ptr<ExecutionImpl>(
      new ExecutionImpl(m_routingObj, args, m_rc, m_region, m_allServer, m_pool,
                        m_authenticatedView)));
}

Execution ExecutionImpl::withCollector(std::shared_ptr<ResultCollector> rs) {
  if (rs == nullptr) {
    throw IllegalArgumentException(
        "Execution::withCollector: collector is null");
  }
  //	m_rc = rs;
  return Execution(std::unique_ptr<ExecutionImpl>(
      new ExecutionImpl(m_routingObj, m_args, rs, m_region, m_allServer, m_pool,
                        m_authenticatedView)));
}

std::shared_ptr<ResultCollector> ExecutionImpl::execute(
    const std::shared_ptr<CacheableVector>& routingObj,
    const std::shared_ptr<Cacheable>& args,
    const std::shared_ptr<ResultCollector>& rs, const std::string& func,
    std::chrono::milliseconds timeout) {
  m_routingObj = routingObj;
  m_args = args;
  m_rc = rs;
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
  if (m_region) {
    err = dynamic_cast<ThinClientRegion*>(m_region.get())
              ->getFuncAttributes(funcName, attrs);
  } else if (m_pool) {
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

std::shared_ptr<ResultCollector> ExecutionImpl::execute(
    const std::string& func, std::chrono::milliseconds timeout) {
  LOGDEBUG("ExecutionImpl::execute: ");
  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    LOGDEBUG("ExecutionImpl::execute function on authenticated cache");
    gua.setAuthenticatedView(m_authenticatedView);
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
    m_rc = std::make_shared<NoResult>();
  } else if (!m_rc) {
    m_rc = std::make_shared<DefaultResultCollector>();
  }

  LOGDEBUG("ExecutionImpl::execute: function state = %d", attrs.getFlags());
  TXState* txState = TSSTXStateWrapper::get().getTXState();

  if (txState != nullptr && m_allServer == true) {
    throw UnsupportedOperationException(
        "Execution::execute: Transaction function execution on all servers is "
        "not supported");
  }

  if (m_region != nullptr) {
    int32_t retryAttempts = 3;
    if (m_pool != nullptr) {
      retryAttempts = m_pool->getRetryAttempts();
    }

    if (m_pool != nullptr && m_pool->getPRSingleHopEnabled()) {
      auto tcrdm = std::dynamic_pointer_cast<ThinClientPoolDM>(m_pool);
      if (!tcrdm) {
        throw IllegalArgumentException(
            "Execute: pool cast to ThinClientPoolDM failed");
      }
      auto cms = tcrdm->getClientMetaDataService();
      auto failedNodes = CacheableHashSet::create();
      if ((!m_routingObj || m_routingObj->empty()) &&
          txState == nullptr) {  // For transactions we should not create
                                 // multiple threads
        LOGDEBUG("ExecutionImpl::execute: m_routingObj is empty");
        auto serverToBucketsMap = cms->groupByServerToAllBuckets(
            m_region, attrs.isOptimizedForWrite());
        if (!serverToBucketsMap || serverToBucketsMap->empty()) {
          LOGDEBUG(
              "ExecutionImpl::execute: m_routingObj is empty and locationMap "
              "is also empty so use old FE onRegion");
          std::dynamic_pointer_cast<ThinClientRegion>(m_region)
              ->executeFunction(func, m_args, m_routingObj, attrs, m_rc,
                                attrs.isHA() ? retryAttempts : 0, timeout);
          dynamic_cast<ThinClientRegion*>(m_region.get())
              ->setMetaDataRefreshed(false);
          cms->enqueueForMetadataRefresh(m_region->getFullPath(), 0);
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
              std::dynamic_pointer_cast<ThinClientRegion>(m_region)
                  ->executeFunctionSH(func, m_args, attrs, m_rc,
                                      serverToKeysMap, failedNodes, timeout,
                                      /*allBuckets*/ true);
          if (reExecute) {  // Fallback to old FE onREgion
            if (attrs.isHA()) {
              m_rc->clearResults();
              auto rs = std::dynamic_pointer_cast<ThinClientRegion>(m_region)
                            ->reExecuteFunction(
                                func, m_args, m_routingObj, attrs, m_rc,
                                attrs.isHA() ? retryAttempts : 0, failedNodes,
                                timeout);
            }
          }
        }
      } else if (m_routingObj != nullptr && m_routingObj->size() == 1) {
        LOGDEBUG("executeFunction onRegion WithFilter size equal to 1 ");
        dynamic_cast<ThinClientRegion*>(m_region.get())
            ->executeFunction(func, m_args, m_routingObj, attrs, m_rc,
                              attrs.isHA() ? retryAttempts : 0, timeout);
      } else {
        if (txState == nullptr) {
          auto serverToKeysMap = cms->getServerToFilterMapFESHOP(
              m_routingObj, m_region, attrs.isOptimizedForWrite());
          if (!serverToKeysMap || serverToKeysMap->empty()) {
            LOGDEBUG(
                "ExecutionImpl::execute: withFilter but locationMap is empty "
                "so use old FE onRegion");
            dynamic_cast<ThinClientRegion*>(m_region.get())
                ->executeFunction(func, m_args, m_routingObj, attrs, m_rc,
                                  attrs.isHA() ? retryAttempts : 0, timeout);
            cms->enqueueForMetadataRefresh(m_region->getFullPath(), 0);
          } else {
            LOGDEBUG(
                "ExecutionImpl::execute: withFilter and locationMap is not "
                "empty");
            bool reExecute =
                dynamic_cast<ThinClientRegion*>(m_region.get())
                    ->executeFunctionSH(func, m_args, attrs, m_rc,
                                        serverToKeysMap, failedNodes, timeout,
                                        /*allBuckets*/ false);
            if (reExecute) {       // Fallback to old FE onREgion
              if (attrs.isHA()) {  // isHA = true
                m_rc->clearResults();
                auto rs = dynamic_cast<ThinClientRegion*>(m_region.get())
                              ->reExecuteFunction(
                                  func, m_args, m_routingObj, attrs, m_rc,
                                  attrs.isHA() ? retryAttempts : 0, failedNodes,
                                  timeout);
              }
            }
          }
        } else {  // For transactions use old way
          dynamic_cast<ThinClientRegion*>(m_region.get())
              ->executeFunction(func, m_args, m_routingObj, attrs, m_rc,
                                attrs.isHA() ? retryAttempts : 0, timeout);
        }
      }
    } else {  // w/o single hop, Fallback to old FE onREgion
      dynamic_cast<ThinClientRegion*>(m_region.get())
          ->executeFunction(func, m_args, m_routingObj, attrs, m_rc,
                            attrs.isHA() ? retryAttempts : 0, timeout);
    }

    if (attrs.hasResult()) {
      m_rc->endResults();
    }

    return m_rc;
  } else if (m_pool != nullptr) {
    if (txState != nullptr) {
      throw UnsupportedOperationException(
          "Execution::execute: Transaction function execution on pool is not "
          "supported");
    }
    if (!m_allServer) {
      executeOnPool(func, attrs, attrs.isHA() ? m_pool->getRetryAttempts() : 0,
                    timeout);
      if (attrs.hasResult()) {
        m_rc->endResults();
      }
      return m_rc;
    }
    executeOnAllServers(func, attrs, timeout);
  } else {
    throw IllegalStateException("Execution::execute: should not be here");
  }
  return m_rc;
}

GfErrType ExecutionImpl::getFuncAttributes(const std::string& func,
                                           FunctionAttributes& attr) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(m_pool.get());
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
      err = dynamic_cast<ThinClientRegion*>(m_region.get())
                ->handleServerException("Region::GET_FUNCTION_ATTRIBUTES",
                                        reply.getException());
      break;
    }
    case TcrMessage::REQUEST_DATA_ERROR: {
      LOGERROR("Error message from server: " + reply.getValue()->toString());
      throw FunctionExecutionException(reply.getValue()->toString());
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
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(m_pool.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }

  std::string exceptionMsg;
  GfErrType err = tcrdm->sendRequestToAllServers(
      func.c_str(), funcAttrs, timeout, m_args, m_rc, exceptionMsg);

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
      throw FunctionExecutionException(message);
    } else {
      throwExceptionIfError("Execute", err);
    }
  }
}
std::shared_ptr<CacheableVector> ExecutionImpl::executeOnPool(
    const std::string& func, FunctionAttributes funcAttrs,
    int32_t retryAttempts, std::chrono::milliseconds timeout) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(m_pool.get());
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
        funcName, m_args, funcAttrs, tcrdm, timeout);
    TcrMessageReply reply(true, tcrdm);
    auto resultCollector = std::unique_ptr<ChunkedFunctionExecutionResponse>(
        new ChunkedFunctionExecutionResponse(reply, funcAttrs.hasResult(),
                                             m_rc));
    reply.setChunkedResultHandler(resultCollector.get());
    reply.setTimeout(timeout);

    GfErrType err = GF_NOERR;
    // Function failover logic is not handled in the network layer. That's why
    // attemptFailover should be always be false when calling sendSyncRequest
    err = tcrdm->sendSyncRequest(msg, reply, false);
    LOGFINE("executeOnPool %d attempt = %d retryAttempts = %d", err, attempt,
            retryAttempts);
    if (err == GF_NOERR &&
        (reply.getMessageType() == TcrMessage::EXCEPTION ||
         reply.getMessageType() == TcrMessage::EXECUTE_FUNCTION_ERROR)) {
      err = ThinClientRegion::handleServerException("Execute",
                                                    reply.getException());
    }
    if (ThinClientBaseDM::isFatalClientError(err)) {
      throwExceptionIfError("ExecuteOnPool:", err);
    } else if (err != GF_NOERR) {
      if (funcAttrs.isHA()) {
        resultCollector->reset();
        m_rc->clearResults();
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
