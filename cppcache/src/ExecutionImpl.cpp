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

#include <sstream>

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

FunctionToFunctionAttributes ExecutionImpl::m_func_attrs;
std::recursive_mutex ExecutionImpl::m_func_attrs_lock;
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

std::shared_ptr<std::vector<int8_t>> ExecutionImpl::getFunctionAttributes(
    const std::string& func) {
  auto&& itr = m_func_attrs.find(func);
  if (itr != m_func_attrs.end()) {
    return itr->second;
  }
  return nullptr;
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

std::shared_ptr<ResultCollector> ExecutionImpl::execute(
    const std::string& func, std::chrono::milliseconds timeout) {
  LOG_DEBUG("ExecutionImpl::execute: ");
  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    LOG_DEBUG("ExecutionImpl::execute function on authenticated cache");
    gua.setAuthenticatedView(m_authenticatedView);
  }
  bool serverHasResult = false;
  bool serverIsHA = false;
  bool serverOptimizeForWrite = false;

  auto&& attr = getFunctionAttributes(func);
  {
    if (attr == nullptr) {
      std::lock_guard<decltype(m_func_attrs_lock)> _guard(m_func_attrs_lock);
      GfErrType err = GF_NOERR;
      attr = getFunctionAttributes(func);
      if (attr == nullptr) {
        if (m_region != nullptr) {
          err = dynamic_cast<ThinClientRegion*>(m_region.get())
                    ->getFuncAttributes(func, &attr);
        } else if (m_pool != nullptr) {
          err = getFuncAttributes(func, &attr);
        }
        if (err != GF_NOERR) {
          throwExceptionIfError("Execute::GET_FUNCTION_ATTRIBUTES", err);
        }
        if (!attr->empty() && err == GF_NOERR) {
          m_func_attrs[func] = attr;
        }
      }
    }
  }
  serverHasResult = ((attr->at(0) == 1) ? true : false);
  serverIsHA = ((attr->at(1) == 1) ? true : false);
  serverOptimizeForWrite = ((attr->at(2) == 1) ? true : false);

  LOG_DEBUG(
      "ExecutionImpl::execute got functionAttributes from server for function "
      "= %s serverHasResult = %d serverIsHA = %d serverOptimizeForWrite = %d ",
      func.c_str(), serverHasResult, serverIsHA, serverOptimizeForWrite);

  if (serverHasResult == false) {
    m_rc = std::make_shared<NoResult>();
  } else if (m_rc == nullptr) {
    m_rc = std::make_shared<DefaultResultCollector>();
  }

  uint8_t isHAHasResultOptimizeForWrite = 0;
  if (serverIsHA) {
    isHAHasResultOptimizeForWrite = isHAHasResultOptimizeForWrite | 1;
  }

  if (serverHasResult) {
    isHAHasResultOptimizeForWrite = isHAHasResultOptimizeForWrite | 2;
  }

  if (serverOptimizeForWrite) {
    isHAHasResultOptimizeForWrite = isHAHasResultOptimizeForWrite | 4;
  }

  LOG_DEBUG("ExecutionImpl::execute: isHAHasResultOptimizeForWrite = %d",
            isHAHasResultOptimizeForWrite);
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
        LOG_DEBUG("ExecutionImpl::execute: m_routingObj is empty");
        auto serverToBucketsMap = cms->groupByServerToAllBuckets(
            m_region,
            /*serverOptimizeForWrite*/ (isHAHasResultOptimizeForWrite & 4) ==
                4);
        if (!serverToBucketsMap || serverToBucketsMap->empty()) {
          LOG_DEBUG(
              "ExecutionImpl::execute: m_routingObj is empty and locationMap "
              "is also empty so use old FE onRegion");
          std::dynamic_pointer_cast<ThinClientRegion>(m_region)
              ->executeFunction(
                  func, m_args, m_routingObj, isHAHasResultOptimizeForWrite,
                  m_rc, (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0,
                  timeout);
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
          LOG_DEBUG(
              "ExecutionImpl::execute: withoutFilter and locationMap is not "
              "empty");
          bool reExecute = std::dynamic_pointer_cast<ThinClientRegion>(m_region)
                               ->executeFunctionSH(
                                   func, m_args, isHAHasResultOptimizeForWrite,
                                   m_rc, serverToKeysMap, failedNodes, timeout,
                                   /*allBuckets*/ true);
          if (reExecute) {  // Fallback to old FE onREgion
            if (isHAHasResultOptimizeForWrite & 1) {  // isHA = true
              m_rc->clearResults();
              auto rs =
                  std::dynamic_pointer_cast<ThinClientRegion>(m_region)
                      ->reExecuteFunction(func, m_args, m_routingObj,
                                          isHAHasResultOptimizeForWrite, m_rc,
                                          (isHAHasResultOptimizeForWrite & 1)
                                              ? retryAttempts
                                              : 0,
                                          failedNodes, timeout);
            } else {  // isHA = false
              m_rc->clearResults();
              dynamic_cast<ThinClientRegion*>(m_region.get())
                  ->executeFunction(
                      func, m_args, m_routingObj, isHAHasResultOptimizeForWrite,
                      m_rc,
                      (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0,
                      timeout);
            }
          }
        }
      } else if (m_routingObj != nullptr && m_routingObj->size() == 1) {
        LOG_DEBUG("executeFunction onRegion WithFilter size equal to 1 ");
        dynamic_cast<ThinClientRegion*>(m_region.get())
            ->executeFunction(
                func, m_args, m_routingObj, isHAHasResultOptimizeForWrite, m_rc,
                (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0,
                timeout);
      } else {
        if (txState == nullptr) {
          auto serverToKeysMap = cms->getServerToFilterMapFESHOP(
              m_routingObj, m_region, /*serverOptimizeForWrite*/
              (isHAHasResultOptimizeForWrite & 4) == 4);
          if (!serverToKeysMap || serverToKeysMap->empty()) {
            LOG_DEBUG(
                "ExecutionImpl::execute: withFilter but locationMap is empty "
                "so use old FE onRegion");
            dynamic_cast<ThinClientRegion*>(m_region.get())
                ->executeFunction(
                    func, m_args, m_routingObj, isHAHasResultOptimizeForWrite,
                    m_rc,
                    (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0,
                    timeout);
            cms->enqueueForMetadataRefresh(m_region->getFullPath(), 0);
          } else {
            LOG_DEBUG(
                "ExecutionImpl::execute: withFilter and locationMap is not "
                "empty");
            bool reExecute =
                dynamic_cast<ThinClientRegion*>(m_region.get())
                    ->executeFunctionSH(func, m_args,
                                        isHAHasResultOptimizeForWrite, m_rc,
                                        serverToKeysMap, failedNodes, timeout,
                                        /*allBuckets*/ false);
            if (reExecute) {  // Fallback to old FE onREgion
              if (isHAHasResultOptimizeForWrite & 1) {  // isHA = true
                m_rc->clearResults();
                auto rs =
                    dynamic_cast<ThinClientRegion*>(m_region.get())
                        ->reExecuteFunction(func, m_args, m_routingObj,
                                            isHAHasResultOptimizeForWrite, m_rc,
                                            (isHAHasResultOptimizeForWrite & 1)
                                                ? retryAttempts
                                                : 0,
                                            failedNodes, timeout);
              } else {  // isHA = false
                m_rc->clearResults();
                dynamic_cast<ThinClientRegion*>(m_region.get())
                    ->executeFunction(
                        func, m_args, m_routingObj,
                        isHAHasResultOptimizeForWrite, m_rc,
                        (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0,
                        timeout);
              }
            }
          }
        } else {  // For transactions use old way
          dynamic_cast<ThinClientRegion*>(m_region.get())
              ->executeFunction(
                  func, m_args, m_routingObj, isHAHasResultOptimizeForWrite,
                  m_rc, (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0,
                  timeout);
        }
      }
    } else {  // w/o single hop, Fallback to old FE onREgion
      dynamic_cast<ThinClientRegion*>(m_region.get())
          ->executeFunction(
              func, m_args, m_routingObj, isHAHasResultOptimizeForWrite, m_rc,
              (isHAHasResultOptimizeForWrite & 1) ? retryAttempts : 0, timeout);
    }
    /*    } catch (TransactionDataNodeHasDepartedException e) {
                    if(txState == nullptr)
                    {
                            GfErrTypeThrowException("Transaction is nullptr",
       GF_CACHE_ILLEGAL_STATE_EXCEPTION);
                    }

                    if(!txState->isReplay())
                            txState->replay(false);
            } catch(TransactionDataRebalancedException e) {
                    if(txState == nullptr)
                    {
                            GfErrTypeThrowException("Transaction is nullptr",
       GF_CACHE_ILLEGAL_STATE_EXCEPTION);
                    }

                    if(!txState->isReplay())
                            txState->replay(true);
            }
    */
    if (serverHasResult == true) {
      // ExecutionImpl::addResults(m_rc, rs);
      m_rc->endResults();
    }

    return m_rc;
  } else if (m_pool != nullptr) {
    if (txState != nullptr) {
      throw UnsupportedOperationException(
          "Execution::execute: Transaction function execution on pool is not "
          "supported");
    }
    if (m_allServer == false) {
      executeOnPool(
          func, isHAHasResultOptimizeForWrite,
          (isHAHasResultOptimizeForWrite & 1) ? m_pool->getRetryAttempts() : 0,
          timeout);
      if (serverHasResult == true) {
        // ExecutionImpl::addResults(m_rc, rs);
        m_rc->endResults();
      }
      return m_rc;
    }
    executeOnAllServers(func, isHAHasResultOptimizeForWrite, timeout);
  } else {
    throw IllegalStateException("Execution::execute: should not be here");
  }
  return m_rc;
}

GfErrType ExecutionImpl::getFuncAttributes(
    const std::string& func, std::shared_ptr<std::vector<int8_t>>* attr) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(m_pool.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }

  GfErrType err = GF_NOERR;

  // do TCR GET_FUNCTION_ATTRIBUTES
  LOG_DEBUG("Tcrmessage request GET_FUNCTION_ATTRIBUTES ");
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
      *attr = reply.getFunctionAttributes();
      break;
    }
    case TcrMessage::EXCEPTION: {
      err = dynamic_cast<ThinClientRegion*>(m_region.get())
                ->handleServerException("Region::GET_FUNCTION_ATTRIBUTES",
                                        reply.getException());
      break;
    }
    case TcrMessage::REQUEST_DATA_ERROR: {
      LOG_ERROR("Error message from server: " + reply.getValue()->toString());
      throw FunctionExecutionException(reply.getValue()->toString());
    }
    default: {
      LOG_ERROR("Unknown message type %d while getting function attributes.",
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
                                        uint8_t getResult,
                                        std::chrono::milliseconds timeout) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(m_pool.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }
  std::shared_ptr<CacheableString> exceptionPtr = nullptr;
  GfErrType err = tcrdm->sendRequestToAllServers(
      func.c_str(), getResult, timeout, m_args, m_rc, exceptionPtr);

  if (err != GF_NOERR) {
    LOG_DEBUG("Execute failed: %d", err);
    if (err == GF_CACHESERVER_EXCEPTION) {
      std::string message;
      if (exceptionPtr) {
        message = std::string("Execute: exception at the server side: ") +
                  exceptionPtr->value().c_str();
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
    const std::string& func, uint8_t getResult, int32_t retryAttempts,
    std::chrono::milliseconds timeout) {
  ThinClientPoolDM* tcrdm = dynamic_cast<ThinClientPoolDM*>(m_pool.get());
  if (tcrdm == nullptr) {
    throw IllegalArgumentException(
        "Execute: pool cast to ThinClientPoolDM failed");
  }
  int32_t attempt = 0;

  // auto csArray = tcrdm->getServers();

  // if (csArray != nullptr && csArray->length() != 0) {
  //  for (int i = 0; i < csArray->length(); i++)
  //  {
  //   auto cs = csArray[i];
  //    TcrEndpoint *ep = nullptr;
  //    /*
  //    std::string endpointStr =
  //    Utils::convertHostToCanonicalForm(cs->value().c_str()
  //    );
  //    */
  //    ep = tcrdm->addEP(cs->value().c_str());
  //  }
  //}

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
        funcName, m_args, getResult, tcrdm, timeout);
    TcrMessageReply reply(true, tcrdm);
    auto resultCollector = std::unique_ptr<ChunkedFunctionExecutionResponse>(
        new ChunkedFunctionExecutionResponse(reply, (getResult & 2) == 2,
                                             m_rc));
    reply.setChunkedResultHandler(resultCollector.get());
    reply.setTimeout(timeout);

    GfErrType err = GF_NOERR;
    err = tcrdm->sendSyncRequest(msg, reply, !(getResult & 1));
    LOG_FINE("executeOnPool %d attempt = %d retryAttempts = %d", err, attempt,
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
      if (getResult & 1) {
        resultCollector->reset();
        m_rc->clearResults();
        attempt++;
        if (attempt > 0) {
          getResult |= 8;  // Send this on server, so that it can identify that
                           // it is a retry attempt.
        }
        continue;
      } else {
        throwExceptionIfError("ExecuteOnPool:", err);
      }
    }
    return nullptr;
  }
  return nullptr;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
