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

#ifndef GEODE_TSSCONNECTIONWRAPPER_H_
#define GEODE_TSSCONNECTIONWRAPPER_H_

#include <map>
#include <string>

#include <ace/TSS_T.h>

#include <geode/Pool.hpp>

namespace apache {
namespace geode {
namespace client {

class TcrEndpoint;
class TcrConnection;

typedef std::map<std::string, TcrConnection*> EpNameVsConnection;

class PoolWrapper {
 private:
  std::shared_ptr<Pool> m_pool;
  EpNameVsConnection m_EpnameVsConnection;
  PoolWrapper& operator=(const PoolWrapper&);
  PoolWrapper(const PoolWrapper&);

 public:
  TcrConnection* getSHConnection(TcrEndpoint* ep);
  void setSHConnection(TcrEndpoint* ep, TcrConnection* conn);
  PoolWrapper();
  ~PoolWrapper();
  void releaseSHConnections(std::shared_ptr<Pool> pool);
  TcrConnection* getAnyConnection();
};

typedef std::map<std::string, PoolWrapper*> poolVsEndpointConnMap;

class TssConnectionWrapper {
 private:
  TcrConnection* m_tcrConn;
  std::shared_ptr<Pool> m_pool;
  poolVsEndpointConnMap m_poolVsEndpointConnMap;
  TssConnectionWrapper& operator=(const TssConnectionWrapper&);
  TssConnectionWrapper(const TssConnectionWrapper&);

 public:
  // TODO shared_ptr - remove or refactor with global work
  static ACE_TSS<TssConnectionWrapper>* s_geodeTSSConn;
  TcrConnection* getConnection();
  TcrConnection* getSHConnection(TcrEndpoint* ep, const char* poolname);
  void setConnection(TcrConnection* conn, const std::shared_ptr<Pool>& pool);
  void setSHConnection(TcrEndpoint* ep, TcrConnection* conn);
  TcrConnection** getConnDoublePtr();
  TssConnectionWrapper();
  ~TssConnectionWrapper();
  void releaseSHConnections(std::shared_ptr<Pool> p);
  TcrConnection* getAnyConnection(const char* poolname);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TSSCONNECTIONWRAPPER_H_
