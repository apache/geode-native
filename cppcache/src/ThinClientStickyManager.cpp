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
#include "ThinClientStickyManager.hpp"

#include "ThinClientPoolDM.hpp"
namespace apache {
namespace geode {
namespace client {
bool ThinClientStickyManager::getStickyConnection(
    TcrConnection*& conn, GfErrType* error,
    std::set<ServerLocation>& excludeServers, bool forTransaction) {
  bool maxConnLimit = false;
  bool connFound = false;
  conn = (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();

  if (!conn) {
    conn =
        m_dm->getConnectionFromQueue(true, error, excludeServers, maxConnLimit);
    if (conn) {
      conn->setAndGetBeingUsed(true, forTransaction);
    }
  } else {
    if (!conn->setAndGetBeingUsed(
            true, forTransaction)) {  // manage connection thread is changing
                                      // the connectiion
      conn = m_dm->getConnectionFromQueue(true, error, excludeServers,
                                          maxConnLimit);
      if (conn) {
        connFound = true;
        conn->setAndGetBeingUsed(true, forTransaction);
      }
    } else {
      connFound = true;
    }
  }
  return connFound;
}

void ThinClientStickyManager::getSingleHopStickyConnection(
    TcrEndpoint* theEP, TcrConnection*& conn) {
  conn = (*TssConnectionWrapper::s_geodeTSSConn)
             ->getSHConnection(theEP, m_dm->getName().c_str());
}

void ThinClientStickyManager::addStickyConnection(TcrConnection* conn) {
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
  TcrConnection* oldConn =
      (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();
  if (oldConn) {
    std::set<TcrConnection**>::iterator it = m_stickyConnList.find(
        (*TssConnectionWrapper::s_geodeTSSConn)->getConnDoublePtr());
    if (it != m_stickyConnList.end()) {
      oldConn->setAndGetBeingUsed(false, false);
      m_stickyConnList.erase(it);
      std::shared_ptr<Pool> p = nullptr;
      (*TssConnectionWrapper::s_geodeTSSConn)->setConnection(nullptr, p);
      m_dm->put(oldConn, false);
    }
  }

  if (conn) {
    (*TssConnectionWrapper::s_geodeTSSConn)
        ->setConnection(conn, m_dm->shared_from_this());
    conn->setAndGetBeingUsed(true, true);  // this is done for transaction
                                           // thread when some one resume
                                           // transaction
    m_stickyConnList.insert(
        (*TssConnectionWrapper::s_geodeTSSConn)->getConnDoublePtr());
  }
}

void ThinClientStickyManager::setStickyConnection(TcrConnection* conn,
                                                  bool forTransaction) {
  if (!conn) {
    std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
    (*TssConnectionWrapper::s_geodeTSSConn)
        ->setConnection(nullptr, m_dm->shared_from_this());
  } else {
    TcrConnection* currentConn =
        (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();
    if (currentConn != conn)  // otherwsie no need to set it again
    {
      std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
      (*TssConnectionWrapper::s_geodeTSSConn)
          ->setConnection(conn, m_dm->shared_from_this());
      conn->setAndGetBeingUsed(
          false,
          forTransaction);  // if transaction then it will keep this as used
      m_stickyConnList.insert(
          (*TssConnectionWrapper::s_geodeTSSConn)->getConnDoublePtr());
    } else {
      currentConn->setAndGetBeingUsed(
          false,
          forTransaction);  // if transaction then it will keep this as used
    }
  }
}

void ThinClientStickyManager::setSingleHopStickyConnection(
    TcrEndpoint* ep, TcrConnection*& conn) {
  (*TssConnectionWrapper::s_geodeTSSConn)->setSHConnection(ep, conn);
}

void ThinClientStickyManager::cleanStaleStickyConnection() {
  LOGDEBUG("Cleaning sticky connections");
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);

  auto maxConnLimit = false;
  std::set<ServerLocation> excludeServers;
  for (auto it = m_stickyConnList.begin(); it != m_stickyConnList.end();) {
    auto conn = (*it);
    if (*conn) {
      if ((*conn)->setAndGetBeingUsed(true, false) &&
          canThisConnBeDeleted(*conn)) {
        auto err = GF_NOERR;
        if (auto temp = m_dm->getConnectionFromQueue(
                false, &err, excludeServers, maxConnLimit)) {
          auto temp1 = *conn;
          //*conn = temp; instead of setting in thread local,
          // put in queue, thread will come and pick it from there
          *conn = nullptr;
          m_dm->put(temp, false);
          temp1->close();
          _GEODE_SAFE_DELETE(temp1);
          m_dm->removeEPConnections(1, false);
          LOGDEBUG("Replaced a sticky connection");
        } else {
          (*conn)->setAndGetBeingUsed(false, false);
        }
      }
      ++it;
    } else {
      it = m_stickyConnList.erase(it);
    }
  }
}

void ThinClientStickyManager::closeAllStickyConnections() {
  LOGDEBUG("ThinClientStickyManager::closeAllStickyConnections()");
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
  for (std::set<TcrConnection**>::iterator it = m_stickyConnList.begin();
       it != m_stickyConnList.end(); it++) {
    TcrConnection** tempConn = *it;
    if (*tempConn) {
      (*tempConn)->close();
      _GEODE_SAFE_DELETE(*tempConn);
      m_dm->removeEPConnections(1, false);
    }
  }
}
bool ThinClientStickyManager::canThisConnBeDeleted(TcrConnection* conn) {
  bool canBeDeleted = false;
  LOGDEBUG("ThinClientStickyManager::canThisConnBeDeleted()");
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
  if (m_dm->canItBeDeletedNoImpl(conn)) return true;
  TcrEndpoint* endPt = conn->getEndpointObject();
  std::lock_guard<decltype(endPt->getQueueHostedMutex())> guardQueue(
      endPt->getQueueHostedMutex());
  if (endPt->isQueueHosted()) {
    for (std::set<TcrConnection**>::iterator it = m_stickyConnList.begin();
         it != m_stickyConnList.end(); it++) {
      TcrConnection* connTemp2 = *(*it);
      if (connTemp2 && connTemp2->getEndpointObject() == endPt) {
        canBeDeleted = true;
        break;
      }
    }
  }
  return canBeDeleted;
}
void ThinClientStickyManager::releaseThreadLocalConnection() {
  TcrConnection* conn =
      (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();
  if (conn) {
    std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
    std::set<TcrConnection**>::iterator it = m_stickyConnList.find(
        (*TssConnectionWrapper::s_geodeTSSConn)->getConnDoublePtr());
    LOGDEBUG("ThinClientStickyManager::releaseThreadLocalConnection()");
    if (it != m_stickyConnList.end()) {
      m_stickyConnList.erase(it);
      conn->setAndGetBeingUsed(false,
                               false);  // now this can be used by next one
      m_dm->put(conn, false);
    }
    (*TssConnectionWrapper::s_geodeTSSConn)
        ->setConnection(nullptr, m_dm->shared_from_this());
  }
  (*TssConnectionWrapper::s_geodeTSSConn)
      ->releaseSHConnections(m_dm->shared_from_this());
}

void ThinClientStickyManager::getAnyConnection(TcrConnection*& conn) {
  conn = (*TssConnectionWrapper::s_geodeTSSConn)
             ->getAnyConnection(m_dm->getName().c_str());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
