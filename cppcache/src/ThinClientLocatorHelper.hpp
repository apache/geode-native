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
#include <set>
#include <string>

#include <boost/thread/shared_mutex.hpp>

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
class TcrConnection;

class ThinClientLocatorHelper {
 public:
  ThinClientLocatorHelper(const ThinClientLocatorHelper&) = delete;
  ThinClientLocatorHelper& operator=(const ThinClientLocatorHelper&) = delete;

  ThinClientLocatorHelper(const std::vector<std::string>& locators,
                          const ThinClientPoolDM* poolDM);
  ThinClientLocatorHelper(const std::vector<std::string>& locators,
                          const std::string& sniProxyHost, int sniProxyPort,
                          const ThinClientPoolDM* poolDM);
  GfErrType getEndpointForNewFwdConn(
      ServerLocation& outEndpoint, std::string& additionalLoc,
      const std::set<ServerLocation>& exclEndPts,
      const std::string& serverGrp = "",
      const TcrConnection* currentServer = nullptr) const;
  GfErrType getEndpointForNewCallBackConn(
      ClientProxyMembershipID& memId, std::list<ServerLocation>& outEndpoint,
      std::string& additionalLoc, int redundancy,
      const std::set<ServerLocation>& exclEndPts,
      const std::string& serverGrp) const;
  GfErrType getAllServers(
      std::vector<std::shared_ptr<ServerLocation> >& servers,
      const std::string& serverGrp) const;
  size_t getCurLocatorsNum() const { return locators_.size(); }
  GfErrType updateLocators(const std::string& serverGrp = "");

 private:
  /**
   * Returns the number of connections retries per request
   * @return Number of connection retries towards locators
   */
  size_t getConnRetries() const;

  /**
   * Returns a shuffled copy of the current locators list
   * @return Locators list
   * @note The original list of locators is copied under the mutex scope
   * @note This method is used instead of directly using the original locators
   *       list in order to avoid having to lock the mutex while establishing
   *       the connection to any of the locators, which is a bad practice in
   *       general.
   */
  std::vector<ServerLocation> getLocators() const;

  /**
   * Creates a connection to the given locator
   * @param location Locator ServerLocation
   * @return A connection for the locator
   */
  std::unique_ptr<Connector> createConnection(
      const ServerLocation& location) const;

  /**
   * Sends a request to the given locator
   * @param location Locator ServerLocation
   * @param request Serializable object of the request
   * @return Serializable object of the response if no error happened,
   *         otherwise it returns a nullptr
   */
  std::shared_ptr<Serializable> sendRequest(
      const ServerLocation& location,
      const std::shared_ptr<Serializable>& request) const;

  /**
   * Data members
   */
  mutable boost::shared_mutex mutex_;
  std::vector<ServerLocation> locators_;
  const ThinClientPoolDM* m_poolDM;
  std::string m_sniProxyHost;
  int m_sniProxyPort;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_THINCLIENTLOCATORHELPER_H_
