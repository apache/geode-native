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
#include "TcpSslConn.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"

namespace apache {
namespace geode {
namespace client {

const int BUFF_SIZE = 3000;

class ConnectionWrapper {
 private:
  Connector*& m_conn;

 public:
  explicit ConnectionWrapper(Connector*& conn) : m_conn(conn) {}
  ~ConnectionWrapper() {
    LOGDEBUG("closing the connection locator1");
    if (m_conn != nullptr) {
      LOGDEBUG("closing the connection locator");
      m_conn->close();
      delete m_conn;
    }
  }
};

ThinClientLocatorHelper::ThinClientLocatorHelper(
    const std::vector<std::string>& locatorAddresses,
    const ThinClientPoolDM* poolDM)
    : m_poolDM(poolDM) {
  for (auto&& locatorAddress : locatorAddresses) {
    m_locHostPort.emplace_back(locatorAddress);
  }
}

ThinClientLocatorHelper::ThinClientLocatorHelper(
    const std::vector<std::string>& locatorAddresses,
    const std::string& sniProxyHost, int sniProxyPort,
    const ThinClientPoolDM* poolDM)
    : m_poolDM(poolDM),
      m_sniProxyHost(sniProxyHost),
      m_sniProxyPort(sniProxyPort) {
  for (auto&& locatorAddress : locatorAddresses) {
    m_locHostPort.emplace_back(locatorAddress);
  }
}

Connector* ThinClientLocatorHelper::createConnection(
    Connector*& conn, const char* hostname, int32_t port,
    std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool) {
  Connector* socket = nullptr;
  auto& systemProperties = m_poolDM->getConnectionManager()
                               .getCacheImpl()
                               ->getDistributedSystem()
                               .getSystemProperties();
  if (systemProperties.sslEnabled()) {
    if (m_sniProxyHost.empty()) {
      socket = new TcpSslConn(
          hostname, static_cast<uint16_t>(port), waitSeconds, maxBuffSizePool,
          systemProperties.sslTrustStore(), systemProperties.sslKeyStore(),
          systemProperties.sslKeystorePassword());
    } else {
      socket =
          new TcpSslConn(hostname, waitSeconds, maxBuffSizePool, m_sniProxyHost,
                         m_sniProxyPort, systemProperties.sslTrustStore(),
                         systemProperties.sslKeyStore(),
                         systemProperties.sslKeystorePassword());
    }
  } else {
    socket = new TcpConn(hostname, port, waitSeconds, maxBuffSizePool);
  }
  conn = socket;
  socket->init();
  return socket;
}

GfErrType ThinClientLocatorHelper::getAllServers(
    std::vector<std::shared_ptr<ServerLocation> >& servers,
    const std::string& serverGrp) {
  std::lock_guard<decltype(m_locatorLock)> guard(m_locatorLock);

  auto& sysProps = m_poolDM->getConnectionManager()
                       .getCacheImpl()
                       ->getDistributedSystem()
                       .getSystemProperties();
  for (size_t i = 0; i < m_locHostPort.size(); i++) {
    auto& loc = m_locHostPort[i];
    try {
      LOGDEBUG("getAllServers getting servers from server = %s ",
               loc.getServerName().c_str());
      auto buffSize = m_poolDM->getSocketBufferSize();
      Connector* conn = nullptr;
      ConnectionWrapper cw(conn);
      createConnection(conn, loc.getServerName().c_str(), loc.getPort(),
                       sysProps.connectTimeout(), buffSize);
      auto request = std::make_shared<GetAllServersRequest>(serverGrp);
      auto data =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput();
      data.writeInt(static_cast<int32_t>(1001));  // GOSSIPVERSION
      data.writeObject(request);
      auto sentLength = conn->send(
          reinterpret_cast<char*>(const_cast<uint8_t*>(data.getBuffer())),
          data.getBufferLength(), m_poolDM->getReadTimeout());
      if (sentLength <= 0) {
        continue;
      }
      char buff[BUFF_SIZE];
      auto receivedLength =
          conn->receive(buff, BUFF_SIZE, m_poolDM->getReadTimeout());
      if (receivedLength <= 0) {
        continue;
      }

      auto di =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataInput(
              reinterpret_cast<uint8_t*>(buff), receivedLength);

      if (di.read() == REPLY_SSL_ENABLED && !sysProps.sslEnabled()) {
        LOGERROR("SSL is enabled on locator, enable SSL in client as well");
        throw AuthenticationRequiredException(
            "SSL is enabled on locator, enable SSL in client as well");
      }
      di.rewindCursor(1);

      auto response =
          std::dynamic_pointer_cast<GetAllServersResponse>(di.readObject());
      servers = response->getServers();
      return GF_NOERR;
    } catch (const AuthenticationRequiredException&) {
      continue;
    } catch (const Exception& excp) {
      LOGFINE("Exception while querying locator: %s: %s",
              excp.getName().c_str(), excp.what());
      continue;
    }
  }
  return GF_NOERR;
}

GfErrType ThinClientLocatorHelper::getEndpointForNewCallBackConn(
    ClientProxyMembershipID& memId, std::list<ServerLocation>& outEndpoint,
    std::string&, int redundancy, const std::set<ServerLocation>& exclEndPts,
    const std::string& serverGrp) {
  std::lock_guard<decltype(m_locatorLock)> guard(m_locatorLock);
  auto& sysProps = m_poolDM->getConnectionManager()
                       .getCacheImpl()
                       ->getDistributedSystem()
                       .getSystemProperties();
  int locatorsRetry = 3;
  if (m_poolDM) {
    int poolRetry = m_poolDM->getRetryAttempts();
    locatorsRetry = poolRetry <= 0 ? locatorsRetry : poolRetry;
  }
  LOGFINER(
      "ThinClientLocatorHelper::getEndpointForNewCallBackConn locatorsRetry = "
      "%d ",
      locatorsRetry);
  for (unsigned attempts = 0;
       attempts <
       (m_locHostPort.size() == 1 ? locatorsRetry : m_locHostPort.size());
       attempts++) {
    ServerLocation loc;
    if (m_locHostPort.size() == 1) {
      loc = m_locHostPort[0];
    } else {
      loc = m_locHostPort[attempts];
    }

    try {
      LOGFINER("Querying locator at [%s:%d] for queue server from group [%s]",
               loc.getServerName().c_str(), loc.getPort(), serverGrp.c_str());
      auto buffSize = m_poolDM->getSocketBufferSize();
      Connector* conn = nullptr;
      ConnectionWrapper cw(conn);
      createConnection(conn, loc.getServerName().c_str(), loc.getPort(),
                       sysProps.connectTimeout(), buffSize);
      auto request = std::make_shared<QueueConnectionRequest>(
          memId, exclEndPts, redundancy, false, serverGrp);
      auto data =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput();
      data.writeInt(static_cast<int32_t>(1001));  // GOSSIPVERSION
      data.writeObject(request);
      auto sentLength = conn->send(
          reinterpret_cast<char*>(const_cast<uint8_t*>(data.getBuffer())),
          data.getBufferLength(), m_poolDM->getReadTimeout());
      if (sentLength <= 0) {
        continue;
      }
      char buff[BUFF_SIZE];
      auto receivedLength =
          conn->receive(buff, BUFF_SIZE, m_poolDM->getReadTimeout());
      if (receivedLength <= 0) {
        continue;
      }
      auto di =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataInput(
              reinterpret_cast<uint8_t*>(buff), receivedLength);

      const auto acceptanceCode = di.read();
      if (acceptanceCode == REPLY_SSL_ENABLED && !sysProps.sslEnabled()) {
        LOGERROR("SSL is enabled on locator, enable SSL in client as well");
        throw AuthenticationRequiredException(
            "SSL is enabled on locator, enable SSL in client as well");
      }
      di.rewindCursor(1);
      auto response =
          std::dynamic_pointer_cast<QueueConnectionResponse>(di.readObject());
      outEndpoint = response->getServers();
      return GF_NOERR;
    } catch (const AuthenticationRequiredException& excp) {
      throw excp;
    } catch (const Exception& excp) {
      LOGFINE("Exception while querying locator: %s: %s",
              excp.getName().c_str(), excp.what());
      continue;
    }
  }
  throw NoAvailableLocatorsException("Unable to query any locators");
}

GfErrType ThinClientLocatorHelper::getEndpointForNewFwdConn(
    ServerLocation& outEndpoint, std::string&,
    const std::set<ServerLocation>& exclEndPts, const std::string& serverGrp,
    const TcrConnection* currentServer) {
  bool locatorFound = false;
  std::lock_guard<decltype(m_locatorLock)> guard(m_locatorLock);
  auto& sysProps = m_poolDM->getConnectionManager()
                       .getCacheImpl()
                       ->getDistributedSystem()
                       .getSystemProperties();

  auto poolRetry = m_poolDM->getRetryAttempts();
  auto locatorsRetry = poolRetry <= 0 ? 3 : poolRetry;
  LOGFINER(
      "ThinClientLocatorHelper::getEndpointForNewFwdConn locatorsRetry = %d ",
      locatorsRetry);
  for (unsigned attempts = 0;
       attempts <
       (m_locHostPort.size() == 1 ? locatorsRetry : m_locHostPort.size());
       attempts++) {
    ServerLocation serLoc;
    if (m_locHostPort.size() == 1) {
      serLoc = m_locHostPort[0];
    } else {
      serLoc = m_locHostPort[attempts];
    }
    try {
      LOGFINE("Querying locator at [%s:%d] for server from group [%s]",
              serLoc.getServerName().c_str(), serLoc.getPort(),
              serverGrp.c_str());
      auto buffSize = m_poolDM->getSocketBufferSize();
      Connector* conn = nullptr;
      ConnectionWrapper cw(conn);
      createConnection(conn, serLoc.getServerName().c_str(), serLoc.getPort(),
                       sysProps.connectTimeout(), buffSize);
      auto data =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput();
      data.writeInt(1001);  // GOSSIPVERSION
      if (currentServer == nullptr) {
        LOGDEBUG("Creating ClientConnectionRequest");
        std::shared_ptr<ClientConnectionRequest> request =
            std::make_shared<ClientConnectionRequest>(exclEndPts, serverGrp);
        data.writeObject(request);
      } else {
        LOGDEBUG("Creating ClientReplacementRequest for connection: ",
                 currentServer->getEndpointObject()->name().c_str());
        std::shared_ptr<ClientReplacementRequest> request =
            std::make_shared<ClientReplacementRequest>(
                currentServer->getEndpointObject()->name(), exclEndPts,
                serverGrp);
        data.writeObject(request);
      }
      auto sentLength = conn->send(
          reinterpret_cast<char*>(const_cast<uint8_t*>(data.getBuffer())),
          data.getBufferLength(), m_poolDM->getReadTimeout());
      if (sentLength <= 0) {
        continue;
      }
      char buff[BUFF_SIZE];
      auto receivedLength =
          conn->receive(buff, BUFF_SIZE, m_poolDM->getReadTimeout());
      if (receivedLength <= 0) {
        continue;  // return GF_EUNDEF;
      }
      auto di =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataInput(
              reinterpret_cast<uint8_t*>(buff), receivedLength);

      const auto acceptanceCode = di.read();
      if (acceptanceCode == REPLY_SSL_ENABLED && !sysProps.sslEnabled()) {
        LOGERROR("SSL is enabled on locator, enable SSL in client as well");
        throw AuthenticationRequiredException(
            "SSL is enabled on locator, enable SSL in client as well");
      }
      di.rewindCursor(1);

      auto response =
          std::dynamic_pointer_cast<ClientConnectionResponse>(di.readObject());
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
    } catch (const AuthenticationRequiredException& excp) {
      throw excp;
    } catch (const Exception& excp) {
      LOGFINE("Exception while querying locator: %s: %s",
              excp.getName().c_str(), excp.what());
      continue;
    }
  }

  if (locatorFound) {
    throw NotConnectedException("No servers found");
  } else {
    throw NoAvailableLocatorsException("Unable to query any locators");
  }
}

GfErrType ThinClientLocatorHelper::updateLocators(
    const std::string& serverGrp) {
  std::lock_guard<decltype(m_locatorLock)> guard(m_locatorLock);
  auto& sysProps = m_poolDM->getConnectionManager()
                       .getCacheImpl()
                       ->getDistributedSystem()
                       .getSystemProperties();

  for (size_t attempts = 0; attempts < m_locHostPort.size(); attempts++) {
    auto&& serLoc = m_locHostPort[attempts];
    Connector* conn = nullptr;
    try {
      auto buffSize = m_poolDM->getSocketBufferSize();
      LOGFINER("Querying locator list at: [%s:%d] for update from group [%s]",
               serLoc.getServerName().c_str(), serLoc.getPort(),
               serverGrp.c_str());
      ConnectionWrapper cw(conn);
      createConnection(conn, serLoc.getServerName().c_str(), serLoc.getPort(),
                       sysProps.connectTimeout(), buffSize);
      auto request = std::make_shared<LocatorListRequest>(serverGrp);
      auto data =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataOutput();
      data.writeInt(static_cast<int32_t>(1001));  // GOSSIPVERSION
      data.writeObject(request);
      auto sentLength = conn->send(
          reinterpret_cast<char*>(const_cast<uint8_t*>(data.getBuffer())),
          data.getBufferLength(), m_poolDM->getReadTimeout());
      if (sentLength <= 0) {
        conn = nullptr;
        continue;
      }
      char buff[BUFF_SIZE];
      auto receivedLength =
          conn->receive(buff, BUFF_SIZE, m_poolDM->getReadTimeout());
      if (receivedLength <= 0) {
        continue;
      }
      auto di =
          m_poolDM->getConnectionManager().getCacheImpl()->createDataInput(
              reinterpret_cast<uint8_t*>(buff), receivedLength);

      const auto acceptanceCode = di.read();
      if (acceptanceCode == REPLY_SSL_ENABLED && !sysProps.sslEnabled()) {
        LOGERROR("SSL is enabled on locator, enable SSL in client as well");
        throw AuthenticationRequiredException(
            "SSL is enabled on locator, enable SSL in client as well");
      }
      di.rewindCursor(1);

      auto response =
          std::dynamic_pointer_cast<LocatorListResponse>(di.readObject());
      auto locators = response->getLocators();
      if (locators.size() > 0) {
        RandGen randGen;
        std::random_shuffle(locators.begin(), locators.end(), randGen);
      }
      std::vector<ServerLocation> temp(m_locHostPort.begin(),
                                       m_locHostPort.end());
      m_locHostPort.clear();
      m_locHostPort.insert(m_locHostPort.end(), locators.begin(),
                           locators.end());
      for (std::vector<ServerLocation>::iterator it = temp.begin();
           it != temp.end(); it++) {
        std::vector<ServerLocation>::iterator it1 =
            std::find(m_locHostPort.begin(), m_locHostPort.end(), *it);
        if (it1 == m_locHostPort.end()) {
          m_locHostPort.push_back(*it);
        }
      }
      return GF_NOERR;
    } catch (const AuthenticationRequiredException& excp) {
      throw excp;
    } catch (const Exception& excp) {
      LOGFINE("Exception while querying locator: %s: %s",
              excp.getName().c_str(), excp.what());
      continue;
    }
  }
  return GF_NOTCON;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
