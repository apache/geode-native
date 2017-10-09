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

#ifndef GEODE_REMOTEQUERYSERVICE_H_
#define GEODE_REMOTEQUERYSERVICE_H_

#include <ace/Recursive_Thread_Mutex.h>
#include <geode/geode_globals.hpp>
#include <memory>
#include <geode/QueryService.hpp>

#include "CqService.hpp"
#include "ThinClientCacheDistributionManager.hpp"
#include "statistics/StatisticsManager.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheImpl;
class ThinClientPoolDM;
typedef std::map<std::string, bool> CqPoolsConnected;
class CPPCACHE_EXPORT RemoteQueryService
    : public QueryService,
      public std::enable_shared_from_this<RemoteQueryService> {
 public:
  RemoteQueryService(CacheImpl* cptr, ThinClientPoolDM* poolDM = nullptr);

  void init();

  QueryPtr newQuery(const char* querystring);

  inline ACE_RW_Thread_Mutex& getLock() { return m_rwLock; }
  inline const volatile bool& invalid() { return m_invalid; }

  void close();

  ~RemoteQueryService() {}
  virtual CqQueryPtr newCq(const char* querystr, const CqAttributesPtr& cqAttr,
                           bool isDurable = false);
  virtual CqQueryPtr newCq(const char* name, const char* querystr,
                           const CqAttributesPtr& cqAttr,
                           bool isDurable = false);
  virtual void closeCqs();
  virtual QueryService::query_container_type getCqs();
  virtual CqQueryPtr getCq(const char* name);
  virtual void executeCqs();
  virtual void stopCqs();
  virtual CqServiceStatisticsPtr getCqServiceStatistics();
  void executeAllCqs(bool failover);
  virtual CacheableArrayListPtr getAllDurableCqsFromServer();
  /**
   * execute all cqs on the endpoint after failover
   */
  GfErrType executeAllCqs(TcrEndpoint* endpoint);
  void receiveNotification(TcrMessage* msg);
  void invokeCqConnectedListeners(ThinClientPoolDM* pool, bool connected);
  // For Lazy Cq Start-no use, no start
  inline void initCqService() {
    if (m_cqService == nullptr) {
      LOGFINE("RemoteQueryService: starting cq service");
      m_cqService = std::make_shared<CqService>(m_tccdm, m_statisticsFactory);
      LOGFINE("RemoteQueryService: started cq service");
    }
  }

 private:
  volatile bool m_invalid;
  mutable ACE_RW_Thread_Mutex m_rwLock;

  ThinClientBaseDM* m_tccdm;
  CqServicePtr m_cqService;
  CqPoolsConnected m_CqPoolsConnected;
  statistics::StatisticsFactory* m_statisticsFactory;
};

typedef std::shared_ptr<RemoteQueryService> RemoteQueryServicePtr;
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REMOTEQUERYSERVICE_H_
