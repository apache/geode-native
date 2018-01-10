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
using namespace apache::geode::client;
bool ThinClientStickyManager::getStickyConnection(
    TcrConnection*& conn, GfErrType* error,
    std::set<ServerLocation>& excludeServers, bool forTransaction) {
  bool maxConnLimit = false;
  bool connFound = false;
  // ACE_Guard<ACE_Recursive_Thread_Mutex> guard( m_stickyLock );
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
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
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
  // ACE_Guard<ACE_Recursive_Thread_Mutex> guard( m_stickyLock );
  if (!conn) {
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
    (*TssConnectionWrapper::s_geodeTSSConn)
        ->setConnection(nullptr, m_dm->shared_from_this());
  } else {
    TcrConnection* currentConn =
        (*TssConnectionWrapper::s_geodeTSSConn)->getConnection();
    if (currentConn != conn)  // otherwsie no need to set it again
    {
      ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
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
  std::set<ServerLocation> excludeServers;
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
  std::find_if(m_stickyConnList.begin(), m_stickyConnList.end(),
               ThinClientStickyManager::isNULL);
  while (1) {
    std::set<TcrConnection**>::iterator it =
        std::find_if(m_stickyConnList.begin(), m_stickyConnList.end(),
                     ThinClientStickyManager::isNULL);
    if (it == m_stickyConnList.end()) break;
    m_stickyConnList.erase(it);
  }
  bool maxConnLimit = false;
  for (std::set<TcrConnection**>::iterator it = m_stickyConnList.begin();
       it != m_stickyConnList.end(); it++) {
    TcrConnection** conn = (*it);
    if ((*conn)->setAndGetBeingUsed(true, false) &&
        canThisConnBeDeleted(*conn)) {
      GfErrType err = GF_NOERR;
      TcrConnection* temp = m_dm->getConnectionFromQueue(
          false, &err, excludeServers, maxConnLimit);
      if (temp) {
        TcrConnection* temp1 = *conn;
        //*conn = temp; instead of setting in thread local put in queue, thread
        // will come and pick it from there
        *conn = nullptr;
        m_dm->put(temp, false);
        temp1->close();
        _GEODE_SAFE_DELETE(temp1);
        m_dm->removeEPConnections(1, false);
        LOGDEBUG("Replaced a sticky connection");
      } else {
        (*conn)->setAndGetBeingUsed(false, false);
      }
      temp = nullptr;
    }
  }
}

void ThinClientStickyManager::closeAllStickyConnections() {
  LOGDEBUG("ThinClientStickyManager::closeAllStickyConnections()");
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
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
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
  if (m_dm->canItBeDeletedNoImpl(conn)) return true;
  TcrEndpoint* endPt = conn->getEndpointObject();
  ACE_Guard<ACE_Recursive_Thread_Mutex> guardQueue(
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
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_stickyLock);
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
bool ThinClientStickyManager::isNULL(TcrConnection** conn) {
  if (*conn == nullptr) return true;
  return false;
}

void ThinClientStickyManager::getAnyConnection(TcrConnection*& conn) {
  conn = (*TssConnectionWrapper::s_geodeTSSConn)
             ->getAnyConnection(m_dm->getName().c_str());
}
