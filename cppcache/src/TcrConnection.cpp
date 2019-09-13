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

#include "TcrConnection.hpp"

#include <cinttypes>

#include <ace/INET_Addr.h>
#include <ace/OS.h>

#include <geode/AuthInitialize.hpp>
#include <geode/SystemProperties.hpp>

#include "ClientProxyMembershipID.hpp"
#include "Connector.hpp"
#include "DiffieHellman.hpp"
#include "DistributedSystemImpl.hpp"
#include "TcpSslConn.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrEndpoint.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientRegion.hpp"
#include "Utils.hpp"
#include "Version.hpp"

namespace apache {
namespace geode {
namespace client {

const int HEADER_LENGTH = 17;
const int CHUNK_HEADER_LENGTH = 5;
const int8_t LAST_CHUNK_MASK = 0x1;
const int64_t INITIAL_CONNECTION_ID = 26739;

#define throwException(ex)                            \
  {                                                   \
    LOGFINEST(ex.getName() + ": " + ex.getMessage()); \
    throw ex;                                         \
  }

struct FinalizeProcessChunk {
 private:
  TcrMessage& m_reply;
  uint16_t m_endpointMemId;

 public:
  FinalizeProcessChunk(TcrMessageReply& reply, uint16_t endpointMemId)
      : m_reply(reply), m_endpointMemId(endpointMemId) {}
  ~FinalizeProcessChunk() noexcept(false) {
    // Enqueue a nullptr chunk indicating a wait for processing to complete.
    m_reply.processChunk(std::vector<uint8_t>(), 0, m_endpointMemId);
  }
};

bool TcrConnection::initTcrConnection(
    TcrEndpoint* endpointObj, const char* endpoint,
    synchronized_set<std::unordered_set<uint16_t>>& ports,
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
  m_creationTime = clock::now();
  connectionId = INITIAL_CONNECTION_ID;
  m_lastAccessed = clock::now();
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
  m_endpoint = endpoint;
  // Precondition:
  // 1. isSecondary ==> isClientNotification

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

  auto handShakeMsg = cacheImpl->createDataOutput();
  bool isNotificationChannel = false;
  // Send byte Acceptor.CLIENT_TO_SERVER = (byte) 100;
  // Send byte Acceptor.SERVER_TO_CLIENT = (byte) 101;
  if (isClientNotification) {
    isNotificationChannel = true;
    if (isSecondary) {
      handShakeMsg.write(static_cast<int8_t>(SECONDARY_SERVER_TO_CLIENT));
    } else {
      handShakeMsg.write(static_cast<int8_t>(PRIMARY_SERVER_TO_CLIENT));
    }
  } else {
    handShakeMsg.write(static_cast<int8_t>(CLIENT_TO_SERVER));
  }

  // added for versioned client
  int8_t versionOrdinal = Version::getOrdinal();
  handShakeMsg.write(versionOrdinal);

  LOGFINE("Client version ordinal is %d", versionOrdinal);

  handShakeMsg.write(static_cast<int8_t>(REPLY_OK));

  // Send byte REPLY_OK = (byte)58;
  if (!isClientNotification) {
    m_port = m_conn->getPort();
    ports.insert(m_port);
  } else {
    auto&& lock = ports.make_lock();
    handShakeMsg.writeInt(static_cast<int32_t>(ports.size()));
    for (const auto& port : ports) {
      handShakeMsg.writeInt(static_cast<int32_t>(port));
    }
  }

  //  Writing handshake readtimeout value for CSVER_51+.
  if (!isClientNotification) {
    // SW: The timeout has been artificially raised to the highest
    // permissible value for bug #232 for now.
    //  minus 10 sec because the GFE 5.7 gridDev branch adds a
    // 5 sec buffer which was causing an int overflow.
    handShakeMsg.writeInt(static_cast<int32_t>(0x7fffffff) - 10000);
  }

  // Write header for byte FixedID since GFE 5.7
  handShakeMsg.write(static_cast<int8_t>(DSCode::FixedIDByte));
  // Writing byte for ClientProxyMembershipID class id=38 as registered on the
  // java server.
  handShakeMsg.write(static_cast<int8_t>(DSCode::ClientProxyMembershipId));
  if (endpointObj->getPoolHADM()) {
    ClientProxyMembershipID* memId =
        endpointObj->getPoolHADM()->getMembershipId();
    uint32_t memIdBufferLength;
    auto memIdBuffer = memId->getDSMemberId(memIdBufferLength);
    handShakeMsg.writeBytes(
        reinterpret_cast<int8_t*>(const_cast<char*>(memIdBuffer)),
        memIdBufferLength);
  } else {
    ACE_TCHAR hostName[256];
    ACE_OS::hostname(hostName, sizeof(hostName) - 1);

    ACE_INET_Addr driver(hostName);

    uint16_t hostPort = 0;

    // Add 3 durable Subcription properties to ClientProxyMembershipID

    auto&& durableId = sysProp.durableClientId();
    auto&& durableTimeOut = sysProp.durableTimeout();

    // Write ClientProxyMembershipID serialized object.
    uint32_t memIdBufferLength;
    auto memId = cacheImpl->getClientProxyMembershipIDFactory().create(
        hostName, driver, hostPort, durableId.c_str(), durableTimeOut);
    auto memIdBuffer = memId->getDSMemberId(memIdBufferLength);
    handShakeMsg.writeBytes(
        reinterpret_cast<int8_t*>(const_cast<char*>(memIdBuffer)),
        memIdBufferLength);
  }
  handShakeMsg.writeInt(static_cast<int32_t>(1));

  bool isDhOn = false;
  bool requireServerAuth = false;
  std::shared_ptr<Properties> credentials;
  std::shared_ptr<CacheableBytes> serverChallenge;

  // Write overrides (just conflation for now)
  handShakeMsg.write(getOverrides(&sysProp));

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
    handShakeMsg.write(
        static_cast<uint8_t>(SECURITY_MULTIUSER_NOTIFICATIONCHANNEL));
  } else if (isDhOn) {
    m_dh = new DiffieHellman();
    m_dh->initDhKeys(sysProp.getSecurityProperties());
    handShakeMsg.write(static_cast<uint8_t>(SECURITY_CREDENTIALS_DHENCRYPT));
  } else if (tmpIsSecurityOn) {
    handShakeMsg.write(static_cast<uint8_t>(SECURITY_CREDENTIALS_NORMAL));
  } else {
    handShakeMsg.write(static_cast<uint8_t>(SECURITY_CREDENTIALS_NONE));
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
        handShakeMsg.writeBoolean(requireServerAuth);
        LOGFINE(
            "HandShake: Server authentication using RSA signature %s required",
            requireServerAuth ? "is" : "not");

        // Send the symmetric key algorithm name string
        handShakeMsg.writeString(sysProp.securityClientDhAlgo());

        // Send the client's DH public key to the server
        auto dhPubKey = m_dh->getPublicKey();
        LOGDEBUG("DH pubkey send len is %d", dhPubKey->length());
        dhPubKey->toData(handShakeMsg);

        if (requireServerAuth) {
          char serverChallengeBytes[64] = {0};
          RandGen getrand;
          for (int pos = 0; pos < 64; pos++) {
            serverChallengeBytes[pos] = getrand(255);
          }
          serverChallenge = CacheableBytes::create(std::vector<int8_t>(
              serverChallengeBytes, serverChallengeBytes + 64));
          serverChallenge->toData(handShakeMsg);
        }
      } else {                       // if isDhOn
        if (isClientNotification) {  //:only for backward connection
          credentials->toData(handShakeMsg);
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

  size_t msgLength;
  auto data = reinterpret_cast<char*>(
      const_cast<uint8_t*>(handShakeMsg.getBuffer(&msgLength)));
  LOGFINE("Attempting handshake with endpoint %s for %s%s connection", endpoint,
          isClientNotification ? (isSecondary ? "secondary " : "primary ") : "",
          isClientNotification ? "subscription" : "client");
  ConnErrType error = sendData(data, msgLength, connectTimeout, false);

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
        credentials->toData(cleartext);
      }
      challengeBytes->toData(cleartext);
      auto ciphertext = m_dh->encrypt(
          cleartext.getBuffer(), static_cast<int>(cleartext.getBufferLength()));

      auto sendCreds = cacheImpl->createDataOutput();
      ciphertext->toData(sendCreds);
      size_t credLength;
      auto credData = reinterpret_cast<char*>(
          const_cast<uint8_t*>(sendCreds.getBuffer(&credLength)));
      // send the encrypted bytes and check the response
      error = sendData(credData, credLength, connectTimeout, false);

      if (error == CONN_NOERR) {
        acceptanceCode = readHandshakeData(1, connectTimeout);
        LOGDEBUG("Handshake: Got acceptanceCode Finally %d", acceptanceCode[0]);
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
    auto dataInput = cacheImpl->createDataInput(
        reinterpret_cast<const uint8_t*>(queueSizeMsg.data()),
        queueSizeMsg.size());
    int32_t queueSize = 0;
    queueSize = dataInput.readInt32();
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
      // Read the DistributedMember object
      auto recvMsgLen = readHandshakeArraySize(connectTimeout);
      auto recvMessage = readHandshakeData(recvMsgLen, connectTimeout);
      // If the distributed member has not been set yet, set it.
      if (getEndpointObject()->getDistributedMemberID() == 0) {
        LOGDEBUG("Deserializing distributed member Id");
        auto dataInputForClient = cacheImpl->createDataInput(
            reinterpret_cast<const uint8_t*>(recvMessage.data()),
            recvMessage.size());
        auto member = std::dynamic_pointer_cast<ClientProxyMembershipID>(
            dataInputForClient.readObject());
        auto memId = cacheImpl->getMemberListForVersionStamp()->add(member);
        getEndpointObject()->setDistributedMemberID(memId);
        LOGDEBUG("Deserialized distributed member Id %d", memId);
      }
    }

    auto recvMsgLenBytes = readHandshakeData(2, connectTimeout);
    auto dataInput3 = m_connectionManager->getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(recvMsgLenBytes.data()),
        recvMsgLenBytes.size());
    uint16_t recvMsgLen2 = dataInput3.readInt16();
    auto recvMessage = readHandshakeData(recvMsgLen2, connectTimeout);

    if (!isClientNotification) {
      auto deltaEnabledMsg = readHandshakeData(1, connectTimeout);
      auto di = m_connectionManager->getCacheImpl()->createDataInput(
          reinterpret_cast<const uint8_t*>(deltaEnabledMsg.data()), 1);
      ThinClientBaseDM::setDeltaEnabledOnServer(di.readBoolean());
    }

    switch (acceptanceCode[0]) {
      case REPLY_OK:
      case SUCCESSFUL_SERVER_TO_CLIENT:
        LOGFINER("Handshake reply: %u,%u,%u", acceptanceCode[0],
                 serverQueueStatus[0], recvMsgLen2);
        if (isClientNotification) readHandshakeInstantiatorMsg(connectTimeout);
        break;
      case REPLY_AUTHENTICATION_FAILED: {
        AuthenticationFailedException ex(
            reinterpret_cast<char*>(recvMessage.data()));
        GF_SAFE_DELETE_CON(m_conn);
        throwException(ex);
      }
      case REPLY_AUTHENTICATION_REQUIRED: {
        AuthenticationRequiredException ex(
            reinterpret_cast<char*>(recvMessage.data()));
        GF_SAFE_DELETE_CON(m_conn);
        throwException(ex);
      }
      case REPLY_DUPLICATE_DURABLE_CLIENT: {
        DuplicateDurableClientException ex(
            reinterpret_cast<char*>(recvMessage.data()));
        GF_SAFE_DELETE_CON(m_conn);
        throwException(ex);
      }
      case REPLY_REFUSED:
      case REPLY_INVALID:
      case UNSUCCESSFUL_SERVER_TO_CLIENT: {
        LOGERROR("Handshake rejected by server[%s]: %s",
                 m_endpointObj->name().c_str(),
                 reinterpret_cast<char*>(recvMessage.data()));
        auto message = std::string("TcrConnection::TcrConnection: ") +
                       "Handshake rejected by server: " +
                       reinterpret_cast<char*>(recvMessage.data());
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
            reinterpret_cast<char*>(recvMessage.data());
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
                            systemProperties.sslTrustStore().c_str(),
                            systemProperties.sslKeyStore().c_str(),
                            systemProperties.sslKeystorePassword().c_str());
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
    char* buffer, size_t length, std::chrono::microseconds receiveTimeoutSec,
    bool checkConnected, bool isNotificationMessage) {
  std::chrono::microseconds defaultWaitSecs =
      isNotificationMessage ? std::chrono::seconds(1) : std::chrono::seconds(2);
  if (defaultWaitSecs > receiveTimeoutSec) defaultWaitSecs = receiveTimeoutSec;

  auto startLen = length;

  while (length > 0 && receiveTimeoutSec > std::chrono::microseconds::zero()) {
    if (checkConnected && !m_connected) {
      return CONN_IOERR;
    }
    if (receiveTimeoutSec < defaultWaitSecs) {
      defaultWaitSecs = receiveTimeoutSec;
    }
    auto readBytes = m_conn->receive(buffer, length, defaultWaitSecs);
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
    LOGDEBUG("TcrConnection::receiveData length = %zu defaultWaitSecs = %z",
             length, defaultWaitSecs.count());
    if (m_poolDM != nullptr) {
      LOGDEBUG("TcrConnection::receiveData readBytes = %zu", readBytes);
      m_poolDM->getStats().incReceivedBytes(static_cast<int64_t>(readBytes));
    }
    receiveTimeoutSec -= defaultWaitSecs;
    if ((length == startLen) && isNotificationMessage) {  // no data read
      break;
    }
  }
  //  Postconditions for checking bounds.
  return (length == 0 ? CONN_NOERR
                      : (length == startLen ? CONN_NODATA : CONN_TIMEOUT));
}

inline ConnErrType TcrConnection::sendData(
    const char* buffer, size_t length, std::chrono::microseconds sendTimeout,
    bool checkConnected) {
  std::chrono::microseconds dummy{0};
  return sendData(dummy, buffer, length, sendTimeout, checkConnected);
}

inline ConnErrType TcrConnection::sendData(
    std::chrono::microseconds& timeSpent, const char* buffer, size_t length,
    std::chrono::microseconds sendTimeout, bool checkConnected) {
  std::chrono::microseconds defaultWaitSecs = std::chrono::seconds(2);
  if (defaultWaitSecs > sendTimeout) defaultWaitSecs = sendTimeout;
  LOGDEBUG(
      "before send len %zu sendTimeoutSec = %z checkConnected = %d m_connected "
      "%d",
      length, sendTimeout.count(), checkConnected, m_connected);
  while (length > 0 && sendTimeout > std::chrono::microseconds::zero()) {
    if (checkConnected && !m_connected) {
      return CONN_IOERR;
    }
    if (sendTimeout < defaultWaitSecs) {
      defaultWaitSecs = sendTimeout;
    }
    auto sentBytes = m_conn->send(buffer, length, defaultWaitSecs);

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

char* TcrConnection::sendRequest(const char* buffer, size_t len,
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
    const TcrMessage& request, size_t len, TcrMessageReply& reply,
    std::chrono::microseconds sendTimeoutSec,
    std::chrono::microseconds receiveTimeoutSec) {
  if (useReplyTimeout(request)) {
    receiveTimeoutSec = reply.getTimeout();
    sendTimeoutSec = reply.getTimeout();
  }

  receiveTimeoutSec -= sendWithTimeouts(request.getMsgData(), len,
                                        sendTimeoutSec, receiveTimeoutSec);

  // to help in decoding the reply based on what was the request type
  reply.setMessageTypeRequest(request.getMessageType());

  if (replyHasResult(request, reply)) {
    readMessageChunked(reply, receiveTimeoutSec, true);
  }
}

bool TcrConnection::useReplyTimeout(const TcrMessage& request) const {
  auto messageType = request.getMessageType();
  return ((messageType == TcrMessage::QUERY) ||
          (messageType == TcrMessage::QUERY_WITH_PARAMETERS) ||
          (messageType == TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE) ||
          (messageType == TcrMessage::GETDURABLECQS_MSG_TYPE) ||
          (messageType == TcrMessage::EXECUTE_FUNCTION) ||
          (messageType == TcrMessage::EXECUTE_REGION_FUNCTION) ||
          (messageType == TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP));
}

std::chrono::microseconds TcrConnection::sendWithTimeouts(
    const char* data, size_t len, std::chrono::microseconds sendTimeout,
    std::chrono::microseconds receiveTimeout) {
  std::chrono::microseconds timeSpent{0};
  send(timeSpent, data, len, sendTimeout, true);

  if (timeSpent >= receiveTimeout) {
    throwException(
        TimeoutException("TcrConnection::send: connection timed out"));
  }

  return timeSpent;
}

bool TcrConnection::replyHasResult(const TcrMessage& request,
                                   TcrMessageReply& reply) {
  auto hasResult = true;

  // no need of it now, this will not come here
  if (request.getMessageType() ==
      TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP) {
    ChunkedFunctionExecutionResponse* resultCollector =
        static_cast<ChunkedFunctionExecutionResponse*>(
            reply.getChunkedResultHandler());
    if (resultCollector->getResult() == false) {
      LOGDEBUG(
          "TcrConnection::sendRequestForChunkedResponse: function execution, "
          "no response desired");
      hasResult = false;
    }
  }

  return hasResult;
}

void TcrConnection::send(const char* buffer, size_t len,
                         std::chrono::microseconds sendTimeoutSec,
                         bool checkConnected) {
  std::chrono::microseconds dummy;
  send(dummy, buffer, len, sendTimeoutSec, checkConnected);
}

void TcrConnection::send(std::chrono::microseconds& timeSpent,
                         const char* buffer, size_t len,
                         std::chrono::microseconds sendTimeoutSec, bool) {
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
  return readMessage(recvLen, receiveTimeoutSec, false, opErr, true);
}

char* TcrConnection::readMessage(size_t* recvLen,
                                 std::chrono::microseconds receiveTimeoutSec,
                                 bool doHeaderTimeoutRetries,
                                 ConnErrType* opErr, bool isNotificationMessage,
                                 int32_t request) {
  char msg_header[HEADER_LENGTH];
  int32_t msgLen;
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
  // ignore msgType
  input.readInt32();
  msgLen = input.readInt32();
  //  check that message length is valid.
  if (!(msgLen > 0) && request == TcrMessage::GET_CLIENT_PR_METADATA) {
    char* fullMessage;
    *recvLen = HEADER_LENGTH + msgLen;
    _GEODE_NEW(fullMessage, char[HEADER_LENGTH + msgLen]);
    std::memcpy(fullMessage, msg_header, HEADER_LENGTH);
    return fullMessage;
    // exit(0);
  }

  // user has to delete this pointer
  char* fullMessage;
  *recvLen = HEADER_LENGTH + msgLen;
  _GEODE_NEW(fullMessage, char[HEADER_LENGTH + msgLen]);
  std::memcpy(fullMessage, msg_header, HEADER_LENGTH);

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
      Utils::convertBytesToString(fullMessage + HEADER_LENGTH, msgLen).c_str());

  return fullMessage;
}

void TcrConnection::readMessageChunked(TcrMessageReply& reply,
                                       std::chrono::microseconds receiveTimeout,
                                       bool doHeaderTimeoutRetries) {
  auto headerTimeout =
      calculateHeaderTimeout(receiveTimeout, doHeaderTimeoutRetries);

  LOGFINER(
      "TcrConnection::readMessageChunked: receiving reply from "
      "endpoint %s",
      m_endpoint);

  auto responseHeader = readResponseHeader(headerTimeout);

  reply.setMessageType(responseHeader.messageType);
  reply.setTransId(responseHeader.transactionId);

  // Initialize the chunk processing
  reply.startProcessChunk(m_chunksProcessSema);

  // indicate an end to chunk processing and wait for processing
  // to end even if reading the chunks fails in middle
  FinalizeProcessChunk endProcessChunk(reply,
                                       m_endpointObj->getDistributedMemberID());

  auto header = responseHeader.header;
  try {
    while (
        processChunk(reply, receiveTimeout, header.chunkLength, header.flags)) {
      header = readChunkHeader(headerTimeout);
    }
  } catch (const Exception&) {
    if (auto handler = reply.getChunkedResultHandler()) {
      if (auto ex = handler->getException()) {
        LOGDEBUG("Found existing exception ", ex->what());
        handler->clearException();
      }
    }
    throw;
  }

  LOGFINER(
      "TcrConnection::readMessageChunked: read full reply "
      "from endpoint %s",
      m_endpoint);
}

std::chrono::microseconds TcrConnection::calculateHeaderTimeout(
    std::chrono::microseconds receiveTimeout, bool retry) {
  auto headerTimeout = receiveTimeout;
  if (retry && receiveTimeout == DEFAULT_READ_TIMEOUT_SECS) {
    headerTimeout *= DEFAULT_TIMEOUT_RETRIES;
  }
  return headerTimeout;
}

chunkedResponseHeader TcrConnection::readResponseHeader(
    std::chrono::microseconds timeout) {
  uint8_t receiveBuffer[HEADER_LENGTH];
  chunkedResponseHeader header;

  auto error = receiveData(reinterpret_cast<char*>(receiveBuffer),
                           HEADER_LENGTH, timeout, true, false);
  if (error != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      throwException(TimeoutException(
          "TcrConnection::readResponseHeader: "
          "connection timed out while receiving message header"));
    } else {
      throwException(GeodeIOException(
          "TcrConnection::readResponseHeader: "
          "connection failure while receiving message header"));
    }
  }

  LOGDEBUG(
      "TcrConnection::readResponseHeader: received header from "
      "endpoint %s; bytes: %s",
      m_endpoint,
      Utils::convertBytesToString(receiveBuffer, HEADER_LENGTH).c_str());

  auto input = m_connectionManager->getCacheImpl()->createDataInput(
      receiveBuffer, HEADER_LENGTH);
  header.messageType = input.readInt32();
  header.numberOfParts = input.readInt32();
  header.transactionId = input.readInt32();
  header.header.chunkLength = input.readInt32();
  header.header.flags = input.read();
  LOGDEBUG(
      "TcrConnection::readResponseHeader: "
      "messageType=%" PRId32 ", numberOfParts=%" PRId32
      ", transactionId=%" PRId32 ", chunkLength=%" PRId32
      ", lastChunkAndSecurityFlags=0x%" PRIx8,
      header.messageType, header.numberOfParts, header.transactionId,
      header.header.chunkLength, header.header.flags);

  return header;
}  // namespace client

chunkHeader TcrConnection::readChunkHeader(std::chrono::microseconds timeout) {
  uint8_t receiveBuffer[CHUNK_HEADER_LENGTH];
  chunkHeader header;

  auto error = receiveData(reinterpret_cast<char*>(receiveBuffer),
                           CHUNK_HEADER_LENGTH, timeout, true, false);
  if (error != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      throwException(TimeoutException(
          "TcrConnection::readChunkHeader: "
          "connection timed out while receiving message header"));
    } else {
      throwException(GeodeIOException(
          "TcrConnection::readChunkHeader: "
          "connection failure while receiving message header"));
    }
  }

  LOGDEBUG(
      "TcrConnection::readChunkHeader: received header from "
      "endpoint %s; bytes: %s",
      m_endpoint,
      Utils::convertBytesToString(receiveBuffer, CHUNK_HEADER_LENGTH).c_str());

  auto input = m_connectionManager->getCacheImpl()->createDataInput(
      receiveBuffer, CHUNK_HEADER_LENGTH);
  header.chunkLength = input.readInt32();
  header.flags = input.read();
  LOGDEBUG(
      "TcrConnection::readChunkHeader: "
      ", chunkLen=%" PRId32 ", lastChunkAndSecurityFlags=0x%" PRIx8,
      header.chunkLength, header.flags);

  return header;
}

std::vector<uint8_t> TcrConnection::readChunkBody(
    std::chrono::microseconds timeout, int32_t chunkLength) {
  std::vector<uint8_t> chunkBody(chunkLength);
  auto error = receiveData(reinterpret_cast<char*>(chunkBody.data()),
                           chunkLength, timeout, true, false);
  if (error != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      throwException(
          TimeoutException("TcrConnection::readChunkBody: "
                           "connection timed out while receiving chunk body"));
    } else {
      throwException(
          GeodeIOException("TcrConnection::readChunkBody: "
                           "connection failure while receiving chunk body"));
    }
  }

  LOGDEBUG(
      "TcrConnection::readChunkBody: received chunk body from endpoint "
      "%s; bytes: %s",
      m_endpoint,
      Utils::convertBytesToString(chunkBody.data(), chunkLength).c_str());
  return chunkBody;
}

bool TcrConnection::processChunk(TcrMessageReply& reply,
                                 std::chrono::microseconds timeout,
                                 int32_t chunkLength,
                                 int8_t lastChunkAndSecurityFlags) {
  // NOTE: this buffer is allocated by readChunkBody, and reply.processChunk
  // takes ownership, so we don't delete it here on failure
  std::vector<uint8_t> chunkBody = readChunkBody(timeout, chunkLength);

  // Process the chunk; the actual processing is done by a separate thread
  // ThinClientBaseDM::m_chunkProcessor.
  reply.processChunk(chunkBody, chunkLength,
                     m_endpointObj->getDistributedMemberID(),
                     lastChunkAndSecurityFlags);
  // Return boolean indicating whether or not there are more chunks, i.e.
  // the *inverse* of the flag indicating this is the last chunk.  It's a
  // little confusing here, but makes calling code clearer.
  return (lastChunkAndSecurityFlags & LAST_CHUNK_MASK) ? false : true;
}

void TcrConnection::close() {
  TcrMessage* closeMsg = TcrMessage::getCloseConnMessage(
      m_poolDM->getConnectionManager().getCacheImpl());
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
    return std::vector<int8_t>(recvMessage, recvMessage + 1);
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
  } else {
    return CacheableBytes::create(
        std::vector<int8_t>(recvMessage, recvMessage + msgLength));
  }
}

std::shared_ptr<CacheableBytes> TcrConnection::readHandshakeByteArray(
    std::chrono::microseconds connectTimeout) {
  uint32_t arraySize = readHandshakeArraySize(connectTimeout);
  return readHandshakeRawData(arraySize, connectTimeout);
}

// read a byte array
int32_t TcrConnection::readHandshakeArraySize(
    std::chrono::microseconds connectTimeout) {
  auto arrayLenHeader = readHandshakeData(1, connectTimeout);

  int32_t arrayLength = static_cast<uint8_t>(arrayLenHeader[0]);
  if (static_cast<int8_t>(arrayLenHeader[0]) == -2) {
    auto arrayLengthBytes = readHandshakeData(2, connectTimeout);
    auto dataInput2 = m_connectionManager->getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(arrayLengthBytes.data()),
        arrayLengthBytes.size());
    arrayLength = dataInput2.readInt16();
  } else if (static_cast<int8_t>(arrayLenHeader[0]) == -3) {
    auto arrayLengthBytes = readHandshakeData(4, connectTimeout);
    auto dataInput2 = m_connectionManager->getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(arrayLengthBytes.data()),
        arrayLengthBytes.size());
    arrayLength = dataInput2.readInt32();
  }

  return arrayLength;
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

  auto di =
      m_connectionManager->getCacheImpl()->createDataInput(recvMessage, 4);
  int32_t val = di.readInt32();

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
  switch (static_cast<DSCode>(cstypeid)) {
    case DSCode::CacheableNullString: {
      return nullptr;
    }
    case DSCode::CacheableASCIIString: {
      auto lenBytes = readHandshakeData(2, connectTimeout);
      auto lenDI = m_connectionManager->getCacheImpl()->createDataInput(
          reinterpret_cast<const uint8_t*>(lenBytes.data()), lenBytes.size());
      length = lenDI.readInt16();

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

  return (clock::now() - m_creationTime) > expiryTime;
}

bool TcrConnection::isIdle(const std::chrono::milliseconds& idleTime) {
  if (idleTime <= std::chrono::milliseconds::zero()) {
    return false;
  }

  return (clock::now() - m_lastAccessed) > idleTime;
}

void TcrConnection::touch() { m_lastAccessed = clock::now(); }

TcrConnection::time_point TcrConnection::getLastAccessed() {
  return m_lastAccessed;
}

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
  m_creationTime = clock::now();
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
      if (m_isUsed.compare_exchange_strong(currentValue,
                                           2 /*for transaction*/)) {
        return true;
      }
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
