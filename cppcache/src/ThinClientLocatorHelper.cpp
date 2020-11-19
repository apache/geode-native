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

#include "ThinClientLocatorHelper.hpp"

#include <algorithm>
#include <set>

#include <boost/thread/lock_types.hpp>

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/SystemProperties.hpp>

#include "ClientConnectionRequest.hpp"
#include "ClientConnectionResponse.hpp"
#include "ClientReplacementRequest.hpp"
#include "LocatorListRequest.hpp"
#include "LocatorListResponse.hpp"
#include "QueueConnectionRequest.hpp"
#include "QueueConnectionResponse.hpp"
#include "TcpConn.hpp"
#include "TcpSslConn.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

const size_t BUFF_SIZE = 3000;
const size_t DEFAULT_CONNECTION_RETRIES = 3;

ThinClientLocatorHelper::ThinClientLocatorHelper(
    const std::vector<std::string>& locators, const ThinClientPoolDM* poolDM)
    : locators_(locators.begin(), locators.end()),
      m_poolDM(poolDM),
      m_sniProxyHost(""),
      m_sniProxyPort(0) {}

ThinClientLocatorHelper::ThinClientLocatorHelper(
    const std::vector<std::string>& locators, const std::string& sniProxyHost,
    int sniProxyPort, const ThinClientPoolDM* poolDM)
    : locators_(locators.begin(), locators.end()),
      m_poolDM(poolDM),
      m_sniProxyHost(sniProxyHost),
      m_sniProxyPort(sniProxyPort) {}

size_t ThinClientLocatorHelper::getConnRetries() const {
  auto retries = m_poolDM->getRetryAttempts();
  return retries <= 0 ? DEFAULT_CONNECTION_RETRIES : retries;
}

std::vector<ServerLocation> ThinClientLocatorHelper::getLocators() const {
  decltype(locators_) locators;
  {
    boost::shared_lock<decltype(mutex_)> guard{mutex_};
    if (locators_.empty()) {
      return {};
    }

    locators = locators_;
  }

  RandGen randGen;
  std::random_shuffle(locators.begin(), locators.end(), randGen);
  return locators;
}

std::unique_ptr<Connector> ThinClientLocatorHelper::createConnection(
    const ServerLocation& location) const {
  auto& sys_prop = m_poolDM->getConnectionManager()
                       .getCacheImpl()
                       ->getDistributedSystem()
                       .getSystemProperties();

  const auto port = location.getPort();
  auto timeout = sys_prop.connectTimeout();
  const auto& hostname = location.getServerName();
  auto buffer_size = m_poolDM->getSocketBufferSize();

  if (sys_prop.sslEnabled()) {
    if (m_sniProxyHost.empty()) {
      return std::unique_ptr<Connector>(new TcpSslConn(
          hostname, static_cast<uint16_t>(port), timeout, buffer_size,
          sys_prop.sslTrustStore(), sys_prop.sslKeyStore(),
          sys_prop.sslKeystorePassword()));
    } else {
      return std::unique_ptr<Connector>(new TcpSslConn(
          hostname, static_cast<uint16_t>(port), m_sniProxyHost, m_sniProxyPort,
          timeout, buffer_size, sys_prop.sslTrustStore(),
          sys_prop.sslKeyStore(), sys_prop.sslKeystorePassword()));
    }
  } else {
    return std::unique_ptr<Connector>(new TcpConn(
        hostname, static_cast<uint16_t>(port), timeout, buffer_size));
  }
}

std::shared_ptr<Serializable> ThinClientLocatorHelper::sendRequest(
    const ServerLocation& location,
    const std::shared_ptr<Serializable>& request) const {
  auto& sys_prop = m_poolDM->getConnectionManager()
                       .getCacheImpl()
                       ->getDistributedSystem()
                       .getSystemProperties();

  try {
    auto conn = createConnection(location);
    auto data =
        m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput();
    data.writeInt(static_cast<int32_t>(1001));  // GOSSIPVERSION
    data.writeObject(request);
    auto sentLength = conn->send(
        reinterpret_cast<char*>(const_cast<uint8_t*>(data.getBuffer())),
        data.getBufferLength(), m_poolDM->getReadTimeout());
    if (sentLength <= 0) {
      return nullptr;
    }
    char buff[BUFF_SIZE];
    const auto receivedLength = conn->receive(buff, m_poolDM->getReadTimeout());
    if (!receivedLength) {
      return nullptr;
    }

    auto di = m_poolDM->getConnectionManager().getCacheImpl()->createDataInput(
        reinterpret_cast<uint8_t*>(buff), receivedLength);

    if (di.read() == REPLY_SSL_ENABLED && !sys_prop.sslEnabled()) {
      LOGERROR("SSL is enabled on locator, enable SSL in client as well");
      throw AuthenticationRequiredException(
          "SSL is enabled on locator, enable SSL in client as well");
    }

    di.rewindCursor(1);
    return di.readObject();
  } catch (const AuthenticationRequiredException& excp) {
    throw excp;
  } catch (const Exception& excp) {
    LOGFINE("Exception while querying locator: %s: %s", excp.getName().c_str(),
            excp.what());
  } catch (...) {
    LOGFINE("Exception while querying locator");
  }

  return nullptr;
}

GfErrType ThinClientLocatorHelper::getAllServers(
    std::vector<std::shared_ptr<ServerLocation> >& servers,
    const std::string& serverGrp) const {
  for (const auto& loc : getLocators()) {
    LOGDEBUG("getAllServers getting servers from server = %s ",
             loc.getServerName().c_str());

    auto request = std::make_shared<GetAllServersRequest>(serverGrp);
    auto response = std::dynamic_pointer_cast<GetAllServersResponse>(
        sendRequest(loc, request));
    if (response == nullptr) {
      continue;
    }

    servers = response->getServers();
    return GF_NOERR;
  }

  return GF_NOERR;
}

GfErrType ThinClientLocatorHelper::getEndpointForNewCallBackConn(
    ClientProxyMembershipID& memId, std::list<ServerLocation>& outEndpoint,
    std::string&, int redundancy, const std::set<ServerLocation>& exclEndPts,
    const std::string& serverGrp) const {
  auto locators = getLocators();
  auto locatorsSize = locators.size();
  auto maxAttempts = getConnRetries();

  LOGFINER(
      "ThinClientLocatorHelper::getEndpointForNewCallBackConn maxAttempts = "
      "%zu",
      maxAttempts);

  for (auto attempt = 0ULL; attempt < maxAttempts;) {
    const auto& loc = locators[attempt++ % locatorsSize];
    LOGFINER("Querying locator at [%s:%d] for queue server from group [%s]",
             loc.getServerName().c_str(), loc.getPort(), serverGrp.c_str());

    auto request = std::make_shared<QueueConnectionRequest>(
        memId, exclEndPts, redundancy, false, serverGrp);
    auto response = std::dynamic_pointer_cast<QueueConnectionResponse>(
        sendRequest(loc, request));
    if (response == nullptr) {
      continue;
    }

    outEndpoint = response->getServers();
    return GF_NOERR;
  }

  throw NoAvailableLocatorsException("Unable to query any locators");
}

GfErrType ThinClientLocatorHelper::getEndpointForNewFwdConn(
    ServerLocation& outEndpoint, std::string&,
    const std::set<ServerLocation>& exclEndPts, const std::string& serverGrp,
    const TcrConnection* currentServer) const {
  bool locatorFound = false;
  auto locators = getLocators();
  auto locatorsSize = locators.size();
  auto maxAttempts = getConnRetries();

  LOGFINER(
      "ThinClientLocatorHelper::getEndpointForNewFwdConn maxAttempts = %zu",
      maxAttempts);

  for (auto attempt = 0ULL; attempt < maxAttempts;) {
    const auto& loc = locators[attempt++ % locatorsSize];
    LOGFINE("Querying locator at [%s:%d] for server from group [%s]",
            loc.getServerName().c_str(), loc.getPort(), serverGrp.c_str());

    std::shared_ptr<Serializable> request;
    if (currentServer == nullptr) {
      LOGDEBUG("Creating ClientConnectionRequest");
      request =
          std::make_shared<ClientConnectionRequest>(exclEndPts, serverGrp);
    } else {
      LOGDEBUG("Creating ClientReplacementRequest for connection: %s",
               currentServer->getEndpointObject()->name().c_str());
      request = std::make_shared<ClientReplacementRequest>(
          currentServer->getEndpointObject()->name(), exclEndPts, serverGrp);
    }

    auto response = std::dynamic_pointer_cast<ClientConnectionResponse>(
        sendRequest(loc, request));
    if (response == nullptr) {
      continue;
    }

    response->printInfo();
    if (!response->serverFound()) {
      LOGFINE("Server not found");
      locatorFound = true;
      continue;
    }

    outEndpoint = response->getServerLocation();
    LOGFINE("Server found at [%s:%d]", outEndpoint.getServerName().c_str(),
            outEndpoint.getPort());

    return GF_NOERR;
  }

  if (locatorFound) {
    throw NotConnectedException("No servers found");
  } else {
    throw NoAvailableLocatorsException("Unable to query any locators");
  }
}

GfErrType ThinClientLocatorHelper::updateLocators(
    const std::string& serverGrp) {
  auto locators = getLocators();
  for (const auto& loc : locators) {
    LOGFINER("Querying locator list at: [%s:%d] for update from group [%s]",
             loc.getServerName().c_str(), loc.getPort(), serverGrp.c_str());

    auto request = std::make_shared<LocatorListRequest>(serverGrp);
    auto response = std::dynamic_pointer_cast<LocatorListResponse>(
        sendRequest(loc, request));
    if (response == nullptr) {
      continue;
    }

    auto new_locators = response->getLocators();
    for (const auto& old_loc : locators) {
      auto iter = std::find(new_locators.begin(), new_locators.end(), old_loc);
      if (iter == new_locators.end()) {
        new_locators.push_back(old_loc);
      }
    }

    {
      boost::unique_lock<decltype(mutex_)> lock(mutex_);
      locators_.swap(new_locators);
    }

    return GF_NOERR;
  }
  return GF_NOTCON;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
