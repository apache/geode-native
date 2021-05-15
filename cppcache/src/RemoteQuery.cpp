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

#include "RemoteQuery.hpp"

#include "ResultSetImpl.hpp"
#include "StructSetImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"
#include "UserAttributes.hpp"
#include "util/bounds.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

RemoteQuery::RemoteQuery(
    std::string querystr,
    const std::shared_ptr<RemoteQueryService>& queryService,
    ThinClientBaseDM* tccdmptr, AuthenticatedView* authenticatedView)
    : m_queryString(querystr),
      m_queryService(queryService),
      m_tccdm(tccdmptr),
      m_authenticatedView(authenticatedView) {
  LOG_FINEST("RemoteQuery: created a new query: " + querystr);
}

std::shared_ptr<SelectResults> RemoteQuery::execute(
    std::chrono::milliseconds timeout) {
  util::PROTOCOL_OPERATION_TIMEOUT_BOUNDS(timeout);
  GuardUserAttributes gua;
  if (m_authenticatedView != nullptr) {
    gua.setAuthenticatedView(m_authenticatedView);
  }
  return execute(timeout, "Query::execute", m_tccdm, nullptr);
}

std::shared_ptr<SelectResults> RemoteQuery::execute(
    std::shared_ptr<CacheableVector> paramList,
    std::chrono::milliseconds timeout) {
  util::PROTOCOL_OPERATION_TIMEOUT_BOUNDS(timeout);
  GuardUserAttributes gua;
  if (m_authenticatedView) {
    gua.setAuthenticatedView(m_authenticatedView);
  }
  return execute(timeout, "Query::execute", m_tccdm, paramList);
}

std::shared_ptr<SelectResults> RemoteQuery::execute(
    std::chrono::milliseconds timeout, const char* func, ThinClientBaseDM* tcdm,
    std::shared_ptr<CacheableVector> paramList) {
  auto pool = dynamic_cast<ThinClientPoolDM*>(tcdm);
  if (pool) {
    pool->getStats().incQueryExecutionId();
  }
  /*get the start time for QueryExecutionTime stat*/
  // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
  bool enableTimeStatistics = tcdm->getConnectionManager()
                                  .getCacheImpl()
                                  ->getDistributedSystem()
                                  .getSystemProperties()
                                  .getEnableTimeStatistics();
  int64_t sampleStartNanos =
      enableTimeStatistics ? Utils::startStatOpTime() : 0;
  TcrMessageReply reply(true, tcdm);
  auto resultCollector =
      std::unique_ptr<ChunkedQueryResponse>(new ChunkedQueryResponse(reply));
  reply.setChunkedResultHandler(
      static_cast<TcrChunkedResult*>(resultCollector.get()));
  GfErrType err = executeNoThrow(timeout, reply, func, tcdm, paramList);
  throwExceptionIfError(func, err);

  std::shared_ptr<SelectResults> sr;

  LOG_FINEST("%s: reading reply for query: %s", func, m_queryString.c_str());
  auto&& values = resultCollector->getQueryResults();
  auto&& fieldNameVec = resultCollector->getStructFieldNames();
  size_t sizeOfFieldNamesVec = fieldNameVec.size();
  if (sizeOfFieldNamesVec == 0) {
    LOG_FINEST("%s: creating ResultSet for query: %s", func,
               m_queryString.c_str());
    sr = std::make_shared<ResultSetImpl>(values);
  } else {
    if (values->size() % fieldNameVec.size() != 0) {
      char exMsg[1024];
      std::snprintf(exMsg, 1023,
                    "%s: Number of values coming from "
                    "server has to be exactly divisible by field count",
                    func);
      throw MessageException(exMsg);
    } else {
      LOG_FINEST("%s: creating StructSet for query: %s", func,
                 m_queryString.c_str());
      sr = std::make_shared<StructSetImpl>(values, fieldNameVec);
    }
  }

  /*update QueryExecutionTime stat */
  if (pool && enableTimeStatistics) {
    Utils::updateStatOpTime(pool->getStats().getStats(),
                            pool->getStats().getQueryExecutionTimeId(),
                            sampleStartNanos);
  }
  return sr;
}

GfErrType RemoteQuery::executeNoThrow(
    std::chrono::milliseconds timeout, TcrMessageReply& reply, const char* func,
    ThinClientBaseDM* tcdm, std::shared_ptr<CacheableVector> paramList) {
  LOG_FINEST("%s: executing query: %s", func, m_queryString.c_str());

  TryReadGuard guard(m_queryService->getLock(), m_queryService->invalid());

  if (m_queryService->invalid()) {
    return GF_CACHE_CLOSED_EXCEPTION;
  }
  LOG_DEBUG("%s: creating QUERY TcrMessage for query: %s", func,
            m_queryString.c_str());
  if (paramList != nullptr) {
    // QUERY_WITH_PARAMETERS
    TcrMessageQueryWithParameters msg(
        new DataOutput(
            m_tccdm->getConnectionManager().getCacheImpl()->createDataOutput()),
        m_queryString, nullptr, paramList, timeout, tcdm);
    msg.setTimeout(timeout);
    reply.setTimeout(timeout);

    LOG_FINEST("%s: sending request for query: %s", func,
               m_queryString.c_str());
    if (tcdm == nullptr) {
      tcdm = m_tccdm;
    }
    auto err = tcdm->sendSyncRequest(msg, reply);
    if (err != GF_NOERR) {
      return err;
    }
    if (reply.getMessageType() == TcrMessage::EXCEPTION) {
      err = ThinClientRegion::handleServerException(func, reply.getException());
      if (err == GF_CACHESERVER_EXCEPTION) {
        err = GF_REMOTE_QUERY_EXCEPTION;
      }
    }
    return err;
  } else {
    TcrMessageQuery msg(
        new DataOutput(
            m_tccdm->getConnectionManager().getCacheImpl()->createDataOutput()),
        m_queryString, timeout, tcdm);
    msg.setTimeout(timeout);
    reply.setTimeout(timeout);

    LOG_FINEST("%s: sending request for query: %s", func,
               m_queryString.c_str());
    if (tcdm == nullptr) {
      tcdm = m_tccdm;
    }
    auto err = tcdm->sendSyncRequest(msg, reply);
    if (err != GF_NOERR) {
      return err;
    }
    if (reply.getMessageType() == TcrMessage::EXCEPTION) {
      err = ThinClientRegion::handleServerException(func, reply.getException());
      if (err == GF_CACHESERVER_EXCEPTION) {
        err = GF_REMOTE_QUERY_EXCEPTION;
      }
    }
    return err;
  }
}

const std::string& RemoteQuery::getQueryString() const { return m_queryString; }

void RemoteQuery::compile() {
  throw UnsupportedOperationException("Not supported in native clients");
}

bool RemoteQuery::isCompiled() {
  throw UnsupportedOperationException("Not supported in native clients");
}

}  // namespace client
}  // namespace geode
}  // namespace apache
