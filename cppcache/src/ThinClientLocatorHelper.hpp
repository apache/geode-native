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

#ifndef GEODE_THINCLIENTLOCATORHELPER_H_
#define GEODE_THINCLIENTLOCATORHELPER_H_

#include <list>
#include <mutex>
#include <set>
#include <string>

#include <geode/internal/geode_globals.hpp>

#include "ClientProxyMembershipID.hpp"
#include "ErrType.hpp"
#include "GetAllServersRequest.hpp"
#include "GetAllServersResponse.hpp"
#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientPoolDM;
class Connector;

class ThinClientLocatorHelper {
 public:
  ThinClientLocatorHelper(const std::vector<std::string>& locatorAddresses,
                          const ThinClientPoolDM* poolDM);
  ThinClientLocatorHelper(const std::vector<std::string>& locatorAddresses,
                          const std::string& sniProxyHost, int sniProxyPort,
                          const ThinClientPoolDM* poolDM);
  GfErrType getEndpointForNewFwdConn(
      ServerLocation& outEndpoint, std::string& additionalLoc,
      const std::set<ServerLocation>& exclEndPts,
      const std::string& serverGrp = "",
      const TcrConnection* currentServer = nullptr);
  GfErrType getEndpointForNewCallBackConn(
      ClientProxyMembershipID& memId, std::list<ServerLocation>& outEndpoint,
      std::string& additionalLoc, int redundancy,
      const std::set<ServerLocation>& exclEndPts, const std::string& serverGrp);
  GfErrType getAllServers(
      std::vector<std::shared_ptr<ServerLocation> >& servers,
      const std::string& serverGrp);
  int32_t getCurLocatorsNum() {
    return static_cast<int32_t>(m_locHostPort.size());
  }
  GfErrType updateLocators(const std::string& serverGrp = "");

 private:
  Connector* createConnection(Connector*& conn, const char* hostname,
                              int32_t port,
                              std::chrono::microseconds waitSeconds,
                              int32_t maxBuffSizePool = 0);
  std::mutex m_locatorLock;
  std::vector<ServerLocation> m_locHostPort;
  const ThinClientPoolDM* m_poolDM;
  ThinClientLocatorHelper(const ThinClientLocatorHelper&);
  ThinClientLocatorHelper& operator=(const ThinClientLocatorHelper&);
  std::string m_sniProxyHost;
  int m_sniProxyPort;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTLOCATORHELPER_H_
