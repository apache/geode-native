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

#include <memory.h>

#include <ace/INET_Addr.h>
#include <ace/OS.h>

#include <geode/DistributedSystem.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/AuthInitialize.hpp>

#include "TcrConnection.hpp"
#include "Connector.hpp"
#include "TcpSslConn.hpp"
#include "ClientProxyMembershipID.hpp"
#include "ThinClientPoolHADM.hpp"
#include "TcrEndpoint.hpp"
#include "GeodeTypeIdsImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "DistributedSystemImpl.hpp"
#include "Version.hpp"
#include "DiffieHellman.hpp"
#include "Utils.hpp"
#include "ThinClientRegion.hpp"

namespace apache {
namespace geode {
namespace client {

const int HEADER_LENGTH = 17;
const int64_t INITIAL_CONNECTION_ID = 26739;

#define throwException(ex)                            \
  {                                                   \
    LOGFINEST(ex.getName() + ": " + ex.getMessage()); \
    throw ex;                                         \
  }
bool TcrConnection::InitTcrConnection(
    TcrEndpoint* endpointObj, const char* endpoint, Set<uint16_t>& ports,
    bool isClientNotification, bool isSecondary,
    std::chrono::microseconds connectTimeout) {
  m_conn = nullptr;
  m_endpointObj = endpointObj;
  m_poolDM = dynamic_cast<ThinClientPoolDM*>(m_endpointObj->getPoolHADM());
  // add to the connection reference counter of the endpoint
  m_endpointObj->addConnRefCounter(1);
  // m_connected = isConnected;
  m_hasServerQueue = NON_REDUNDANT_SERVER;
  m_queueSize = 0;
  m_dh = nullptr;
  // m_chunksProcessSema = 0;
  m_creationTime = ACE_OS::gettimeofday();
  connectionId = INITIAL_CONNECTION_ID;
  m_lastAccessed = ACE_OS::gettimeofday();
  auto cacheImpl = m_poolDM->getConnectionManager().getCacheImpl();
  const auto& distributedSystem = cacheImpl->getDistributedSystem();
  const auto& sysProp = distributedSystem.getSystemProperties();

  LOGDEBUG(
      "Tcrconnection const isSecondary = %d and isClientNotification = %d, "
      "this = %p,  conn ref to endopint %d",
      isSecondary, isClientNotification, this,
      m_endpointObj->getConnRefCounter());
  bool isPool = false;
  m_isBeingUsed = false;
  GF_DEV_ASSERT(endpoint != nullptr);
  m_endpoint = endpoint;
  // Precondition:
  // 1. isSecondary ==> isClientNotification

  GF_DEV_ASSERT(!isSecondary || isClientNotification);

  // Create TcpConn object which manages a socket connection with the endpoint.
  if (endpointObj && endpointObj->getPoolHADM()) {
    m_conn = createConnection(
        m_endpoint, connectTimeout,
        static_cast<int32_t>(
            endpointObj->getPoolHADM()->getSocketBufferSize()));
    isPool = true;
  } else {
    m_conn = createConnection(m_endpoint, connectTimeout,
                              sysProp.maxSocketBufferSize());
  }

  GF_DEV_ASSERT(m_conn != nullptr);

  auto handShakeMsg = cacheImpl->createDataOutput();
  bool isNotificationChannel = false;
  // Send byte Acceptor.CLIENT_TO_SERVER = (byte) 100;
  // Send byte Acceptor.SERVER_TO_CLIENT = (byte) 101;
  if (isClientNotification) {
    isNotificationChannel = true;
    if (isSecondary) {
      handShakeMsg->write(static_cast<int8_t>(SECONDARY_SERVER_TO_CLIENT));
    } else {
      handShakeMsg->write(static_cast<int8_t>(PRIMARY_SERVER_TO_CLIENT));
    }
  } else {
    handShakeMsg->write(static_cast<int8_t>(CLIENT_TO_SERVER));
  }

  // added for versioned client
  int8_t versionOrdinal = Version::getOrdinal();
  handShakeMsg->write(versionOrdinal);

  LOGFINE("Client version ordinal is %d", versionOrdinal);

  handShakeMsg->write(static_cast<int8_t>(REPLY_OK));

  // Send byte REPLY_OK = (byte)58;
  if (!isClientNotification) {
    m_port = m_conn->getPort();
    ports.insert(m_port);
  } else {
    // add the local ports to message
    Set<uint16_t>::Iterator iter = ports.iterator();
    handShakeMsg->writeInt(static_cast<int32_t>(ports.size()));
    while (iter.hasNext()) {
      handShakeMsg->writeInt(static_cast<int32_t>(iter.next()));
    }
  }

  //  Writing handshake readtimeout value for CSVER_51+.
  if (!isClientNotification) {
    // SW: The timeout has been artificially raised to the highest
    // permissible value for bug #232 for now.
    //  minus 10 sec because the GFE 5.7 gridDev branch adds a
    // 5 sec buffer which was causing an int overflow.
    handShakeMsg->writeInt((int32_t)0x7fffffff - 10000);
  }

  // Write header for byte FixedID since GFE 5.7
  handShakeMsg->write(static_cast<int8_t>(GeodeTypeIdsImpl::FixedIDByte));
  // Writing byte for ClientProxyMembershipID class id=38 as registered on the
  // java server.
  handShakeMsg->write(
      static_cast<int8_t>(GeodeTypeIdsImpl::ClientProxyMembershipId));
  if (endpointObj->getPoolHADM()) {
    ClientProxyMembershipID* memId =
        endpointObj->getPoolHADM()->getMembershipId();
    uint32_t memIdBufferLength;
    const char* memIdBuffer = memId->getDSMemberId(memIdBufferLength);
    handShakeMsg->writeBytes((int8_t*)memIdBuffer, memIdBufferLength);
  } else {
    ACE_TCHAR hostName[256];
    ACE_OS::hostname(hostName, sizeof(hostName) - 1);

    ACE_INET_Addr driver(hostName);
    uint32_t hostAddr = driver.get_ip_address();
    uint16_t hostPort = 0;

    // Add 3 durable Subcription properties to ClientProxyMembershipID

    auto&& durableId = sysProp.durableClientId();
    auto&& durableTimeOut = sysProp.durableTimeout();

    // Write ClientProxyMembershipID serialized object.
    uint32_t memIdBufferLength;
    const auto memId = cacheImpl->getClientProxyMembershipIDFactory().create(
        hostName, hostAddr, hostPort, durableId.c_str(), durableTimeOut);
    const auto memIdBuffer = memId->getDSMemberId(memIdBufferLength);
    handShakeMsg->writeBytes((int8_t*)memIdBuffer, memIdBufferLength);
  }
  handShakeMsg->writeInt((int32_t)1);

  bool isDhOn = false;
  bool requireServerAuth = false;
  std::shared_ptr<Properties> credentials;
  std::shared_ptr<CacheableBytes> serverChallenge;

  // Write overrides (just conflation for now)
  handShakeMsg->write(getOverrides(&sysProp));

  bool tmpIsSecurityOn = nullptr != cacheImpl->getAuthInitialize();
  isDhOn = sysProp.isDhOn();

  if (m_endpointObj) {
    tmpIsSecurityOn = tmpIsSecurityOn || this->m_endpointObj->isMultiUserMode();
    auto dhalgo =
        sysProp.getSecurityProperties()->find("security-client-dhalgo");

    LOGDEBUG("TcrConnection this->m_endpointObj->isMultiUserMode() = %d ",
             this->m_endpointObj->isMultiUserMode());
    if (this->m_endpointObj->isMultiUserMode()) {
      if (dhalgo != nullptr && dhalgo->length() > 0) isDhOn = true;
    }
  }

  LOGDEBUG(
      "TcrConnection algo name %s tmpIsSecurityOn = %d isDhOn = %d "
      "isNotificationChannel = %d ",
      sysProp.securityClientDhAlgo().c_str(), tmpIsSecurityOn, isDhOn,
      isNotificationChannel);
  bool doIneedToSendCreds = true;
  if (isNotificationChannel && m_endpointObj &&
      this->m_endpointObj->isMultiUserMode()) {
    isDhOn = false;
    tmpIsSecurityOn = false;
    doIneedToSendCreds = false;
  }

  if (isNotificationChannel && !doIneedToSendCreds) {
    handShakeMsg->write(
        static_cast<uint8_t>(SECURITY_MULTIUSER_NOTIFICATIONCHANNEL));
  } else if (isDhOn) {
    m_dh = new DiffieHellman();
    m_dh->initDhKeys(sysProp.getSecurityProperties());
    handShakeMsg->write(static_cast<uint8_t>(SECURITY_CREDENTIALS_DHENCRYPT));
  } else if (tmpIsSecurityOn) {
    handShakeMsg->write(static_cast<uint8_t>(SECURITY_CREDENTIALS_NORMAL));
  } else {
    handShakeMsg->write(static_cast<uint8_t>(SECURITY_CREDENTIALS_NONE));
  }

  if (tmpIsSecurityOn) {
    try {
      LOGFINER("TcrConnection: about to invoke authloader");
      const auto& tmpSecurityProperties = sysProp.getSecurityProperties();
      if (tmpSecurityProperties == nullptr) {
        LOGWARN("TcrConnection: security properties not found.");
      }
      // only for backward connection
      if (isClientNotification) {
        if (const auto& authInitialize = cacheImpl->getAuthInitialize()) {
          LOGFINER(
              "TcrConnection: acquired handle to authLoader, "
              "invoking getCredentials");

          const auto& tmpAuthIniSecurityProperties =
              authInitialize->getCredentials(tmpSecurityProperties, m_endpoint);
          LOGFINER("TcrConnection: after getCredentials ");
          credentials = tmpAuthIniSecurityProperties;
        }
      }

      if (isDhOn) {
        auto ksPath = tmpSecurityProperties->find("security-client-kspath");
        requireServerAuth = (ksPath != nullptr && ksPath->length() > 0);
        handShakeMsg->writeBoolean(requireServerAuth);
        LOGFINE(
            "HandShake: Server authentication using RSA signature %s required",
            requireServerAuth ? "is" : "not");

        // Send the symmetric key algorithm name string
        handShakeMsg->writeString(sysProp.securityClientDhAlgo());

        // Send the client's DH public key to the server
        auto dhPubKey = m_dh->getPublicKey();
        LOGDEBUG("DH pubkey send len is %d", dhPubKey->length());
        dhPubKey->toData(*handShakeMsg);

        if (requireServerAuth) {
          char serverChallengeBytes[64] = {0};
          RandGen getrand;
          for (int pos = 0; pos < 64; pos++) {
            serverChallengeBytes[pos] = getrand(255);
          }
          serverChallenge = CacheableBytes::create(
              std::vector<int8_t>(serverChallengeBytes, serverChallengeBytes + 64));
          serverChallenge->toData(*handShakeMsg);
        }
      } else {                       // if isDhOn
        if (isClientNotification) {  //:only for backward connection
          credentials->toData(*handShakeMsg);
        }
      }  // else isDhOn
    } catch (const AuthenticationRequiredException&) {
      LOGDEBUG("AuthenticationRequiredException got");
      throw;
    } catch (const AuthenticationFailedException&) {
      LOGDEBUG("AuthenticationFailedException got");
      throw;
    } catch (const Exception& ex) {
      LOGWARN("TcrConnection: failed to acquire handle to authLoader: [%s] %s",
              ex.getName().c_str(), ex.what());
      auto message =
          std::string("TcrConnection: failed to load authInit library: ") +
          ex.what();
      throwException(AuthenticationFailedException(message));
    }
  }

  size_t msgLengh;
  char* data = (char*)handShakeMsg->getBuffer(&msgLengh);
  LOGFINE("Attempting handshake with endpoint %s for %s%s connection", endpoint,
          isClientNotification ? (isSecondary ? "secondary " : "primary ") : "",
          isClientNotification ? "subscription" : "client");
  ConnErrType error = sendData(data, msgLengh, connectTimeout, false);

  if (error == CONN_NOERR) {
    auto acceptanceCode = readHandshakeData(1, connectTimeout);

    LOGDEBUG(" Handshake: Got Accept Code %d", acceptanceCode[0]);
    /* adongre */
    if (acceptanceCode[0] == REPLY_SSL_ENABLED && !sysProp.sslEnabled()) {
      LOGERROR("SSL is enabled on server, enable SSL in client as well");
      AuthenticationRequiredException ex(
          "SSL is enabled on server, enable SSL in client as well");
      GF_SAFE_DELETE_CON(m_conn);
      throwException(ex);
    }

    // if diffie-hellman based credential encryption is enabled
    if (isDhOn && acceptanceCode[0] == REPLY_OK) {
      // read the server's DH public key
      auto pubKeyBytes = readHandshakeByteArray(connectTimeout);
      LOGDEBUG(" Handshake: Got pubKeySize %d", pubKeyBytes->length());

      // set the server's public key on client's DH side
      // DiffieHellman::setPublicKeyOther(pubKeyBytes);
      m_dh->setPublicKeyOther(pubKeyBytes);

      // Note: SK Algo is set in DistributedSystem::connect()
      // DiffieHellman::computeSharedSecret();
      m_dh->computeSharedSecret();

      if (requireServerAuth) {
        // Read Subject Name
        auto subjectName = readHandshakeString(connectTimeout);
        LOGDEBUG("Got subject %s", subjectName->value().c_str());
        // read the server's signature bytes
        auto responseBytes = readHandshakeByteArray(connectTimeout);
        LOGDEBUG("Handshake: Got response size %d", responseBytes->length());
        LOGDEBUG("Handshake: Got serverChallenge size %d",
                 serverChallenge->length());
        if (!m_dh->verify(subjectName, serverChallenge, responseBytes)) {
          throwException(AuthenticationFailedException(
              "Handshake: failed to verify server challenge response"));
        }
        LOGFINE("HandShake: Verified server challenge response");
      }

      // read the challenge bytes from the server
      auto challengeBytes = readHandshakeByteArray(connectTimeout);
      LOGDEBUG("Handshake: Got challengeSize %d", challengeBytes->length());

      // encrypt the credentials and challenge bytes
      auto cleartext = cacheImpl->createDataOutput();
      if (isClientNotification) {  //:only for backward connection
        credentials->toData(*cleartext);
      }
      challengeBytes->toData(*cleartext);
      auto ciphertext =
          m_dh->encrypt(cleartext->getBuffer(), cleartext->getBufferLength());

      auto sendCreds = cacheImpl->createDataOutput();
      ciphertext->toData(*sendCreds);
      size_t credLen;
      char* credData = (char*)sendCreds->getBuffer(&credLen);
      // send the encrypted bytes and check the response
      error = sendData(credData, credLen, connectTimeout, false);

      if (error == CONN_NOERR) {
        acceptanceCode = readHandshakeData(1, connectTimeout);
        LOGDEBUG("Handshake: Got acceptanceCode Finally %d",
                 acceptanceCode[0]);
      } else {
        int32_t lastError = ACE_OS::last_error();
        LOGERROR("Handshake failed, errno: %d, server may not be running",
                 lastError);
        GF_SAFE_DELETE_CON(m_conn);
        if (error & CONN_TIMEOUT) {
          throwException(TimeoutException(
              "TcrConnection::TcrConnection: "
              "connection timed out during diffie-hellman handshake"));
        } else {
          throwException(
              GeodeIOException("TcrConnection::TcrConnection: "
                               "Handshake failure during diffie-hellman"));
        }
      }
    }

    auto serverQueueStatus = readHandshakeData(1, connectTimeout);

    //  TESTING: Durable clients - set server queue status.
    // 0 - Non-Redundant , 1- Redundant , 2- Primary
    if (serverQueueStatus[0] == 1) {
      m_hasServerQueue = REDUNDANT_SERVER;
    } else if (serverQueueStatus[0] == 2) {
      m_hasServerQueue = PRIMARY_SERVER;
    } else {
      m_hasServerQueue = NON_REDUNDANT_SERVER;
    }
    auto queueSizeMsg = readHandshakeData(4, connectTimeout);
    auto dI = cacheImpl->createDataInput(
        reinterpret_cast<const uint8_t*>(queueSizeMsg.data()),
        queueSizeMsg.size());
    int32_t queueSize = 0;
    queueSize = dI->readInt32();
    m_queueSize = queueSize > 0 ? queueSize : 0;

    m_endpointObj->setServerQueueStatus(m_hasServerQueue, m_queueSize);

    ////////////////////////// Set Pool Specific Q Size only when
    ///////////////////////////////////
    ////////////////////////// 1. SereverQStatus = Primary or
    ///////////////////////////////////
    ////////////////////////// 2. SereverQStatus = Non-Redundant and
    ///////////////////////////////////
    ////////////////////////// 3. Only when handshake is for subscription
    ///////////////////////////////////
    if (m_poolDM != nullptr) {
      if ((m_hasServerQueue == PRIMARY_SERVER ||
           m_hasServerQueue == NON_REDUNDANT_SERVER) &&
          isClientNotification) {
        m_poolDM->setPrimaryServerQueueSize(queueSize);
      }
    }

    if (!isClientNotification) {
      // Read and ignore the DistributedMember object
      auto arrayLenHeader = readHandshakeData(1, connectTimeout);
      int32_t recvMsgLen = static_cast<int32_t>(arrayLenHeader[0]);
      // now check for array length headers - since GFE 5.7
      if (static_cast<int8_t>(arrayLenHeader[0]) == -2) {
        auto recvMsgLenBytes = readHandshakeData(2, connectTimeout);
        auto dI2 = m_connectionManager->getCacheImpl()->createDataInput(
            reinterpret_cast<const uint8_t*>(recvMsgLenBytes.data()),
            recvMsgLenBytes.size());
        recvMsgLen = dI2->readInt16();
      } else if (static_cast<int8_t>(arrayLenHeader[0]) == -3) {
        auto recvMsgLenBytes = readHandshakeData(4, connectTimeout);
        auto dI2 = m_connectionManager->getCacheImpl()->createDataInput(
            reinterpret_cast<const uint8_t*>(recvMsgLenBytes.data()),
            recvMsgLenBytes.size());
        recvMsgLen = dI2->readInt32();
      }
      auto recvMessage = readHandshakeData(recvMsgLen, connectTimeout);
      // If the distributed member has not been set yet, set it.
      if (getEndpointObject()->getDistributedMemberID() == 0) {
        LOGDEBUG("Deserializing distributed member Id");
        auto diForClient = cacheImpl->createDataInput(
            reinterpret_cast<const uint8_t*>(recvMessage.data()),
            recvMessage.size());
        auto member = std::static_pointer_cast<ClientProxyMembershipID>(
            diForClient->readObject());
        auto memId = cacheImpl->getMemberListForVersionStamp()->add(member);
        getEndpointObject()->setDistributedMemberID(memId);
        LOGDEBUG("Deserialized distributed member Id %d", memId);
      }
    }

    auto recvMsgLenBytes = readHandshakeData(2, connectTimeout);
    auto dI3 = m_connectionManager->getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(recvMsgLenBytes.data()),
        recvMsgLenBytes.size());
    uint16_t recvMsgLen2 = dI3->readInt16();
    auto recvMessage = readHandshakeData(recvMsgLen2, connectTimeout);

    if (!isClientNotification) {
      auto deltaEnabledMsg = readHandshakeData(1, connectTimeout);
      auto di = m_connectionManager->getCacheImpl()->createDataInput(
          reinterpret_cast<const uint8_t*>(deltaEnabledMsg.data()), 1);
      ThinClientBaseDM::setDeltaEnabledOnServer(di->readBoolean());
    }

    switch (acceptanceCode[0]) {
      case REPLY_OK:
      case SUCCESSFUL_SERVER_TO_CLIENT:
        LOGFINER("Handshake reply: %u,%u,%u", acceptanceCode[0],
                 serverQueueStatus[0], recvMsgLen2);
        if (isClientNotification) readHandshakeInstantiatorMsg(connectTimeout);
        break;
      case REPLY_AUTHENTICATION_FAILED: {
        AuthenticationFailedException ex((char*)recvMessage.data());
        GF_SAFE_DELETE_CON(m_conn);
        throwException(ex);
        // not expected to be reached
        break;
      }
      case REPLY_AUTHENTICATION_REQUIRED: {
        AuthenticationRequiredException ex((char*)recvMessage.data());
        GF_SAFE_DELETE_CON(m_conn);
        throwException(ex);
        // not expected to be reached
        break;
      }
      case REPLY_DUPLICATE_DURABLE_CLIENT: {
        DuplicateDurableClientException ex((char*)recvMessage.data());
        GF_SAFE_DELETE_CON(m_conn);
        throwException(ex);
        // not expected to be reached
        break;
      }
      case REPLY_REFUSED:
      case REPLY_INVALID:
      case UNSUCCESSFUL_SERVER_TO_CLIENT: {
        LOGERROR("Handshake rejected by server[%s]: %s",
                 m_endpointObj->name().c_str(), (char*)recvMessage.data());
        auto message =
            std::string("TcrConnection::TcrConnection: ") +
            "Handshake rejected by server: " + (char*)recvMessage.data();
        CacheServerException ex(message);
        GF_SAFE_DELETE_CON(m_conn);
        throw ex;
      }
      default: {
        LOGERROR(
            "Unknown error[%d] received from server [%s] in handshake: "
            "%s",
            acceptanceCode[0], m_endpointObj->name().c_str(),
            recvMessage.data());
        auto message =
            std::string("TcrConnection::TcrConnection: Unknown error") +
            " received from server in handshake: " +
            (char*)recvMessage.data();
        MessageException ex(message);
        GF_SAFE_DELETE_CON(m_conn);
        throw ex;
      }
    }

  } else {
    int32_t lastError = ACE_OS::last_error();
    LOGFINE("Handshake failed, errno: %d: %s", lastError,
            ACE_OS::strerror(lastError));
    GF_SAFE_DELETE_CON(m_conn);
    if (error & CONN_TIMEOUT) {
      throw TimeoutException(
          "TcrConnection::TcrConnection: "
          "connection timed out during handshake");
    } else {
      throw GeodeIOException(
          "TcrConnection::TcrConnection: "
          "Handshake failure");
    }
  }

  // TODO: we can authenticate endpoint here if pool is not in multiuser mode.
  // for backward connection we send credentials to server in handshake itself.
  // for forward connection we need to send credentail to server
  //---if pool in not in multiuser node
  //---or old endpoint case.

  if (this->m_endpointObj && !isNotificationChannel && tmpIsSecurityOn &&
      (!isPool || !this->m_endpointObj->isMultiUserMode())) {
    // this->m_endpointObj->authenticateEndpoint(this);
    return true;
  }

  return false;
}

Connector* TcrConnection::createConnection(
    const char* endpoint, std::chrono::microseconds connectTimeout,
    int32_t maxBuffSizePool) {
  Connector* socket = nullptr;
  auto& systemProperties = m_connectionManager->getCacheImpl()
                               ->getDistributedSystem()
                               .getSystemProperties();
  if (systemProperties.sslEnabled()) {
    socket = new TcpSslConn(endpoint, connectTimeout, maxBuffSizePool,
                            systemProperties.sslKeystorePassword().c_str(),
                            systemProperties.sslTrustStore().c_str(),
                            systemProperties.sslKeyStore().c_str());
  } else {
    socket = new TcpConn(endpoint, connectTimeout, maxBuffSizePool);
  }
  // as socket.init() calls throws exception...
  m_conn = socket;
  socket->init();
  return socket;
}

/* The timeout behaviour for different methods is as follows:
 * receive():
 *   Header: default timeout
 *   Body: default timeout
 * sendRequest()/sendRequestForChunkedResponse():
 *  default timeout during send; for receive:
 *   Header: default timeout * default timeout retries to handle large payload
 *           if a timeout other than default timeout is specified then
 *           that is used instead
 *   Body: default timeout
 */
inline ConnErrType TcrConnection::receiveData(
    char* buffer, int32_t length, std::chrono::microseconds receiveTimeoutSec,
    bool checkConnected, bool isNotificationMessage) {
  GF_DEV_ASSERT(buffer != nullptr);
  GF_DEV_ASSERT(m_conn != nullptr);

  std::chrono::microseconds defaultWaitSecs =
      isNotificationMessage ? std::chrono::seconds(1) : std::chrono::seconds(2);
  if (defaultWaitSecs > receiveTimeoutSec) defaultWaitSecs = receiveTimeoutSec;

  int32_t startLen = length;

  while (length > 0 && receiveTimeoutSec > std::chrono::microseconds::zero()) {
    if (checkConnected && !m_connected) {
      return CONN_IOERR;
    }
    if (receiveTimeoutSec < defaultWaitSecs) {
      defaultWaitSecs = receiveTimeoutSec;
    }
    int32_t readBytes = m_conn->receive(buffer, length, defaultWaitSecs);
    int32_t lastError = ACE_OS::last_error();
    length -= readBytes;
    if (length > 0 && lastError != ETIME && lastError != ETIMEDOUT) {
      return CONN_IOERR;
    }
    buffer += readBytes;
    /*
      Update pools byteRecieved stat here.
      readMessageChunked, readMessage, readHandshakeData,
      readHandshakeRawData, readHandShakeBytes, readHandShakeInt,
      readHandshakeString, all call TcrConnection::receiveData.
    */
    LOGDEBUG("TcrConnection::receiveData length = %d defaultWaitSecs = %d",
             length, defaultWaitSecs.count());
    if (m_poolDM != nullptr) {
      LOGDEBUG("TcrConnection::receiveData readBytes = %d", readBytes);
      m_poolDM->getStats().incReceivedBytes(static_cast<int64_t>(readBytes));
    }
    receiveTimeoutSec -= defaultWaitSecs;
    if ((length == startLen) && isNotificationMessage) {  // no data read
      break;
    }
  }
  //  Postconditions for checking bounds.
  GF_DEV_ASSERT(startLen >= length);
  GF_DEV_ASSERT(length >= 0);
  return (length == 0 ? CONN_NOERR
                      : (length == startLen ? CONN_NODATA : CONN_TIMEOUT));
}

inline ConnErrType TcrConnection::sendData(
    const char* buffer, int32_t length, std::chrono::microseconds sendTimeout,
    bool checkConnected) {
  std::chrono::microseconds dummy{0};
  return sendData(dummy, buffer, length, sendTimeout, checkConnected);
}

inline ConnErrType TcrConnection::sendData(
    std::chrono::microseconds& timeSpent, const char* buffer, int32_t length,
    std::chrono::microseconds sendTimeout, bool checkConnected) {
  GF_DEV_ASSERT(buffer != nullptr);
  GF_DEV_ASSERT(m_conn != nullptr);

  std::chrono::microseconds defaultWaitSecs = std::chrono::seconds(2);
  if (defaultWaitSecs > sendTimeout) defaultWaitSecs = sendTimeout;
  LOGDEBUG(
      "before send len %d sendTimeoutSec = %d checkConnected = %d m_connected "
      "%d",
      length, sendTimeout.count(), checkConnected, m_connected);
  while (length > 0 && sendTimeout > std::chrono::microseconds::zero()) {
    if (checkConnected && !m_connected) {
      return CONN_IOERR;
    }
    if (sendTimeout < defaultWaitSecs) {
      defaultWaitSecs = sendTimeout;
    }
    int32_t sentBytes = m_conn->send(buffer, length, defaultWaitSecs);

    length -= sentBytes;
    buffer += sentBytes;
    // we don't want to decrement the remaining time for the last iteration
    if (length == 0) {
      break;
    }
    int32_t lastError = ACE_OS::last_error();
    if (length > 0 && lastError != ETIME && lastError != ETIMEDOUT) {
      return CONN_IOERR;
    }

    timeSpent += defaultWaitSecs;
    sendTimeout -= defaultWaitSecs;
  }

  return (length == 0 ? CONN_NOERR : CONN_TIMEOUT);
}

char* TcrConnection::sendRequest(const char* buffer, int32_t len,
                                 size_t* recvLen,
                                 std::chrono::microseconds sendTimeoutSec,
                                 std::chrono::microseconds receiveTimeoutSec,
                                 int32_t request) {
  LOGDEBUG("TcrConnection::sendRequest");
  std::chrono::microseconds timeSpent{0};

  send(timeSpent, buffer, len, sendTimeoutSec);

  if (timeSpent >= receiveTimeoutSec)
    throwException(
        TimeoutException("TcrConnection::send: connection timed out"));

  receiveTimeoutSec -= timeSpent;
  ConnErrType opErr = CONN_NOERR;
  return readMessage(recvLen, receiveTimeoutSec, true, &opErr, false, request);
}

void TcrConnection::sendRequestForChunkedResponse(
    const TcrMessage& request, int32_t len, TcrMessageReply& reply,
    std::chrono::microseconds sendTimeoutSec,
    std::chrono::microseconds receiveTimeoutSec) {
  int32_t msgType = request.getMessageType();
  // ACE_OS::memcpy(&msgType, buffer, 4);
  // msgType = ntohl(msgType);

  /*receiveTimeoutSec = (msgType == TcrMessage::QUERY ||
    msgType == TcrMessage::QUERY_WITH_PARAMETERS ||
    msgType == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
    msgType == TcrMessage::GETDURABLECQS_MSG_TYPE ||
    msgType == TcrMessage::EXECUTE_FUNCTION ||
    msgType == TcrMessage::EXECUTE_REGION_FUNCTION ||
    msgType == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP)
    ? reply.getTimeout() : receiveTimeoutSec;

  //send + recieve should be part of API timeout
  sendTimeoutSec = (msgType == TcrMessage::QUERY ||
    msgType == TcrMessage::QUERY_WITH_PARAMETERS ||
    msgType == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
    msgType == TcrMessage::GETDURABLECQS_MSG_TYPE ||
    msgType == TcrMessage::EXECUTE_FUNCTION ||
    msgType == TcrMessage::EXECUTE_REGION_FUNCTION ||
    msgType == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP)
    ? reply.getTimeout() : sendTimeoutSec;
    */
  switch (msgType) {
    case TcrMessage::QUERY:
    case TcrMessage::QUERY_WITH_PARAMETERS:
    case TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE:
    case TcrMessage::GETDURABLECQS_MSG_TYPE:
    case TcrMessage::EXECUTE_FUNCTION:
    case TcrMessage::EXECUTE_REGION_FUNCTION:
    case TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP: {
      receiveTimeoutSec = reply.getTimeout();
      sendTimeoutSec = reply.getTimeout();
      break;
    }
    default:
      break;
  }
  /*if((msgType == TcrMessage::QUERY ||
    msgType == TcrMessage::QUERY_WITH_PARAMETERS ||
    msgType == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE ||
    msgType == TcrMessage::GETDURABLECQS_MSG_TYPE ||
    msgType == TcrMessage::EXECUTE_FUNCTION ||
    msgType == TcrMessage::EXECUTE_REGION_FUNCTION))
  {
    receiveTimeoutSec = reply.getTimeout();
    sendTimeoutSec = reply.getTimeout();
  }*/

  // send(buffer, len, sendTimeoutSec);
  std::chrono::microseconds timeSpent{0};
  send(timeSpent, request.getMsgData(), len, sendTimeoutSec, true);

  if (timeSpent >= receiveTimeoutSec)
    throwException(
        TimeoutException("TcrConnection::send: connection timed out"));

  receiveTimeoutSec -= timeSpent;

  // to help in decoding the reply based on what was the request type
  reply.setMessageTypeRequest(msgType);
  // no need of it now, this will not come here
  if (msgType == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP) {
    ChunkedFunctionExecutionResponse* resultCollector =
        static_cast<ChunkedFunctionExecutionResponse*>(
            reply.getChunkedResultHandler());
    if (resultCollector->getResult() == false) {
      LOGDEBUG(
          "TcrConnection::sendRequestForChunkedResponse: function execution, "
          "no response desired");
      return;
    }
  }
  readMessageChunked(reply, receiveTimeoutSec, true);
}

void TcrConnection::send(const char* buffer, int len,
                         std::chrono::microseconds sendTimeoutSec,
                         bool checkConnected) {
  std::chrono::microseconds dummy;
  send(dummy, buffer, len, sendTimeoutSec, checkConnected);
}

void TcrConnection::send(std::chrono::microseconds& timeSpent,
                         const char* buffer, int len,
                         std::chrono::microseconds sendTimeoutSec,
                         bool checkConnected) {
  GF_DEV_ASSERT(m_conn != nullptr);

  // LOGINFO("TcrConnection::send: [%p] sending request to endpoint %s;",
  //:  this, m_endpoint);

  LOGDEBUG(
      "TcrConnection::send: [%p] sending request to endpoint %s; bytes: %s",
      this, m_endpoint, Utils::convertBytesToString(buffer, len).c_str());

  ConnErrType error = sendData(timeSpent, buffer, len, sendTimeoutSec);

  LOGFINER(
      "TcrConnection::send: completed send request to endpoint %s "
      "with error: %d",
      m_endpoint, error);

  if (error != CONN_NOERR) {
    if (error == CONN_TIMEOUT) {
      throwException(
          TimeoutException("TcrConnection::send: connection timed out"));
    } else {
      throwException(
          GeodeIOException("TcrConnection::send: connection failure"));
    }
  }
}

char* TcrConnection::receive(size_t* recvLen, ConnErrType* opErr,
                             std::chrono::microseconds receiveTimeoutSec) {
  GF_DEV_ASSERT(m_conn != nullptr);

  return readMessage(recvLen, receiveTimeoutSec, false, opErr, true);
}

char* TcrConnection::readMessage(size_t* recvLen,
                                 std::chrono::microseconds receiveTimeoutSec,
                                 bool doHeaderTimeoutRetries,
                                 ConnErrType* opErr, bool isNotificationMessage,
                                 int32_t request) {
  char msg_header[HEADER_LENGTH];
  int32_t msgType, msgLen;
  ConnErrType error;

  std::chrono::microseconds headerTimeout = receiveTimeoutSec;
  if (doHeaderTimeoutRetries &&
      receiveTimeoutSec == DEFAULT_READ_TIMEOUT_SECS) {
    headerTimeout = DEFAULT_READ_TIMEOUT_SECS * DEFAULT_TIMEOUT_RETRIES;
  }

  LOGDEBUG("TcrConnection::readMessage: receiving reply from endpoint %s",
           m_endpoint);

  error = receiveData(msg_header, HEADER_LENGTH, headerTimeout, true,
                      isNotificationMessage);
  LOGDEBUG("TcrConnection::readMessage after recieve data");
  if (error != CONN_NOERR) {
    //  the !isNotificationMessage ensures that notification channel
    // gets the TimeoutException when no data was received and is ignored by
    // notification channel; when data has been received then it throws
    // GeodeIOException that causes the channel to close as required
    if (error == CONN_NODATA ||
        (error == CONN_TIMEOUT && !isNotificationMessage)) {
      if (isNotificationMessage) {
        // fix #752 - do not throw periodic TimeoutException for subscription
        // channels to avoid frequent stack trace processing.
        return nullptr;
      } else {
        throwException(TimeoutException(
            "TcrConnection::readMessage: "
            "connection timed out while receiving message header"));
      }
    } else {
      if (isNotificationMessage) {
        *opErr = CONN_IOERR;
        return nullptr;
      }
      throwException(GeodeIOException(
          "TcrConnection::readMessage: "
          "connection failure while receiving message header"));
    }
  }

  LOGDEBUG(
      "TcrConnection::readMessage: received header from endpoint %s; "
      "bytes: %s",
      m_endpoint,
      Utils::convertBytesToString(msg_header, HEADER_LENGTH).c_str());

  auto input = m_connectionManager->getCacheImpl()->createDataInput(
      reinterpret_cast<uint8_t*>(msg_header), HEADER_LENGTH);
  msgType = input->readInt32();
  msgLen = input->readInt32();
  //  check that message length is valid.
  if (!(msgLen > 0) && request == TcrMessage::GET_CLIENT_PR_METADATA) {
    char* fullMessage;
    *recvLen = HEADER_LENGTH + msgLen;
    _GEODE_NEW(fullMessage, char[HEADER_LENGTH + msgLen]);
    ACE_OS::memcpy(fullMessage, msg_header, HEADER_LENGTH);
    return fullMessage;
    // exit(0);
  }
  // GF_DEV_ASSERT(msgLen > 0);

  // user has to delete this pointer
  char* fullMessage;
  *recvLen = HEADER_LENGTH + msgLen;
  _GEODE_NEW(fullMessage, char[HEADER_LENGTH + msgLen]);
  ACE_OS::memcpy(fullMessage, msg_header, HEADER_LENGTH);

  std::chrono::microseconds mesgBodyTimeout = receiveTimeoutSec;
  if (isNotificationMessage) {
    mesgBodyTimeout = receiveTimeoutSec * DEFAULT_TIMEOUT_RETRIES;
  }
  error = receiveData(fullMessage + HEADER_LENGTH, msgLen, mesgBodyTimeout,
                      true, isNotificationMessage);
  if (error != CONN_NOERR) {
    delete[] fullMessage;
    //  the !isNotificationMessage ensures that notification channel
    // gets the GeodeIOException and not TimeoutException;
    // this is required since header has already been read meaning there could
    // be stale data on socket and so it should close the notification channel
    // while TimeoutException is normally ignored by notification channel
    if ((error & CONN_TIMEOUT) && !isNotificationMessage) {
      throwException(TimeoutException(
          "TcrConnection::readMessage: "
          "connection timed out while receiving message body"));
    } else {
      if (isNotificationMessage) {
        *opErr = CONN_IOERR;
        return nullptr;
      }
      throwException(
          GeodeIOException("TcrConnection::readMessage: "
                           "connection failure while receiving message body"));
    }
  }

  LOGDEBUG(
      "TcrConnection::readMessage: received message body from "
      "endpoint %s; bytes: %s",
      m_endpoint,
      Utils::convertBytesToString(fullMessage + HEADER_LENGTH, msgLen)

          .c_str());

  // This is the test case when msg type is GET_CLIENT_PR_METADATA and msgLen is
  // 0.
  /*if (request == TcrMessage::GET_CLIENT_PR_METADATA) {
  LOGCONFIG("Amey request == TcrMessage::GET_CLIENT_PR_METADATA");
  char* fullMessage2;
  *recvLen = HEADER_LENGTH;
  _GEODE_NEW( fullMessage2, char[HEADER_LENGTH ] );
  ACE_OS::memcpy(fullMessage2, msg_header, HEADER_LENGTH);
  return fullMessage2;
  }*/

  return fullMessage;
}

void TcrConnection::readMessageChunked(
    TcrMessageReply& reply, std::chrono::microseconds receiveTimeoutSec,
    bool doHeaderTimeoutRetries) {
  const int HDR_LEN = 5;
  const int HDR_LEN_12 = 12;
  uint8_t msg_header[HDR_LEN_12 + HDR_LEN];
  ConnErrType error;

  std::chrono::microseconds headerTimeout = receiveTimeoutSec;
  if (doHeaderTimeoutRetries &&
      receiveTimeoutSec == DEFAULT_READ_TIMEOUT_SECS) {
    headerTimeout = DEFAULT_READ_TIMEOUT_SECS * DEFAULT_TIMEOUT_RETRIES;
  }

  LOGFINER(
      "TcrConnection::readMessageChunked: receiving reply from "
      "endpoint %s",
      m_endpoint);

  error = receiveData(reinterpret_cast<char*>(msg_header), HDR_LEN_12 + HDR_LEN,
                      headerTimeout, true, false);
  if (error != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      throwException(TimeoutException(
          "TcrConnection::readMessageChunked: "
          "connection timed out while receiving message header"));
    } else {
      throwException(GeodeIOException(
          "TcrConnection::readMessageChunked: "
          "connection failure while receiving message header"));
    }
  }

  LOGDEBUG(
      "TcrConnection::readMessageChunked: received header from "
      "endpoint %s; bytes: %s",
      m_endpoint, Utils::convertBytesToString(msg_header, HDR_LEN_12).c_str());

  auto input = m_connectionManager->getCacheImpl()->createDataInput(
      msg_header, HDR_LEN_12);
  int32_t msgType = input->readInt32();
  reply.setMessageType(msgType);
  int32_t txId;
  int32_t numOfParts = input->readInt32();
  LOGDEBUG("TcrConnection::readMessageChunked numberof parts = %d ",
           numOfParts);
  // input->advanceCursor(4);
  txId = input->readInt32();
  reply.setTransId(txId);

  // bool isLastChunk = false;
  uint8_t isLastChunk = 0x0;

  int chunkNum = 0;

  // Initialize the chunk processing
  reply.startProcessChunk(m_chunksProcessSema);

  //  indicate an end to chunk processing and wait for processing
  // to end even if reading the chunks fails in middle
  struct FinalizeProcessChunk {
   private:
    TcrMessage& m_reply;
    uint16_t m_endpointmemId;

   public:
    FinalizeProcessChunk(TcrMessageReply& reply, uint16_t endpointmemId)
        : m_reply(reply), m_endpointmemId(endpointmemId) {}
    ~FinalizeProcessChunk() {
      // Enqueue a nullptr chunk indicating a wait for processing to complete.
      m_reply.processChunk(nullptr, 0, m_endpointmemId);
    }
  } endProcessChunk(reply, m_endpointObj->getDistributedMemberID());
  bool first = true;
  do {
    // uint8_t chunk_header[HDR_LEN];
    if (!first) {
      error = receiveData(reinterpret_cast<char*>(msg_header + HDR_LEN_12),
                          HDR_LEN, headerTimeout, true, false);
      if (error != CONN_NOERR) {
        if (error & CONN_TIMEOUT) {
          throwException(TimeoutException(
              "TcrConnection::readMessageChunked: "
              "connection timed out while receiving chunk header"));
        } else {
          throwException(GeodeIOException(
              "TcrConnection::readMessageChunked: "
              "connection failure while receiving chunk header"));
        }
      }
    } else {
      first = false;
    }
    ++chunkNum;

    LOGDEBUG(
        "TcrConnection::readMessageChunked: received chunk header %d "
        "from endpoint %s; bytes: %s",
        chunkNum, m_endpoint,
        Utils::convertBytesToString((msg_header + HDR_LEN_12), HDR_LEN)

            .c_str());

    auto input =
        m_connectionManager->getCacheImpl()->createDataInput(
            msg_header + HDR_LEN_12, HDR_LEN);
    int32_t chunkLen;
    chunkLen = input->readInt32();
    //  check that chunk length is valid.
    GF_DEV_ASSERT(chunkLen > 0);
    isLastChunk = input->read();

    uint8_t* chunk_body;
    _GEODE_NEW(chunk_body, uint8_t[chunkLen]);
    error = receiveData(reinterpret_cast<char*>(chunk_body), chunkLen,
                        receiveTimeoutSec, true, false);
    if (error != CONN_NOERR) {
      delete[] chunk_body;
      if (error & CONN_TIMEOUT) {
        throwException(TimeoutException(
            "TcrConnection::readMessageChunked: "
            "connection timed out while receiving chunk body"));
      } else {
        throwException(
            GeodeIOException("TcrConnection::readMessageChunked: "
                             "connection failure while receiving chunk body"));
      }
    }

    LOGDEBUG(
        "TcrConnection::readMessageChunked: received chunk body %d "
        "from endpoint %s; bytes: %s",
        chunkNum, m_endpoint,
        Utils::convertBytesToString(chunk_body, chunkLen).c_str());
    // Process the chunk; the actual processing is done by a separate thread
    // ThinClientBaseDM::m_chunkProcessor.

    reply.processChunk(chunk_body, chunkLen,
                       m_endpointObj->getDistributedMemberID(), isLastChunk);
  } while (!(isLastChunk & 0x1));

  LOGFINER(
      "TcrConnection::readMessageChunked: read full reply "
      "from endpoint %s",
      m_endpoint);
}

void TcrConnection::close() {
  // If this is a short lived grid client, don't bother with this close ack
  // message
  if (m_poolDM->getConnectionManager()
          .getCacheImpl()
          ->getDistributedSystem()
          .getSystemProperties()
          .isGridClient()) {
    return;
  }

  TcrMessage* closeMsg = TcrMessage::getCloseConnMessage(
      m_poolDM->getConnectionManager().getCacheImpl()->getCache());
  try {
    if (!TcrConnectionManager::TEST_DURABLE_CLIENT_CRASH &&
        !m_connectionManager->isNetDown()) {
      send(closeMsg->getMsgData(), closeMsg->getMsgLength(),
           std::chrono::seconds(2), false);
    }
  } catch (Exception& e) {
    LOGINFO("Close connection message failed with msg: %s", e.what());
  } catch (...) {
    LOGINFO("Close connection message failed");
  }
}

std::vector<int8_t> TcrConnection::readHandshakeData(
    int32_t msgLength, std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;
  if (msgLength < 0) {
    msgLength = 0;
  }
  char* recvMessage;
  _GEODE_NEW(recvMessage, char[msgLength + 1]);
  recvMessage[msgLength] = '\0';
  if (msgLength == 0) {
    return std::vector<int8_t>(recvMessage, recvMessage +1);
  }
  if ((error = receiveData(recvMessage, msgLength, connectTimeout, false)) !=
      CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake"));
    } else {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure"));
    }
  } else {
    return std::vector<int8_t>(recvMessage, recvMessage + msgLength + 1);
  }
  return std::vector<int8_t>{};
}

std::shared_ptr<CacheableBytes> TcrConnection::readHandshakeRawData(
    int32_t msgLength, std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;
  if (msgLength < 0) {
    msgLength = 0;
  }
  if (msgLength == 0) {
    return nullptr;
  }
  char* recvMessage;
  _GEODE_NEW(recvMessage, char[msgLength]);
  if ((error = receiveData(recvMessage, msgLength, connectTimeout, false)) !=
      CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake"));
    } else {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure"));
    }
    // not expected to be reached
    return nullptr;
  } else {
    return CacheableBytes::create(std::vector<int8_t>(recvMessage, recvMessage +
                                        msgLength));
  }
}

std::shared_ptr<CacheableBytes> TcrConnection::readHandshakeByteArray(
    std::chrono::microseconds connectTimeout) {
  uint32_t arraySize = readHandshakeArraySize(connectTimeout);
  return readHandshakeRawData(arraySize, connectTimeout);
}

// read a byte array
uint32_t TcrConnection::readHandshakeArraySize(
    std::chrono::microseconds connectTimeout) {
  auto codeBytes = readHandshakeData(1, connectTimeout);
  auto codeDI = m_connectionManager->getCacheImpl()->createDataInput(
      reinterpret_cast<const uint8_t*>(codeBytes.data()),
      codeBytes.size());
  uint8_t code = codeDI->read();
  uint32_t arraySize = 0;
  if (code == 0xFF) {
    return 0;
  } else {
    int32_t tempLen = code;
    if (tempLen > 252) {  // 252 is java's ((byte)-4 && 0xFF)
      if (code == 0xFE) {
        auto lenBytes = readHandshakeData(2, connectTimeout);
        auto lenDI = m_connectionManager->getCacheImpl()->createDataInput(
            reinterpret_cast<const uint8_t*>(lenBytes.data()),
            lenBytes.size());
        uint16_t val = lenDI->readInt16();
        tempLen = val;
      } else if (code == 0xFD) {
        auto lenBytes = readHandshakeData(4, connectTimeout);
        auto lenDI = m_connectionManager->getCacheImpl()->createDataInput(
            reinterpret_cast<const uint8_t*>(lenBytes.data()),
            lenBytes.size());
        uint32_t val = lenDI->readInt32();
        tempLen = val;
      } else {
        GF_SAFE_DELETE_CON(m_conn);
        throwException(IllegalStateException("unexpected array length code"));
      }
    }
    arraySize = tempLen;
  }

  return arraySize;
}

void TcrConnection::readHandshakeInstantiatorMsg(
    std::chrono::microseconds connectTimeout) {
  int hashMapSize = readHandshakeArraySize(connectTimeout);
  for (int i = 0; i < hashMapSize; i++) {
    readHandShakeBytes(6, connectTimeout);  // reading integer and arraylist
                                            // type
    int aLen = readHandshakeArraySize(connectTimeout);
    for (int j = 0; j < aLen; j++) {
      readHandshakeString(connectTimeout);
    }
  }

  hashMapSize = readHandshakeArraySize(connectTimeout);
  for (int i = 0; i < hashMapSize; i++) {
    readHandShakeBytes(5, connectTimeout);  // reading integer
    readHandshakeString(connectTimeout);
  }

  // added in 3.6 and 6.6
  int hashMapSize2 = readHandshakeArraySize(connectTimeout);
  for (int i = 0; i < hashMapSize2; i++) {
    readHandShakeBytes(6, connectTimeout);  // reading integer and arraylist
                                            // type
    int aLen = readHandshakeArraySize(connectTimeout);
    for (int j = 0; j < aLen; j++) {
      readHandshakeString(connectTimeout);
    }
  }
}
void TcrConnection::readHandShakeBytes(
    int numberOfBytes, std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;
  uint8_t* recvMessage;
  _GEODE_NEW(recvMessage, uint8_t[numberOfBytes]);

  if ((error = receiveData(reinterpret_cast<char*>(recvMessage), numberOfBytes,
                           connectTimeout, false)) != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake"));
    } else {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure"));
    }
  }

  _GEODE_SAFE_DELETE_ARRAY(recvMessage);
}

int32_t TcrConnection::readHandShakeInt(
    std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;
  uint8_t* recvMessage;
  _GEODE_NEW(recvMessage, uint8_t[4]);

  if ((error = receiveData(reinterpret_cast<char*>(recvMessage), 4,
                           connectTimeout, false)) != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake"));
    } else {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure"));
    }
  }

  auto di = m_connectionManager->getCacheImpl()->createDataInput(
      recvMessage, 4);
  int32_t val = di->readInt32();

  _GEODE_SAFE_DELETE_ARRAY(recvMessage);

  return val;
}

std::shared_ptr<CacheableString> TcrConnection::readHandshakeString(
    std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;

  char cstypeid;
  if (receiveData(&cstypeid, 1, connectTimeout, false) != CONN_NOERR) {
    GF_SAFE_DELETE_CON(m_conn);
    if (error & CONN_TIMEOUT) {
      LOGFINE("Timeout receiving string typeid");
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake reading string type ID"));
    } else {
      LOGFINE("IO error receiving string typeid");
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure reading string type ID"));
    }
  }

  LOGDEBUG("Received string typeid as %d", cstypeid);

  uint32_t length = 0;
  switch (static_cast<int8_t>(cstypeid)) {
    case GeodeTypeIds::CacheableNullString: {
      return nullptr;
      break;
    }
    case GeodeTypeIds::CacheableASCIIString: {
      auto lenBytes = readHandshakeData(2, connectTimeout);
      auto lenDI = m_connectionManager->getCacheImpl()->createDataInput(
          reinterpret_cast<const uint8_t*>(lenBytes.data()),
          lenBytes.size());
      length = lenDI->readInt16();

      break;
    }
    default: {
      GF_SAFE_DELETE_CON(m_conn);
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure: Unexpected string type ID"));
    }
  }

  LOGDEBUG(" Received string len %d", length);

  if (length == 0) {
    return nullptr;
  }

  auto recvMessage = std::unique_ptr<char>(new char[length + 1]);
  recvMessage.get()[length] = '\0';

  if ((error = receiveData(recvMessage.get(), length, connectTimeout, false)) !=
      CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      GF_SAFE_DELETE_CON(m_conn);
      LOGFINE("Timeout receiving string data");
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake reading string bytes"));
    } else {
      GF_SAFE_DELETE_CON(m_conn);
      LOGFINE("IO error receiving string data");
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure reading string bytes"));
    }
    // not expected to be reached
    return nullptr;
  } else {
    LOGDEBUG(" Received string data [%s]", recvMessage.get());
    auto retval =
        CacheableString::create(std::string(recvMessage.get(), length));
    return retval;
  }
}
bool TcrConnection::hasExpired(const std::chrono::milliseconds& expiryTime) {
  if (expiryTime <= std::chrono::milliseconds::zero()) {
    return false;
  }

  ACE_Time_Value _expiryTime(expiryTime);

  if (ACE_OS::gettimeofday() - m_creationTime > _expiryTime) {
    return true;
  } else {
    return false;
  }
}

bool TcrConnection::isIdle(const std::chrono::milliseconds& idleTime) {
  if (idleTime <= std::chrono::milliseconds::zero()) {
    return false;
  }

  ACE_Time_Value _idleTime(idleTime);

  if (ACE_OS::gettimeofday() - m_lastAccessed > _idleTime) {
    return true;
  } else {
    return false;
  }
}

void TcrConnection::touch() { m_lastAccessed = ACE_OS::gettimeofday(); }

ACE_Time_Value TcrConnection::getLastAccessed() { return m_lastAccessed; }

uint8_t TcrConnection::getOverrides(const SystemProperties* props) {
  uint8_t conflateByte = 0;

  auto&& conflate = props->conflateEvents();
  if (conflate == "true") {
    conflateByte = 1;
  } else if (conflate == "false") {
    conflateByte = 2;
  }

  return conflateByte;
}

void TcrConnection::updateCreationTime() {
  m_creationTime = ACE_OS::gettimeofday();
  touch();
}

TcrConnection::~TcrConnection() {
  LOGDEBUG("Tcrconnection destructor %p . conn ref to endopint %d", this,
           m_endpointObj->getConnRefCounter());
  m_endpointObj->addConnRefCounter(-1);
  if (m_conn != nullptr) {
    LOGDEBUG("closing the connection");
    m_conn->close();
    GF_SAFE_DELETE_CON(m_conn);
  }

  if (m_dh != nullptr) {
    m_dh->clearDhKeys();
    _GEODE_SAFE_DELETE(m_dh);
  }
}

bool TcrConnection::setAndGetBeingUsed(volatile bool isBeingUsed,
                                       bool forTransaction) {
  uint32_t currentValue = 0;
  uint32_t retVal = 0U;

  if (!forTransaction) {
    if (isBeingUsed) {
      if (m_isUsed == 1 || m_isUsed == 2) return false;
      if (m_isUsed.compare_exchange_strong(currentValue, 1)) return true;
      return false;
    } else {
      m_isUsed = 0;
      return true;
    }
  } else {
    if (isBeingUsed) {
      if (m_isUsed == 1) {  // already used
        return false;
      }
      if (m_isUsed == 2) {  // transaction thread has set, reused it
        return true;
      }
      if (m_isUsed.compare_exchange_strong(currentValue, 2 /*for transaction*/))
        return true;
      return false;
    } else {
      // m_isUsed = 0;//this will done by releasing the connection by
      // transaction at the end of transaction
      return true;
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
