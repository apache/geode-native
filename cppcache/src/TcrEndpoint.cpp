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

#include "TcrEndpoint.hpp"

#include <chrono>
#include <thread>

#include <geode/AuthInitialize.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "DistributedSystemImpl.hpp"
#include "StackTrace.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientRegion.hpp"
#include "Utils.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

const char* TcrEndpoint::NC_Notification = "NC Notification";

TcrEndpoint::TcrEndpoint(const std::string& name, CacheImpl* cacheImpl,
                         ACE_Semaphore& failoverSema,
                         ACE_Semaphore& cleanupSema,
                         ACE_Semaphore& redundancySema, ThinClientBaseDM* DM,
                         bool isMultiUserMode)
    : m_notifyConnection(nullptr),
      m_notifyReceiver(nullptr),
      m_cacheImpl(cacheImpl),
      m_maxConnections(cacheImpl->getDistributedSystem()
                           .getSystemProperties()
                           .connectionPoolSize()),
      m_numRegionListener(0),
      m_needToConnectInLock(false),
      m_isQueueHosted(false),
      m_uniqueId(0),
      m_failoverSema(failoverSema),
      m_cleanupSema(cleanupSema),
      m_redundancySema(redundancySema),
      m_baseDM(DM),
      m_name(name),
      m_notificationCleanupSema(0),
      m_numberOfTimesFailed(0),
      m_numRegions(0),
      m_pingTimeouts(0),
      m_notifyCount(0),
      m_dupCount(0),
      m_isAuthenticated(false),
      m_msgSent(false),
      m_pingSent(false),
      m_isMultiUserMode(isMultiUserMode),
      m_connected(false),
      m_isActiveEndpoint(false),
      m_serverQueueStatus(NON_REDUNDANT_SERVER),
      m_queueSize(0),
      m_distributedMemId(0),
      m_isServerQueueStatusSet(false) {
  /*
  m_name = Utils::convertHostToCanonicalForm(m_name.c_str() );
  */
}

TcrEndpoint::~TcrEndpoint() {
  m_connected = false;
  m_isActiveEndpoint = false;
  closeConnections();
  {
    // force close the notification channel -- see bug #295
    std::lock_guard<decltype(m_notifyReceiverLock)> guard(m_notifyReceiverLock);
    if (m_numRegionListener > 0) {
      LOGFINE(
          "Connection to %s still has references "
          "to subscription channel while closing",
          m_name.c_str());
      // fail in dev build to track #295 better in regressions
      m_numRegionListener = 0;
      closeNotification();
    }
  }
  while (m_notifyCount > 0) {
    LOGDEBUG("TcrEndpoint::~TcrEndpoint(): reducing notify count at %d",
             m_notifyCount);
    m_notificationCleanupSema.acquire();
    m_notifyCount--;
  }
  LOGFINE("Connection to %s deleted", m_name.c_str());
}

inline bool TcrEndpoint::needtoTakeConnectLock() {
#ifdef __linux
  if (m_cacheImpl->getDistributedSystem()
          .getSystemProperties()
          .connectWaitTimeout() > std::chrono::seconds::zero()) {
    return m_needToConnectInLock;  // once pipe or other socket error will take
                                   // lock to connect.
  }
  return false;  // once pipe or other socket error will take lock to connect.
#else
  return false;
#endif
}

GfErrType TcrEndpoint::createNewConnectionWL(
    TcrConnection*& newConn, bool isClientNotification, bool isSecondary,
    std::chrono::microseconds connectTimeout) {
  using clock = std::chrono::steady_clock;

  LOGFINE("TcrEndpoint::createNewConnectionWL");
  auto connectWaitTimeout = m_cacheImpl->getDistributedSystem()
                                .getSystemProperties()
                                .connectWaitTimeout();

  auto stopAt = clock::now() + connectWaitTimeout;
  auto connCreated = false;
  std::unique_lock<decltype(m_connectLock)> lock(m_connectLock,
                                                 std::defer_lock);

  while (clock::now() < stopAt) {
    auto locked = lock.try_lock_until(stopAt);

    LOGFINE("TcrEndpoint::createNewConnectionWL ret = %d interval = %ld",
            locked, connectWaitTimeout.count());

    if (locked) {
      try {
        LOGFINE("TcrEndpoint::createNewConnectionWL got lock");
        newConn = new TcrConnection(m_cacheImpl->tcrConnectionManager());
        newConn->initTcrConnection(shared_from_this(), m_ports,
                                   isClientNotification, isSecondary,
                                   connectTimeout);

        connCreated = true;             // to break while loop
        m_needToConnectInLock = false;  // no need to take lock
        LOGFINE("New Connection Created");
        break;
      } catch (const TimeoutException&) {
        LOGINFO("Timeout1 in handshake with endpoint[%s]", m_name.c_str());
        return GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA;
      } catch (std::exception& ex) {
        LOGWARN("Failed1 in handshake with endpoint[%s]: %s", m_name.c_str(),
                ex.what());
        return GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA;
      } catch (...) {
        LOGWARN("Unknown1 failure in handshake with endpoint[%s]",
                m_name.c_str());
        return GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA;
      }
    }
  }

  if (!connCreated) {
    LOGFINE("TcrEndpoint::createNewConnectionWL timeout");
    // throwException(TimeoutException("Thread is hanged in connect call"));
    return GF_CLIENT_WAIT_TIMEOUT;
  }

  return GF_NOERR;
}

GfErrType TcrEndpoint::createNewConnection(
    TcrConnection*& newConn, bool isClientNotification, bool isSecondary,
    std::chrono::microseconds connectTimeout, int32_t timeoutRetries,
    bool appThreadRequest) {
  LOGFINE(
      "TcrEndpoint::createNewConnection: connectTimeout =%s "
      "m_needToConnectInLock=%d appThreadRequest =%d",
      to_string(connectTimeout).c_str(), m_needToConnectInLock,
      appThreadRequest);
  GfErrType err = GF_NOERR;
  newConn = nullptr;
  while (timeoutRetries-- >= 0) {
    try {
      if (newConn == nullptr) {
        if (!needtoTakeConnectLock() || !appThreadRequest) {
          newConn = new TcrConnection(m_cacheImpl->tcrConnectionManager());
          bool authenticate = newConn->initTcrConnection(
              shared_from_this(), m_ports, isClientNotification, isSecondary,
              connectTimeout);
          if (authenticate) {
            authenticateEndpoint(newConn);
          }
        } else {
          err = createNewConnectionWL(newConn, isClientNotification,
                                      isSecondary, connectTimeout);
          if (err == GF_CLIENT_WAIT_TIMEOUT ||
              err == GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA) {
            break;
          }
        }
        // m_connected = true;
      }
      err = GF_NOERR;
      break;
    } catch (const TimeoutException&) {
      LOGINFO("Timeout in handshake with endpoint[%s]", m_name.c_str());
      err = GF_TIMEOUT;
      m_needToConnectInLock = true;  // while creating the connection
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } catch (const GeodeIOException& ex) {
      LOGINFO("IO error in handshake with endpoint[%s]: %s", m_name.c_str(),
              ex.what());
      err = GF_IOERR;
      m_needToConnectInLock = true;  // while creating the connection
      break;
    } catch (const AuthenticationFailedException& ex) {
      LOGWARN("Authentication failed in handshake with endpoint[%s]: %s",
              m_name.c_str(), ex.what());
      err = GF_AUTHENTICATION_FAILED_EXCEPTION;
      break;
    } catch (const AuthenticationRequiredException& ex) {
      LOGWARN("Authentication required in handshake with endpoint[%s]: %s",
              m_name.c_str(), ex.what());
      err = GF_AUTHENTICATION_REQUIRED_EXCEPTION;
      break;
    } catch (const CacheServerException& ex) {
      LOGWARN("Exception in handshake on server[%s]: %s", m_name.c_str(),
              ex.what());
      err = GF_CACHESERVER_EXCEPTION;
      break;
    } catch (const Exception& ex) {
      LOGWARN("Failed in handshake with endpoint[%s]: %s", m_name.c_str(),
              ex.what());
      err = GF_MSG;
      break;
    } catch (std::exception& ex) {
      LOGWARN("Failed in handshake with endpoint[%s]: %s", m_name.c_str(),
              ex.what());
      err = GF_MSG;
      break;
    } catch (...) {
      LOGWARN("Unknown failure in handshake with endpoint[%s]", m_name.c_str());
      err = GF_MSG;
      break;
    }
  }
  if (err != GF_NOERR && newConn != nullptr) {
    _GEODE_SAFE_DELETE(newConn);
  }
  return err;
}

void TcrEndpoint::authenticateEndpoint(TcrConnection*& conn) {
  LOGDEBUG(
      "TcrEndpoint::authenticateEndpoint m_isAuthenticated  = %d "
      "m_baseDM = %p, connection = %p",
      m_isAuthenticated, m_baseDM, conn);
  if (!m_isAuthenticated && m_baseDM) {
    setConnected();
    std::lock_guard<decltype(m_endpointAuthenticationLock)> guard(
        m_endpointAuthenticationLock);
    GfErrType err = GF_NOERR;
    auto creds = getCredentials();

    if (creds != nullptr) {
      LOGDEBUG("TcrEndpoint::authenticateEndpoint got creds from app = %zu",
               creds->getSize());
    } else {
      LOGDEBUG("TcrEndpoint::authenticateEndpoint no creds from app ");
    }

    TcrMessageUserCredential request(
        new DataOutput(m_cacheImpl->createDataOutput()), creds, m_baseDM);

    LOGDEBUG("request is created");
    TcrMessageReply reply(true, m_baseDM);
    err = sendRequestConnWithRetry(request, reply, conn);
    LOGDEBUG("TcrEndpoint::authenticateEndpoint - ERROR: %d", err);
    if (err == GF_NOERR) {
      LOGDEBUG(
          "TcrEndpoint::authenticateEndpoint - successfully authenticated on "
          "conn %p",
          conn);
      // put the object into local region
      switch (reply.getMessageType()) {
        case TcrMessage::RESPONSE: {
          // nothing to be done;
          break;
        }
        case TcrMessage::EXCEPTION: {
          err = ThinClientRegion::handleServerException("AuthException",
                                                        reply.getException());
          break;
        }
        default: {
          LOGERROR("Unknown message type %d while sending credentials",
                   reply.getMessageType());
          err = GF_MSG;
          break;
        }
      }
    }
    // throw exception if it is not authenticated
    throwExceptionIfError("TcrEndpoint::authenticateEndpoint", err);

    m_isAuthenticated = true;
  }
}
std::shared_ptr<Properties> TcrEndpoint::getCredentials() {
  const auto& distributedSystem = m_cacheImpl->getDistributedSystem();
  const auto& tmpSecurityProperties =
      distributedSystem.getSystemProperties().getSecurityProperties();

  if (const auto& authInitialize = m_cacheImpl->getAuthInitialize()) {
    LOGFINER(
        "Acquired handle to AuthInitialize plugin, "
        "getting credentials for %s",
        m_name.c_str());
    const auto& tmpAuthIniSecurityProperties =
        authInitialize->getCredentials(tmpSecurityProperties, m_name.c_str());
    LOGFINER("Done getting credentials");
    return tmpAuthIniSecurityProperties;
  }
  return nullptr;
}

ServerQueueStatus TcrEndpoint::getFreshServerQueueStatus(
    int32_t& queueSize, bool addToQueue, TcrConnection*& statusConn) {
  GfErrType err = GF_NOERR;
  TcrConnection* newConn;
  ServerQueueStatus status = NON_REDUNDANT_SERVER;

  err = createNewConnection(newConn, false, false,
                            m_cacheImpl->getDistributedSystem()
                                .getSystemProperties()
                                .connectTimeout());
  if (err == GF_NOERR) {
    status = newConn->getServerQueueStatus(queueSize);

    if (status == REDUNDANT_SERVER || status == PRIMARY_SERVER) {
      if (addToQueue) {
        m_opConnections.put(newConn, true);
      } else {
        statusConn = newConn;
      }
      m_connected = true;
      return status;
    } else {
      //  remove port from ports list (which is sent to server in notification
      //  handshake).
      closeConnection(newConn);
      return status;
    }
  }

  return status;
}

GfErrType TcrEndpoint::registerDM(bool clientNotification, bool isSecondary,
                                  bool isActiveEndpoint,
                                  ThinClientBaseDM* distMgr) {
  // Pre-conditions:
  // 1. If this is a secondary server then clientNotification must be true

  bool connected = false;
  GfErrType err = GF_NOERR;

  std::lock_guard<decltype(m_connectionLock)> connectionLockGuard(
      m_connectionLock);
  // Three cases here:
  // 1. m_connected is false, m_isActiveEndpoint is false and then
  //    if isActiveEndpoint is true, then create 'max' connections
  // 2. m_connected is false, m_isActiveEndpoint is false and then
  //    if isActiveEndpoint is false, then create just one connection
  //    to ping the server
  // 3. m_connected is true, m_isActiveEndpoint is false (i.e. server was
  //    previously not an active endpoint) then if isSecondary is false then
  //    create 'max-1' connections else do nothing
  m_opConnections.reset();
  if (m_maxConnections <= 0) {
    connected = true;
  } else if (!m_isActiveEndpoint) {
    int maxConnections = 0;
    if (isActiveEndpoint) {
      if (m_connected) {
        maxConnections = m_maxConnections - 1;
      } else {
        maxConnections = m_maxConnections;
      }
    } else if (!m_connected) {
      maxConnections = 1;
    }
    if (maxConnections > 0) {
      LOGINFO("Starting Handshake with %s%s",
              (isSecondary ? "secondary server "
                           : (isActiveEndpoint ? "" : "primary server ")),
              m_name.c_str());
      for (int connNum = 0; connNum < maxConnections; ++connNum) {
        TcrConnection* newConn;
        if ((err = createNewConnection(newConn, false, false,
                                       m_cacheImpl->getDistributedSystem()
                                           .getSystemProperties()
                                           .connectTimeout(),
                                       0, m_connected)) != GF_NOERR) {
          m_connected = false;
          m_isActiveEndpoint = false;
          closeConnections();
          return err;
        }
        m_opConnections.put(newConn, true);
      }
      LOGINFO("Handshake with %s%s success",
              (isSecondary ? "secondary server "
                           : (isActiveEndpoint ? "" : "primary server ")),
              m_name.c_str());
      m_connected = true;
      m_isActiveEndpoint = isActiveEndpoint;
    }
  }

  if (m_connected || connected) {
    if (clientNotification) {
      if (distMgr != nullptr) {
        std::lock_guard<decltype(m_distMgrsLock)> guardDistMgrs(m_distMgrsLock);
        m_distMgrs.push_back(distMgr);
      }
      LOGFINEST(
          "Registering subscription "
          "channel for endpoint %s",
          m_name.c_str());
      // setup notification channel for the first region
      std::lock_guard<decltype(m_notifyReceiverLock)> notifyReceiverLockGuard(
          m_notifyReceiverLock);
      if (m_numRegionListener == 0) {
        if ((err = createNewConnection(m_notifyConnection, true, isSecondary,
                                       m_cacheImpl->getDistributedSystem()
                                               .getSystemProperties()
                                               .connectTimeout() *
                                           3,
                                       0)) != GF_NOERR) {
          m_connected = false;
          m_isActiveEndpoint = false;
          closeConnections();
          LOGWARN("Failed to start subscription channel for endpoint %s",
                  m_name.c_str());
          return err;
        }
        m_notifyReceiver =
            std::unique_ptr<Task<TcrEndpoint>>(new Task<TcrEndpoint>(
                this, &TcrEndpoint::receiveNotification, NC_Notification));
        m_notifyReceiver->start();
      }
      ++m_numRegionListener;
      LOGFINEST("Incremented notification region count for endpoint %s to %d",
                m_name.c_str(), m_numRegionListener);
      m_connected = true;
    }
  }

  // Post-conditions:
  // 1. The endpoint should be marked as active, only if m_connected is true
  // 2. If this is not an active endpoint and it is connected then only one
  //    connection + notify channel
  return err;
}

void TcrEndpoint::unregisterDM(bool clientNotification,
                               ThinClientBaseDM* distMgr, bool) {
  if (clientNotification) {
    LOGFINEST(
        "Closing subscription "
        "channel for endpoint %s",
        m_name.c_str());
    // close notification channel if there is no region
    std::lock_guard<decltype(m_notifyReceiverLock)> guard(m_notifyReceiverLock);
    if (m_numRegionListener > 0 && --m_numRegionListener == 0) {
      closeNotification();
    }
    LOGFINEST("Decremented subscription region count for endpoint %s to %d",
              m_name.c_str(), m_numRegionListener);
    if (distMgr != nullptr) {
      std::lock_guard<decltype(m_distMgrsLock)> guardDistMgrs(m_distMgrsLock);
      m_distMgrs.remove(distMgr);
    }
    LOGFINEST("Done unsubscribe for endpoint %s", m_name.c_str());
  }
}

void TcrEndpoint::pingServer(ThinClientPoolDM* poolDM) {
  LOGDEBUG("Sending ping message to endpoint %s", m_name.c_str());
  if (!m_connected) {
    LOGFINER("Skipping ping task for disconnected endpoint %s", m_name.c_str());
    return;
  }

  if (!m_msgSent && !m_pingSent) {
    TcrMessagePing* pingMsg = TcrMessage::getPingMessage(m_cacheImpl);
    TcrMessageReply reply(true, nullptr);
    LOGFINEST("Sending ping message to endpoint %s", m_name.c_str());
    GfErrType error;
    if (poolDM != nullptr) {
      error = poolDM->sendRequestToEP(*pingMsg, reply, this);
    } else {
      error = send(*pingMsg, reply);
    }
    LOGFINEST("Sent ping message to endpoint %s with error code %d%s",
              m_name.c_str(), error, error == GF_NOERR ? " (no error)" : "");
    if (error == GF_NOERR) {
      m_pingSent = true;
    }
    if (error == GF_TIMEOUT && m_pingTimeouts < 2) {
      ++m_pingTimeouts;
    } else {
      m_pingTimeouts = 0;
      //  Only call setConnectionStatus if the status has changed (non thread
      //  safe check)
      // This is to avoid blocking the ping thread if notification channel takes
      // a long time to
      // complete causing the server to drop the client in the midst of
      // connection establishment.
      bool connected = (error == GF_NOERR)
                           ? (reply.getMessageType() == TcrMessage::REPLY)
                           : false;
      if (m_connected != connected) {
        setConnectionStatus(connected);
      }
    }
    LOGFINEST("Completed sending ping message to endpoint %s", m_name.c_str());
  } else {
    m_msgSent = false;
    m_pingSent = false;
  }
}

bool TcrEndpoint::checkDupAndAdd(std::shared_ptr<EventId> eventid) {
  return m_cacheImpl->tcrConnectionManager().checkDupAndAdd(eventid);
}

void TcrEndpoint::receiveNotification(std::atomic<bool>& isRunning) {
  LOGFINE("Started subscription channel for endpoint %s", m_name.c_str());
  while (isRunning) {
    TcrMessageReply* msg = nullptr;
    try {
      size_t dataLen;
      ConnErrType opErr = CONN_NOERR;
      auto data = m_notifyConnection->receive(&dataLen, &opErr,
                                              std::chrono::seconds(5));

      if (opErr == CONN_IOERR) {
        // Endpoint is disconnected, this exception is expected
        LOGFINER(
            "IO exception while receiving subscription event for endpoint %d",
            opErr);
        if (isRunning) {
          setConnectionStatus(false);
          // close notification channel
          std::lock_guard<decltype(m_notifyReceiverLock)> guard(
              m_notifyReceiverLock);
          if (m_numRegionListener > 0) {
            m_numRegionListener = 0;
            closeNotification();
          }
        }
        break;
      }

      if (data) {
        msg = new TcrMessageReply(true, m_baseDM);
        msg->initCqMap();
        msg->setData(data, static_cast<int32_t>(dataLen),
                     getDistributedMemberID(),
                     *(m_cacheImpl->getSerializationRegistry()),
                     *(m_cacheImpl->getMemberListForVersionStamp()));
        handleNotificationStats(static_cast<int64_t>(dataLen));
        LOGDEBUG("receive notification %d", msg->getMessageType());

        if (!isRunning) {
          _GEODE_SAFE_DELETE(msg);
          break;
        }

        if (msg->getMessageType() == TcrMessage::SERVER_TO_CLIENT_PING) {
          LOGFINE("Received ping from server subscription channel.");
        }

        // ignore some message types like REGISTER_INSTANTIATORS
        if (msg->shouldIgnore()) {
          _GEODE_SAFE_DELETE(msg);
          continue;
        }

        bool isMarker = (msg->getMessageType() == TcrMessage::CLIENT_MARKER);
        if (!msg->hasCqPart()) {
          if (msg->getMessageType() != TcrMessage::CLIENT_MARKER) {
            const std::string& regionFullPath1 = msg->getRegionName();
            auto region1 = m_cacheImpl->getRegion(regionFullPath1);

            if (region1 != nullptr &&
                !static_cast<ThinClientRegion*>(region1.get())
                     ->getDistMgr()
                     ->isEndpointAttached(this)) {
              // drop event before even processing the eventid for duplicate
              // checking
              LOGFINER("Endpoint %s dropping event for region %s",
                       m_name.c_str(), regionFullPath1.c_str());
              _GEODE_SAFE_DELETE(msg);
              continue;
            }
          }
        }

        if (!checkDupAndAdd(msg->getEventId())) {
          m_dupCount++;
          if (m_dupCount % 100 == 1) {
            LOGFINE("Dropped %dst duplicate notification message", m_dupCount);
          }
          _GEODE_SAFE_DELETE(msg);
          continue;
        }

        if (isMarker) {
          LOGFINE("Got a marker message on endpont %s", m_name.c_str());
          m_cacheImpl->processMarker();
          processMarker();
          _GEODE_SAFE_DELETE(msg);
        } else {
          if (!msg->hasCqPart())  // || msg->isInterestListPassed())
          {
            const std::string& regionFullPath = msg->getRegionName();
            auto region = m_cacheImpl->getRegion(regionFullPath);

            if (region != nullptr) {
              static_cast<ThinClientRegion*>(region.get())
                  ->receiveNotification(msg);
            } else {
              LOGWARN(
                  "Notification for region %s that does not exist in "
                  "client cacheImpl.",
                  regionFullPath.c_str());
            }
          } else {
            LOGDEBUG("receive cq notification %d", msg->getMessageType());
            auto queryService = getQueryService();
            if (queryService != nullptr) {
              static_cast<RemoteQueryService*>(queryService.get())
                  ->receiveNotification(msg);
            }
          }
        }
      }
    } catch (const TimeoutException&) {
      // If there is no notification, this exception is expected
      // But this is valid only when *no* data has been received
      // otherwise if data has been read then TcrConnection will throw
      // a GeodeIOException which will cause the channel to close.
      LOGDEBUG(
          "receiveNotification timed out: no data received from "
          "endpoint %s",
          m_name.c_str());
    } catch (const GeodeIOException& e) {
      // Endpoint is disconnected, this exception is expected
      LOGFINER(
          "IO exception while receiving subscription event for endpoint %s: %s",
          m_name.c_str(), e.what());
      if (m_connected) {
        setConnectionStatus(false);
        // close notification channel
        std::lock_guard<decltype(m_notifyReceiverLock)> guard(
            m_notifyReceiverLock);
        if (m_numRegionListener > 0) {
          m_numRegionListener = 0;
          closeNotification();
        }
      }
      break;
    } catch (const Exception& ex) {
      _GEODE_SAFE_DELETE(msg);
      LOGERROR(
          "Exception while receiving subscription event for endpoint %s:: %s: "
          "%s",
          m_name.c_str(), ex.getName().c_str(), ex.what());
    } catch (...) {
      _GEODE_SAFE_DELETE(msg);
      LOGERROR(
          "Unexpected exception while "
          "receiving subscription event from endpoint %s",
          m_name.c_str());
    }
  }
  LOGFINE("Ended subscription channel for endpoint %s", m_name.c_str());
}

inline bool TcrEndpoint::compareTransactionIds(int32_t reqTransId,
                                               int32_t replyTransId,
                                               std::string& failReason,
                                               TcrConnection* conn) {
  LOGDEBUG("TcrEndpoint::compareTransactionIds requested id = %d ,replied = %d",
           reqTransId, replyTransId);
  if (replyTransId != reqTransId) {
    LOGERROR(
        "Transaction ids do not match on endpoint %s for "
        "send operation: %d, %d. Possible serialization mismatch",
        m_name.c_str(), reqTransId, replyTransId);
    closeConnection(conn);
    failReason = "mismatch of transaction IDs in operation";
    return false;
  }
  return true;
}

inline bool TcrEndpoint::handleIOException(const std::string& message,
                                           TcrConnection*& conn, bool) {
  int32_t lastError = ACE_OS::last_error();
  if (lastError == ECONNRESET || lastError == EPIPE) {
    _GEODE_SAFE_DELETE(conn);
  } else {
    closeConnection(conn);
  }
  LOGFINE(
      "IO error during send for endpoint %s "
      "[errno: %d: %s]: %s",
      m_name.c_str(), lastError, ACE_OS::strerror(lastError), message.c_str());
  // EAGAIN =11, EWOULDBLOCK = 10035L, EPIPE = 32, ECONNRESET =10054L(An
  // existing connection was forcibly closed by the remote host.)
  if (!(lastError == EAGAIN || lastError == EWOULDBLOCK /*||
        lastError == ECONNRESET */
        /*|| lastError == EPIPE*/)) {
    // break from enclosing loop without retries
    // something wrong try connect in lock
    m_needToConnectInLock = true;
    return false;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  return true;
}

GfErrType TcrEndpoint::sendRequestConn(const TcrMessage& request,
                                       TcrMessageReply& reply,
                                       TcrConnection* conn,
                                       std::string& failReason) {
  int32_t type = request.getMessageType();
  GfErrType error = GF_NOERR;

  LOGFINER("Sending request type %d to endpoint [%s] via connection [%p]", type,
           m_name.c_str(), conn);
  // TcrMessage * req = const_cast<TcrMessage *>(&request);
  LOGDEBUG("TcrEndpoint::sendRequestConn  = %p", m_baseDM);
  if (m_baseDM != nullptr) m_baseDM->beforeSendingRequest(request, conn);
  if (((type == TcrMessage::EXECUTE_FUNCTION ||
        type == TcrMessage::EXECUTE_REGION_FUNCTION) &&
       (request.hasResult() & 2))) {
    conn->sendRequestForChunkedResponse(request, request.getMsgLength(), reply,
                                        request.getTimeout(),
                                        reply.getTimeout());
  } else if (type == TcrMessage::REGISTER_INTEREST_LIST ||
             type == TcrMessage::REGISTER_INTEREST ||
             type == TcrMessage::QUERY ||
             type == TcrMessage::QUERY_WITH_PARAMETERS ||
             type == TcrMessage::GET_ALL_70 ||
             type == TcrMessage::GET_ALL_WITH_CALLBACK ||
             type == TcrMessage::PUTALL ||
             type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
             type == TcrMessage::REMOVE_ALL ||
             ((type == TcrMessage::EXECUTE_FUNCTION ||
               type == TcrMessage::EXECUTE_REGION_FUNCTION) &&
              (request.hasResult() & 2)) ||
             type ==
                 TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||  // This is
                                                                    // kept
                                                                    // aside as
                                                                    // server
                                                                    // always
                                                                    // sends
                                                                    // chunked
                                                                    // response.
             type == TcrMessage::EXECUTECQ_MSG_TYPE ||
             type == TcrMessage::STOPCQ_MSG_TYPE ||
             type == TcrMessage::CLOSECQ_MSG_TYPE ||
             type == TcrMessage::KEY_SET ||
             type == TcrMessage::CLOSECLIENTCQS_MSG_TYPE ||
             type == TcrMessage::GETCQSTATS_MSG_TYPE ||
             type == TcrMessage::MONITORCQ_MSG_TYPE ||
             type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
             type == TcrMessage::GETDURABLECQS_MSG_TYPE) {
    conn->sendRequestForChunkedResponse(request, request.getMsgLength(), reply,
                                        request.getTimeout(),
                                        reply.getTimeout());
    LOGDEBUG("sendRequestConn: calling sendRequestForChunkedResponse DONE");
  } else {
    // Chk request type to request if so request.getCallBackArg flag & setCall
    // back arg flag to true, and in response chk for this flag.
    if (request.getMessageType() == TcrMessage::REQUEST) {
      if (request.isCallBackArguement()) {
        reply.setCallBackArguement(true);
      }
    }
    size_t dataLen;
    auto data = conn->sendRequest(request.getMsgData(), request.getMsgLength(),
                                  &dataLen, request.getTimeout(),
                                  reply.getTimeout(), request.getMessageType());
    reply.setMessageTypeRequest(type);
    reply.setData(
        data, static_cast<int32_t>(dataLen), getDistributedMemberID(),
        *(m_cacheImpl->getSerializationRegistry()),
        *(m_cacheImpl
              ->getMemberListForVersionStamp()));  // memory is released by
                                                   // TcrMessage setData().
  }

  // reset idle timeout of the connection for pool connection manager
  if (type != TcrMessage::PING) {
    conn->touch();
  }

  if (reply.getMessageType() == TcrMessage::INVALID) {
    if (type == TcrMessage::EXECUTE_FUNCTION ||
        type == TcrMessage::EXECUTE_REGION_FUNCTION ||
        type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP) {
      ChunkedFunctionExecutionResponse* resultCollector =
          dynamic_cast<ChunkedFunctionExecutionResponse*>(
              reply.getChunkedResultHandler());
      if (resultCollector->getResult() == false) {
        LOGDEBUG("TcrEndpoint::send: function execution, no response desired");
        //            m_opConnections.put( conn, false );
        //  return GF_NOERR;
        error = GF_NOERR;
      }
    } else {
      // Treat INVALID messages like IO exceptions
      error = GF_IOERR;
    }
  }
  // do we need to consider case where compareTransactionIds return true?
  // I think we will not have issue here
  else if (!compareTransactionIds(request.getTransId(), reply.getTransId(),
                                  failReason, conn)) {
    error = GF_NOTCON;
  }
  if (error == GF_NOERR) {
    if (m_baseDM) {
      m_baseDM->afterSendingRequest(request, reply, conn);
    }
  }

  return error;
}

bool TcrEndpoint::isMultiUserMode() {
  LOGDEBUG("TcrEndpoint::isMultiUserMode %d", m_isMultiUserMode);
  return m_isMultiUserMode;
}

GfErrType TcrEndpoint::sendRequestWithRetry(
    const TcrMessage& request, TcrMessageReply& reply, TcrConnection*& conn,
    bool& epFailure, std::string& failReason, int maxSendRetries,
    bool useEPPool, std::chrono::microseconds requestedTimeout,
    bool isBgThread) {
  GfErrType error = GF_NOTCON;
  bool createNewConn = false;
  // int32_t type = request.getMessageType();
  int sendRetryCount = 0;

  //  Retry on the following send errors:
  // Timeout: 1 retry
  // EAGAIN, ECONNRESET, EWOULDBLOCK: 1 retry
  // Connection pool is empty (too many threads or no connections available): 1
  // retry

  do {
    if (sendRetryCount > 0) {
      // this is a retry. set the retry bit in the early Ack
      (const_cast<TcrMessage&>(request)).updateHeaderForRetry();
    }

    auto timeout = requestedTimeout;
    epFailure = false;
    if (useEPPool) {
      if (m_maxConnections == 0) {
        std::lock_guard<decltype(m_connectionLock)> guard(m_connectionLock);
        if (m_maxConnections == 0) {
          LOGFINE(
              "Creating a new connection when connection-pool-size system "
              "property set to 0");
          if ((error = createNewConnection(conn, false, false,
                                           m_cacheImpl->getDistributedSystem()
                                               .getSystemProperties()
                                               .connectTimeout())) !=
              GF_NOERR) {
            epFailure = true;
            continue;
          }
          m_maxConnections = 1;
        }
      }
    }
    LOGDEBUG("TcrEndpoint::send() getting a connection for endpoint %s",
             m_name.c_str());
    if (createNewConn) {
      createNewConn = false;
      if (!m_connected) {
        return GF_NOTCON;
      } else if ((error =
                      createNewConnection(conn, false, false,
                                          m_cacheImpl->getDistributedSystem()
                                              .getSystemProperties()
                                              .connectTimeout(),
                                          0, true)) != GF_NOERR) {
        epFailure = true;
        continue;
      }
    } else if (conn == nullptr && useEPPool) {
      LOGFINER(
          "sendRequestWithRetry:: looking for connection in queue timeout = "
          "%s",
          to_string(timeout).c_str());
      // max wait time to get a connection
      conn = m_opConnections.getUntil(timeout);
    }
    if (!m_connected) {
      return GF_NOTCON;
    }
    if (conn != nullptr) {
      LOGDEBUG("TcrEndpoint::send() obtained a connection for endpoint %s",
               m_name.c_str());
      int reqTransId = request.getTransId();

      try {
        LOGDEBUG("Calling sendRequestConn");
        error = sendRequestConn(request, reply, conn, failReason);
        if (error == GF_IOERR) {
          epFailure = true;
          failReason = "received INVALID reply from server";
          if (!handleIOException(failReason, conn, isBgThread)) {
            break;
          }
          createNewConn = true;
        } else if (error == GF_NOTCON) {
          epFailure = true;
          createNewConn = true;
        } else {
          if (useEPPool) {
            m_opConnections.put(conn, false);
          }
          return GF_NOERR;
        }
      } catch (const TimeoutException&) {
        error = GF_TIMEOUT;
        LOGFINE(
            "Send timed out for endpoint %s. "
            "Message txid = %d",
            m_name.c_str(), reqTransId);
        closeFailedConnection(conn);
        /*
        if ( !(m_poolHADM && m_poolHADM->getThreadLocalConnections()) ){ //close
        connection only when not a sticky connection.
          closeConnection( conn );
        }*/
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        int32_t type = request.getMessageType();
        epFailure = (type != TcrMessage::QUERY && type != TcrMessage::PUTALL &&
                     type != TcrMessage::PUT_ALL_WITH_CALLBACK &&
                     type != TcrMessage::EXECUTE_FUNCTION &&
                     type != TcrMessage::EXECUTE_REGION_FUNCTION &&
                     type != TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP &&
                     type != TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE);

        // epFailure = true;
        failReason = "timed out waiting for endpoint";
        createNewConn = true;
      } catch (const GeodeIOException& ex) {
        error = GF_IOERR;
        epFailure = true;
        failReason = "IO error for endpoint";
        if (!handleIOException(ex.what(), conn,
                               isBgThread)) {  // change here
          break;
        }
        createNewConn = true;
      } catch (const Exception& ex) {
        failReason = ex.getName();
        failReason.append(": ");
        failReason.append(ex.what());
        LOGWARN("Error during send for endpoint %s due to %s", m_name.c_str(),
                failReason.c_str());
        if (compareTransactionIds(reqTransId, reply.getTransId(), failReason,
                                  conn)) {
          if (Log::warningEnabled()) {
            LOGWARN("Stack trace: %s", ex.getStackTrace().c_str());
          }
          error = GF_MSG;
          if (useEPPool) {
            m_opConnections.put(conn, false);
          } else {
            // we are here its better to close the connection as
            // "compareTransactionIds"
            // will not close the connection
            closeConnection(conn);
          }
          break;
        } else {
          error = GF_NOTCON;
          epFailure = true;
          createNewConn = true;
        }
      } catch (...) {
        failReason = "unexpected exception";
        LOGERROR(
            "Unexpected exception while sending request to "
            "endpoint %s",
            m_name.c_str());
        if (compareTransactionIds(reqTransId, reply.getTransId(), failReason,
                                  conn)) {
          error = GF_MSG;
          if (useEPPool) {
            m_opConnections.put(conn, false);
          } else {
            // we are here its better to close the connection as
            // "compareTransactionIds"
            // will not close the connection
            closeConnection(conn);
          }
          break;
        } else {
          error = GF_NOTCON;
          epFailure = true;
          createNewConn = true;
        }
      }
    } else {
      if (useEPPool) {
        epFailure = true;
        failReason = "server connection could not be obtained";
        if (timeout <= std::chrono::microseconds::zero()) {
          error = GF_TIMEOUT;
          LOGWARN(
              "No connection available for %ld seconds "
              "for endpoint %s.",
              requestedTimeout.count(), m_name.c_str());
        } else {
          error = GF_NOTCON;
          LOGFINE(
              "Returning without connection with %s seconds remaining "
              "for endpoint %s.",
              std::to_string(timeout.count()).c_str(), m_name.c_str());
        }
      } else {
        LOGERROR("Unexpected failure while sending request to server.");
      }
    }
  } while (++sendRetryCount <= maxSendRetries);
  return error;
}  // namespace client

void TcrEndpoint::setRetry(const TcrMessage& request, int& maxSendRetries) {
  int32_t type = request.getMessageType();
  if (type == TcrMessage::QUERY || type == TcrMessage::QUERY_WITH_PARAMETERS ||
      type == TcrMessage::PUTALL || type == TcrMessage::PUT_ALL_WITH_CALLBACK ||
      type == TcrMessage::EXECUTE_FUNCTION ||
      type == TcrMessage::EXECUTE_REGION_FUNCTION ||
      type == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP ||
      type == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE) {
    maxSendRetries = 0;
  }
}

GfErrType TcrEndpoint::send(const TcrMessage& request, TcrMessageReply& reply) {
  GfErrType error = GF_NOTCON;

  int maxSendRetries = 1;
  setRetry(request, maxSendRetries);

  TcrConnection* conn = nullptr;
  bool epFailure;
  std::string failReason;
  //  TODO: remove sendRetryCount as parameter.
  error = sendRequestWithRetry(request, reply, conn, epFailure, failReason,
                               maxSendRetries, true, reply.getTimeout());

  if (error == GF_NOERR) {
    m_msgSent = true;
  }

  if (error != GF_NOERR && epFailure) {
    LOGFINE("Send Giving up for endpoint %s; reason: %s.", m_name.c_str(),
            failReason.c_str());
    setConnectionStatus(false);
  }

  return error;
}

GfErrType TcrEndpoint::sendRequestConnWithRetry(const TcrMessage& request,
                                                TcrMessageReply& reply,
                                                TcrConnection*& conn,
                                                bool isBgThread) {
  GfErrType error = GF_NOTCON;

  int maxSendRetries = 1;
  setRetry(request, maxSendRetries);

  //  Retry on the following send errors:
  // Timeout: 1 retry
  // EAGAIN, ECONNRESET, EWOULDBLOCK: 1 retry
  // Connection pool is empty (too many threads or no connections available): 1
  // retry
  bool epFailure;
  std::string failReason;
  LOGFINE("sendRequestConnWithRetry:: maxSendRetries = %d ", maxSendRetries);
  error = sendRequestWithRetry(request, reply, conn, epFailure, failReason,
                               maxSendRetries, false, reply.getTimeout(),
                               isBgThread);
  if (error == GF_NOERR) {
    m_msgSent = true;
  }

  if (error != GF_NOERR && epFailure) {
    LOGFINE("sendRequestConnWithRetry: Giving up for endpoint %s; reason: %s.",
            m_name.c_str(), failReason.c_str());
    setConnectionStatus(false);
  }

  return error;
}

void TcrEndpoint::setConnectionStatus(bool status) {
  // : Store the original value of m_isActiveEndpoint.
  // This is to try make failover more resilient for the case when
  // a foreground operation thread is connecting to an endpoint while
  // the notification thread is disconnecting from the same, or vice versa.
  // By comparing the original value with the new value we know if
  // someone else has changed the status in that duration, and skip
  // the change if that is the case.
  // Same logic applies for the ping thread.
  // Try something like (after the 2.5 patch release):
  // bool wasActive = m_isActiveEndpoint;
  // Then after taking the lock:
  // If ( !wasActive && isActiveEndpoint ) { return; }
  std::lock_guard<decltype(m_connectionLock)> guard(m_connectionLock);
  if (m_connected != status) {
    bool connected = m_connected;
    m_connected = status;
    if (connected) {
      m_numberOfTimesFailed += 1;
      m_isAuthenticated = false;
      // disconnected
      LOGFINE("Disconnecting from endpoint %s", m_name.c_str());
      closeConnections();
      m_isActiveEndpoint = false;
      LOGFINE("Disconnected from endpoint %s", m_name.c_str());
      triggerRedundancyThread();
    }
  }
}

void TcrEndpoint::triggerRedundancyThread() {
  m_failoverSema.release();
  m_redundancySema.release();
}

void TcrEndpoint::closeConnection(TcrConnection*& conn) {
  conn->close();
  m_ports.erase(conn->getPort());
  try {
    _GEODE_SAFE_DELETE(conn);
  } catch (...) {
  }
}

void TcrEndpoint::closeConnections() {
  m_opConnections.close();
  m_ports.clear();

  m_cacheImpl->doIfDestroyNotPending([&]() {
    if (!m_cacheImpl->isClosed()) {
      m_maxConnections = m_cacheImpl->getDistributedSystem()
                             .getSystemProperties()
                             .connectionPoolSize();
    }
  });
}

/*
void TcrEndpoint::sendNotificationCloseMsg()
{
  if (m_notifyConnection != nullptr) {
    m_notifyReceiver->stop();
    m_notifyConnection->close();
  }
}
*/

void TcrEndpoint::closeNotification() {
  LOGFINEST("Closing subscription channel for endpoint %s", m_name.c_str());
  m_notifyConnection->close();
  m_notifyReceiver->stopNoblock();
  TcrConnectionManager& tccm = m_cacheImpl->tcrConnectionManager();
  tccm.addNotificationForDeletion(m_notifyReceiver.get(), m_notifyConnection,
                                  m_notificationCleanupSema);
  m_notifyCount++;
  m_cleanupSema.release();
  m_isQueueHosted = false;
  LOGFINEST(
      "Added susbcription channel for deletion and "
      "released cleanup semaphore for endpoint %s",
      m_name.c_str());
}

void TcrEndpoint::stopNoBlock() {
  if (m_notifyReceiver != nullptr) {
    m_notifyConnection->close();
    m_notifyReceiver->stopNoblock();
  }
}

void TcrEndpoint::stopNotifyReceiverAndCleanup() {
  LOGFINER("Stopping subscription receiver and cleaning up");
  std::lock_guard<decltype(m_notifyReceiverLock)> guard(m_notifyReceiverLock);

  if (m_notifyReceiver != nullptr) {
    LOGFINER("Waiting for notification thread...");
    // m_notifyReceiver->stopNoblock();
    m_notifyReceiver->wait();
    bool found = false;
    for (const auto& it : m_notifyReceiverList) {
      if (it == m_notifyReceiver.get()) {
        found = true;
        break;
      }
    }

    if (!found) {
      m_notifyReceiver = nullptr;
      _GEODE_SAFE_DELETE(m_notifyConnection);
    }
  }

  m_numRegionListener = 0;

  if (!m_notifyConnectionList.empty()) {
    LOGFINER("TcrEndpoint::stopNotifyReceiverAndCleanup: notifylist size = %zu",
             m_notifyConnectionList.size());
    for (auto& it : m_notifyConnectionList) {
      LOGFINER(
          "TcrEndpoint::stopNotifyReceiverAndCleanup: deleting old notify "
          "connections.");
      _GEODE_SAFE_DELETE(it);
    }
  }
}

void TcrEndpoint::setServerQueueStatus(ServerQueueStatus queueStatus,
                                       int32_t queueSize) {
  if (!m_isServerQueueStatusSet) {
    m_isServerQueueStatusSet = true;
    m_serverQueueStatus = queueStatus;
    m_queueSize = queueSize;
  }
}

bool TcrEndpoint::isQueueHosted() { return m_isQueueHosted; }

void TcrEndpoint::processMarker() {
  m_cacheImpl->tcrConnectionManager().processMarker();
}

std::shared_ptr<QueryService> TcrEndpoint::getQueryService() {
  return m_cacheImpl->getQueryService(true);
}

void TcrEndpoint::closeFailedConnection(TcrConnection*& conn) {
  closeConnection(conn);
}

void TcrEndpoint::handleNotificationStats(int64_t) {}

}  // namespace client
}  // namespace geode
}  // namespace apache
