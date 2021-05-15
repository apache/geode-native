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
  conn = TssConnectionWrapper::get().getConnection();

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
    const TcrEndpoint& theEP, TcrConnection*& conn) {
  conn = TssConnectionWrapper::get().getSHConnection(theEP, m_dm->getName());
}

void ThinClientStickyManager::addStickyConnection(TcrConnection* conn) {
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);

  if (auto oldConn = TssConnectionWrapper::get().getConnection()) {
    const auto& it =
        m_stickyConnList.find(TssConnectionWrapper::get().getConnDoublePtr());
    if (it != m_stickyConnList.end()) {
      oldConn->setAndGetBeingUsed(false, false);
      m_stickyConnList.erase(it);
      TssConnectionWrapper::get().setConnection(nullptr, nullptr);
      m_dm->put(oldConn, false);
    }
  }

  if (conn) {
    TssConnectionWrapper::get().setConnection(conn, m_dm->shared_from_this());
    conn->setAndGetBeingUsed(true, true);  // this is done for transaction
                                           // thread when some one resume
                                           // transaction
    m_stickyConnList.insert(TssConnectionWrapper::get().getConnDoublePtr());
  }
}

void ThinClientStickyManager::setStickyConnection(TcrConnection* conn,
                                                  bool forTransaction) {
  if (!conn) {
    std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
    TssConnectionWrapper::get().setConnection(nullptr,
                                              m_dm->shared_from_this());
  } else {
    auto currentConn = TssConnectionWrapper::get().getConnection();
    if (currentConn != conn) {
      // otherwsie no need to set it again
      std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
      TssConnectionWrapper::get().setConnection(conn, m_dm->shared_from_this());
      // if transaction then it will keep this as used
      conn->setAndGetBeingUsed(false, forTransaction);
      m_stickyConnList.insert(TssConnectionWrapper::get().getConnDoublePtr());
    } else {
      // if transaction then it will keep this as used
      currentConn->setAndGetBeingUsed(false, forTransaction);
    }
  }
}

void ThinClientStickyManager::setSingleHopStickyConnection(
    const TcrEndpoint& ep, TcrConnection* conn) {
  TssConnectionWrapper::get().setSHConnection(ep, conn);
}

void ThinClientStickyManager::cleanStaleStickyConnection() {
  LOG_DEBUG("Cleaning sticky connections");
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
          LOG_DEBUG("Replaced a sticky connection");
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
  LOG_DEBUG("ThinClientStickyManager::closeAllStickyConnections()");
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
  for (const auto& tempConn : m_stickyConnList) {
    if (*tempConn) {
      (*tempConn)->close();
      _GEODE_SAFE_DELETE(*tempConn);
      m_dm->removeEPConnections(1, false);
    }
  }
}

bool ThinClientStickyManager::canThisConnBeDeleted(TcrConnection* conn) {
  bool canBeDeleted = false;
  LOG_DEBUG("ThinClientStickyManager::canThisConnBeDeleted()");
  std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
  if (m_dm->canItBeDeletedNoImpl(conn)) return true;
  auto endPt = conn->getEndpointObject();
  std::lock_guard<decltype(endPt->getQueueHostedMutex())> guardQueue(
      endPt->getQueueHostedMutex());
  if (endPt->isQueueHosted()) {
    for (const auto& it : m_stickyConnList) {
      auto connTemp = *it;
      if (connTemp && connTemp->getEndpointObject() == endPt) {
        canBeDeleted = true;
        break;
      }
    }
  }
  return canBeDeleted;
}

void ThinClientStickyManager::releaseThreadLocalConnection() {
  if (auto conn = TssConnectionWrapper::get().getConnection()) {
    std::lock_guard<decltype(m_stickyLock)> keysGuard(m_stickyLock);
    const auto& it =
        m_stickyConnList.find(TssConnectionWrapper::get().getConnDoublePtr());
    LOG_DEBUG("ThinClientStickyManager::releaseThreadLocalConnection()");
    if (it != m_stickyConnList.end()) {
      m_stickyConnList.erase(it);
      // now this can be used by next one
      conn->setAndGetBeingUsed(false, false);
      m_dm->put(conn, false);
    }
    TssConnectionWrapper::get().setConnection(nullptr,
                                              m_dm->shared_from_this());
  }
  TssConnectionWrapper::get().releaseSHConnections(*m_dm);
}

void ThinClientStickyManager::getAnyConnection(TcrConnection*& conn) {
  conn = TssConnectionWrapper::get().getAnyConnection(m_dm->getName());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
