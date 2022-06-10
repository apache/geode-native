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

#include <geode/AuthInitialize.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "ClientProxyMembershipID.hpp"
#include "Connector.hpp"
#include "DistributedSystemImpl.hpp"
#include "FunctionMacros.hpp"
#include "TcpConn.hpp"
#include "TcpSslConn.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrEndpoint.hpp"
#include "ThinClientPoolHADM.hpp"
#include "ThinClientRegion.hpp"
#include "Utils.hpp"
#include "Version.hpp"

INIT_GNFN("TcrConnection")

#define throwException(ex)                            \
  do {                                                \
    LOGFINEST(ex.getName() + ": " + ex.getMessage()); \
    throw ex;                                         \
  } while (0)

namespace {
bool useReplyTimeout(const apache::geode::client::TcrMessage& request) {
  switch (request.getMessageType()) {
    case apache::geode::client::TcrMessage::QUERY:
    case apache::geode::client::TcrMessage::QUERY_WITH_PARAMETERS:
    case apache::geode::client::TcrMessage::EXECUTECQ_WITH_IR_MSG_TYPE:
    case apache::geode::client::TcrMessage::GETDURABLECQS_MSG_TYPE:
    case apache::geode::client::TcrMessage::EXECUTE_FUNCTION:
    case apache::geode::client::TcrMessage::EXECUTE_REGION_FUNCTION:
    case apache::geode::client::TcrMessage::EXECUTE_REGION_FUNCTION_SINGLE_HOP:
      return true;
    default:
      break;
  }

  return false;
}

int expiryTimeVariancePercentage() {
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> distribution(-9, 9);
  return distribution(generator);
}

const int HEADER_LENGTH = 17;
const int CHUNK_HEADER_LENGTH = 5;
const int8_t LAST_CHUNK_MASK = 0x1;
const int64_t INITIAL_CONNECTION_ID = 26739;

struct FinalizeProcessChunk {
 private:
  apache::geode::client::TcrMessage& reply_;
  uint16_t endpointMemId_;

 public:
  FinalizeProcessChunk(apache::geode::client::TcrMessageReply& reply,
                       uint16_t endpointMemId)
      : reply_(reply), endpointMemId_(endpointMemId) {}
  ~FinalizeProcessChunk() noexcept(false) {
    // Enqueue a nullptr chunk indicating a wait for processing to complete.
    reply_.processChunk(std::vector<uint8_t>(), 0, endpointMemId_);
  }
};
}  // namespace

namespace apache {
namespace geode {
namespace client {

TcrConnection::TcrConnection(const TcrConnectionManager& connectionManager)
    : expiryTimeVariancePercentage_{expiryTimeVariancePercentage()},
      connectionId(0),
      connectionManager_(connectionManager),
      hasServerQueue_(NON_REDUNDANT_SERVER),
      queueSize_(0),
      port_(0),
      chunks_process_semaphore_(0),
      isBeingUsed_(false),
      isUsed_(0),
      poolDM_(nullptr) {}

bool TcrConnection::initTcrConnection(
    std::shared_ptr<TcrEndpoint> endpointObj,
    synchronized_set<std::unordered_set<uint16_t>>& ports,
    bool isClientNotification, bool isSecondary,
    std::chrono::microseconds connectTimeout) {
  endpointObj_ = endpointObj;
  poolDM_ = dynamic_cast<ThinClientPoolDM*>(endpointObj_->getPoolHADM());
  hasServerQueue_ = NON_REDUNDANT_SERVER;
  queueSize_ = 0;
  lastAccessed_ = creationTime_ = std::chrono::steady_clock::now();
  connectionId = INITIAL_CONNECTION_ID;
  auto cacheImpl = poolDM_->getConnectionManager().getCacheImpl();
  const auto& distributedSystem = cacheImpl->getDistributedSystem();
  const auto& sysProp = distributedSystem.getSystemProperties();

  bool isPool = false;
  isBeingUsed_ = false;
  // Precondition:
  // 1. isSecondary ==> isClientNotification

  // Create TcpConn object which manages a socket connection with the endpoint.
  if (endpointObj && endpointObj->getPoolHADM()) {
    createConnection(endpointObj_->name().c_str(), connectTimeout,
                     static_cast<int32_t>(
                         endpointObj->getPoolHADM()->getSocketBufferSize()));
    isPool = true;
  } else {
    createConnection(endpointObj_->name().c_str(), connectTimeout,
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

  Version::write(handShakeMsg, Version::current());
  LOGFINE("Client version ordinal is %d", Version::current().getOrdinal());

  handShakeMsg.write(static_cast<int8_t>(REPLY_OK));

  // Send byte REPLY_OK = (byte)58;
  if (!isClientNotification) {
    port_ = conn_->getPort();
    ports.insert(port_);
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
  handShakeMsg.write(static_cast<int8_t>(DSFid::ClientProxyMembershipId));
  if (endpointObj->getPoolHADM()) {
    ClientProxyMembershipID* memId =
        endpointObj->getPoolHADM()->getMembershipId();
    const auto& dsMemberId = memId->getDSMemberId();
    handShakeMsg.writeBytes(
        reinterpret_cast<const uint8_t*>(dsMemberId.c_str()),
        static_cast<int32_t>(dsMemberId.size()));
  } else {
    // Add 3 durable Subcription properties to ClientProxyMembershipID
    auto&& durableId = sysProp.durableClientId();
    auto&& durableTimeOut = sysProp.durableTimeout();

    // Write ClientProxyMembershipID serialized object.
    auto memId = cacheImpl->getClientProxyMembershipIDFactory().create(
        durableId.c_str(), durableTimeOut);
    const auto& dsMemberId = memId->getDSMemberId();
    handShakeMsg.writeBytes(
        reinterpret_cast<const uint8_t*>(dsMemberId.c_str()),
        static_cast<int32_t>(dsMemberId.size()));
  }
  handShakeMsg.writeInt(static_cast<int32_t>(1));

  bool requireServerAuth = false;
  std::shared_ptr<Properties> credentials;
  std::shared_ptr<CacheableBytes> serverChallenge;

  // Write overrides (just conflation for now)
  handShakeMsg.write(getOverrides(&sysProp));

  bool tmpIsSecurityOn = nullptr != cacheImpl->getAuthInitialize();

  if (endpointObj_) {
    tmpIsSecurityOn = tmpIsSecurityOn || endpointObj_->isMultiUserMode();
  }

  LOGDEBUG(
      "TcrConnection tmpIsSecurityOn = %d isNotificationChannel = "
      "%d ",
      tmpIsSecurityOn, isNotificationChannel);
  bool doIneedToSendCreds = true;
  if (isNotificationChannel && endpointObj_ &&
      this->endpointObj_->isMultiUserMode()) {
    tmpIsSecurityOn = false;
    doIneedToSendCreds = false;
  }

  if (isNotificationChannel && !doIneedToSendCreds) {
    handShakeMsg.write(
        static_cast<uint8_t>(SECURITY_MULTIUSER_NOTIFICATIONCHANNEL));
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
              authInitialize->getCredentials(tmpSecurityProperties,
                                             endpointObj_->name().c_str());
          LOGFINER("TcrConnection: after getCredentials ");
          credentials = tmpAuthIniSecurityProperties;
        }
      }
      if (isClientNotification) {
        credentials->toData(handShakeMsg);
      }
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
  LOGFINE("Attempting handshake with endpoint %s for %s%s connection",
          endpointObj->name().c_str(),
          isClientNotification ? (isSecondary ? "secondary " : "primary ") : "",
          isClientNotification ? "subscription" : "client");
  LOGDEBUG("%s(%p): Handshake bytes: (%d): %s", __GNFN__, this, msgLength,
           Utils::convertBytesToString(data, msgLength).c_str());

  ConnErrType error = sendData(data, msgLength, connectTimeout);

  if (error == CONN_NOERR) {
    std::vector<int8_t> recdBytes;

    auto acceptanceCode = readHandshakeData(1, connectTimeout);
    recdBytes.push_back(acceptanceCode[0]);

    if (acceptanceCode[0] == REPLY_SSL_ENABLED && !sysProp.sslEnabled()) {
      LOGERROR("SSL is enabled on server, enable SSL in client as well");
      AuthenticationRequiredException ex(
          "SSL is enabled on server, enable SSL in client as well");
      conn_.reset();
      throwException(ex);
    }

    auto serverQueueStatus = readHandshakeData(1, connectTimeout);
    recdBytes.push_back(serverQueueStatus[0]);

    //  TESTING: Durable clients - set server queue status.
    // 0 - Non-Redundant , 1- Redundant , 2- Primary
    if (serverQueueStatus[0] == 1) {
      hasServerQueue_ = REDUNDANT_SERVER;
    } else if (serverQueueStatus[0] == 2) {
      hasServerQueue_ = PRIMARY_SERVER;
    } else {
      hasServerQueue_ = NON_REDUNDANT_SERVER;
    }
    auto queueSizeMsg = readHandshakeData(4, connectTimeout);
    recdBytes.insert(std::end(recdBytes), std::begin(queueSizeMsg),
                     std::end(queueSizeMsg));
    queueSize_ = static_cast<int32_t>(queueSizeMsg[0]) << 24 |
                 static_cast<int32_t>(queueSizeMsg[1]) << 16 |
                 static_cast<int32_t>(queueSizeMsg[2]) << 8 |
                 static_cast<int32_t>(queueSizeMsg[3]);
    queueSize_ = queueSize_ > 0 ? queueSize_ : 0;

    endpointObj_->setServerQueueStatus(hasServerQueue_, queueSize_);

    ////////////////////////// Set Pool Specific Q Size when
    ///////////////////////////////////
    ////////////////////////// 1. ServerQStatus is Primary or
    ///////////////////////////////////
    ////////////////////////// 2. ServerQStatus is Non-Redundant but
    ///////////////////////////////////
    ////////////////////////// 3. ONLY when handshake is for subscription
    ///////////////////////////////////
    if ((hasServerQueue_ == PRIMARY_SERVER ||
         hasServerQueue_ == NON_REDUNDANT_SERVER) &&
        isClientNotification) {
      poolDM_->setPrimaryServerQueueSize(queueSize_);
    }

    if (!isClientNotification) {
      // Read the DistributedMember object
      auto recvMsgLen = readHandshakeArraySize(connectTimeout);
      recdBytes.push_back((recvMsgLen & 0xff000000) >> 24);
      recdBytes.push_back((recvMsgLen & 0x00ff0000) >> 16);
      recdBytes.push_back((recvMsgLen & 0x0000ff00) >> 8);
      recdBytes.push_back(recvMsgLen & 0x000000ff);

      auto recvMessage = readHandshakeData(recvMsgLen, connectTimeout);
      recdBytes.insert(std::end(recdBytes), std::begin(recvMessage),
                       std::end(recvMessage));

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
    uint16_t recvMsgLen2 = static_cast<int16_t>(recvMsgLenBytes[0]) << 8 |
                           static_cast<int16_t>(recvMsgLenBytes[1]);
    recdBytes.insert(std::end(recdBytes), std::begin(recvMsgLenBytes),
                     std::end(recvMsgLenBytes));

    auto recvMessage = readHandshakeData(recvMsgLen2, connectTimeout);
    recdBytes.insert(std::end(recdBytes), std::begin(recvMessage),
                     std::end(recvMessage));

    if (!isClientNotification) {
      auto deltaEnabledMsg = readHandshakeData(1, connectTimeout);
      recdBytes.push_back(deltaEnabledMsg[0]);
      ThinClientBaseDM::setDeltaEnabledOnServer(deltaEnabledMsg[0] ? true
                                                                   : false);
    }

    LOGDEBUG(
        "%s(%p): isClientNotification=%s, Handshake response bytes: (%d) %s",
        __GNFN__, this, isClientNotification ? "true" : "false",
        recdBytes.size(),
        Utils::convertBytesToString(recdBytes.data(), recdBytes.size())
            .c_str());

    switch (acceptanceCode[0]) {
      case REPLY_OK:
      case SUCCESSFUL_SERVER_TO_CLIENT:
        LOGFINER("Handshake reply: %u,%u,%u", acceptanceCode[0],
                 serverQueueStatus[0], recvMsgLen2);
        if (isClientNotification) {
          readHandshakeInstantiatorMsg(connectTimeout);
        }
        break;
      case REPLY_AUTHENTICATION_FAILED: {
        AuthenticationFailedException ex(
            reinterpret_cast<char*>(recvMessage.data()));
        conn_.reset();
        throwException(ex);
      }
      case REPLY_AUTHENTICATION_REQUIRED: {
        AuthenticationRequiredException ex(
            reinterpret_cast<char*>(recvMessage.data()));
        conn_.reset();
        throwException(ex);
      }
      case REPLY_DUPLICATE_DURABLE_CLIENT: {
        DuplicateDurableClientException ex(
            reinterpret_cast<char*>(recvMessage.data()));
        conn_.reset();
        throwException(ex);
      }
      case REPLY_REFUSED:
      case REPLY_INVALID:
      case UNSUCCESSFUL_SERVER_TO_CLIENT: {
        LOGERROR("Handshake rejected by server[%s]: %s",
                 endpointObj_->name().c_str(),
                 reinterpret_cast<char*>(recvMessage.data()));
        auto message = std::string("TcrConnection::TcrConnection: ") +
                       "Handshake rejected by server: " +
                       reinterpret_cast<char*>(recvMessage.data());
        CacheServerException ex(message);
        conn_.reset();
        throw ex;
      }
      default: {
        LOGERROR(
            "Unknown error[%d] received from server [%s] in handshake: "
            "%s",
            acceptanceCode[0], endpointObj_->name().c_str(),
            recvMessage.data());
        auto message =
            std::string("TcrConnection::TcrConnection: Unknown error") +
            " received from server in handshake: " +
            reinterpret_cast<char*>(recvMessage.data());
        MessageException ex(message);
        conn_.reset();
        throw ex;
      }
    }

  } else {
    conn_.reset();
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

  if (this->endpointObj_ && !isNotificationChannel && tmpIsSecurityOn &&
      (!isPool || !this->endpointObj_->isMultiUserMode())) {
    // this->endpointObj_->authenticateEndpoint(this);
    return true;
  }

  return false;
}

void TcrConnection::createConnection(const std::string& address,
                                     std::chrono::microseconds connectTimeout,
                                     int32_t maxBuffSizePool) {
  Connector* socket = nullptr;
  auto& systemProperties = connectionManager_.getCacheImpl()
                               ->getDistributedSystem()
                               .getSystemProperties();

  if (systemProperties.sslEnabled()) {
    const auto& sniHostname = poolDM_->getSniProxyHost();
    if (sniHostname.empty()) {
      conn_.reset(new TcpSslConn(address, connectTimeout, maxBuffSizePool,
                                 systemProperties.sslTrustStore(),
                                 systemProperties.sslKeyStore(),
                                 systemProperties.sslKeystorePassword()));
    } else {
      const auto sniPort = poolDM_->getSniProxyPort();
      conn_.reset(new TcpSslConn(
          address, connectTimeout, maxBuffSizePool, sniHostname, sniPort,
          systemProperties.sslTrustStore(), systemProperties.sslKeyStore(),
          systemProperties.sslKeystorePassword()));
    }
  } else {
    conn_.reset(new TcpConn(address, connectTimeout, maxBuffSizePool));
  }
}

ConnErrType TcrConnection::receiveData(
    char* buffer, const size_t length,
    const std::chrono::microseconds timeout) {
  try {
    const auto readBytes = conn_->receive(
        buffer, length,
        std::chrono::duration_cast<std::chrono::milliseconds>(timeout));

    poolDM_->getStats().incReceivedBytes(static_cast<int64_t>(readBytes));
  } catch (boost::system::system_error& ex) {
    switch (ex.code().value()) {
      case boost::asio::error::eof:
        return CONN_NODATA;
      case boost::asio::error::operation_aborted:
        return CONN_TIMEOUT;
      default:
        break;
    }
    return CONN_IOERR;
  }

  return CONN_NOERR;
}

ConnErrType TcrConnection::sendData(const char* buffer, size_t length,
                                    std::chrono::microseconds timeout) {
  try {
    conn_->send(buffer, length,
                std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
  } catch (boost::system::system_error& ex) {
    switch (ex.code().value()) {
      case boost::asio::error::operation_aborted:
        return CONN_TIMEOUT;
      default:
        break;
    }
    return CONN_IOERR;
  }

  return CONN_NOERR;
}

char* TcrConnection::sendRequest(const char* buffer, size_t len,
                                 size_t* recvLen,
                                 std::chrono::microseconds sendTimeoutSec,
                                 std::chrono::microseconds receiveTimeoutSec,
                                 int32_t request) {
  const auto start = std::chrono::system_clock::now();
  send(buffer, len, sendTimeoutSec);
  const auto timeSpent = start - std::chrono::system_clock::now();

  if (timeSpent >= receiveTimeoutSec) {
    throwException(
        TimeoutException("TcrConnection::send: connection timed out"));
  }

  receiveTimeoutSec -=
      std::chrono::duration_cast<decltype(receiveTimeoutSec)>(timeSpent);
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

std::chrono::microseconds TcrConnection::sendWithTimeouts(
    const char* data, size_t len, std::chrono::microseconds sendTimeout,
    std::chrono::microseconds receiveTimeout) {
  const auto start = std::chrono::system_clock::now();
  send(data, len, sendTimeout, true);
  const auto timeSpent = start - std::chrono::system_clock::now();

  if (timeSpent >= receiveTimeout) {
    throwException(
        TimeoutException("TcrConnection::send: connection timed out"));
  }

  return std::chrono::duration_cast<std::chrono::microseconds>(timeSpent);
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
                         std::chrono::microseconds sendTimeoutSec, bool) {
  LOGDEBUG(
      "TcrConnection::send: [%p] sending request to endpoint %s; bytes: %s",
      this, endpointObj_->name().c_str(),
      Utils::convertBytesToString(buffer, len).c_str());

  switch (sendData(buffer, len, sendTimeoutSec)) {
    case CONN_NOERR:
      break;
    case CONN_TIMEOUT:
      throwException(
          TimeoutException("TcrConnection::send: connection timed out"));
    case CONN_NODATA:
    case CONN_IOERR:
    case CONN_OTHERERR:
      throwException(
          GeodeIOException("TcrConnection::send: connection failure"));
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
  if (doHeaderTimeoutRetries && receiveTimeoutSec == DEFAULT_READ_TIMEOUT) {
    headerTimeout = DEFAULT_READ_TIMEOUT * DEFAULT_TIMEOUT_RETRIES;
  }

  error = receiveData(msg_header, HEADER_LENGTH, headerTimeout);

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
      "TcrConnection::readMessage(%p): received header from endpoint %s; "
      "bytes: %s",
      this, endpointObj_->name().c_str(),
      Utils::convertBytesToString(msg_header, HEADER_LENGTH).c_str());

  auto input = connectionManager_.getCacheImpl()->createDataInput(
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
  error = receiveData(fullMessage + HEADER_LENGTH, msgLen, mesgBodyTimeout);
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
      endpointObj_->name().c_str(),
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
      endpointObj_->name().c_str());

  auto responseHeader = readResponseHeader(headerTimeout);

  reply.setMessageType(responseHeader.messageType);
  reply.setTransId(responseHeader.transactionId);

  // Initialize the chunk processing
  reply.startProcessChunk(chunks_process_semaphore_);

  // indicate an end to chunk processing and wait for processing
  // to end even if reading the chunks fails in middle
  FinalizeProcessChunk endProcessChunk(reply,
                                       endpointObj_->getDistributedMemberID());

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
      endpointObj_->name().c_str());
}

std::chrono::microseconds TcrConnection::calculateHeaderTimeout(
    std::chrono::microseconds receiveTimeout, bool retry) {
  auto headerTimeout = receiveTimeout;
  if (retry && receiveTimeout == DEFAULT_READ_TIMEOUT) {
    headerTimeout *= DEFAULT_TIMEOUT_RETRIES;
  }
  return headerTimeout;
}

chunkedResponseHeader TcrConnection::readResponseHeader(
    std::chrono::microseconds timeout) {
  uint8_t receiveBuffer[HEADER_LENGTH];
  chunkedResponseHeader header;

  auto error = receiveData(reinterpret_cast<char*>(receiveBuffer),
                           HEADER_LENGTH, timeout);
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
      "TcrConnection::readResponseHeader(%p): received header from "
      "endpoint %s; bytes: %s",
      this, endpointObj_->name().c_str(),
      Utils::convertBytesToString(receiveBuffer, HEADER_LENGTH).c_str());

  auto input = connectionManager_.getCacheImpl()->createDataInput(
      receiveBuffer, HEADER_LENGTH);
  header.messageType = input.readInt32();
  header.numberOfParts = input.readInt32();
  header.transactionId = input.readInt32();
  header.header.chunkLength = input.readInt32();
  header.header.flags = input.read();
  LOGDEBUG(
      "TcrConnection::readResponseHeader(%p): "
      "messageType=%" PRId32 ", numberOfParts=%" PRId32
      ", transactionId=%" PRId32 ", chunkLength=%" PRId32
      ", lastChunkAndSecurityFlags=0x%" PRIx8,
      this, header.messageType, header.numberOfParts, header.transactionId,
      header.header.chunkLength, header.header.flags);

  return header;
}  // namespace client

chunkHeader TcrConnection::readChunkHeader(std::chrono::microseconds timeout) {
  uint8_t receiveBuffer[CHUNK_HEADER_LENGTH];
  chunkHeader header;

  auto error = receiveData(reinterpret_cast<char*>(receiveBuffer),
                           CHUNK_HEADER_LENGTH, timeout);
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
      endpointObj_->name().c_str(),
      Utils::convertBytesToString(receiveBuffer, CHUNK_HEADER_LENGTH).c_str());

  auto input = connectionManager_.getCacheImpl()->createDataInput(
      receiveBuffer, CHUNK_HEADER_LENGTH);
  header.chunkLength = input.readInt32();
  header.flags = input.read();
  LOGDEBUG(
      "TcrConnection::readChunkHeader(%p): "
      ", chunkLen=%" PRId32 ", lastChunkAndSecurityFlags=0x%" PRIx8,
      this, header.chunkLength, header.flags);

  return header;
}

std::vector<uint8_t> TcrConnection::readChunkBody(
    std::chrono::microseconds timeout, int32_t chunkLength) {
  std::vector<uint8_t> chunkBody(chunkLength);
  auto error = receiveData(reinterpret_cast<char*>(chunkBody.data()),
                           chunkLength, timeout);
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
      "TcrConnection::readChunkBody(%p): received chunk body from endpoint "
      "%s; bytes: %s",
      this, endpointObj_->name().c_str(),
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
  // ThinClientBaseDM::chunkProcessor_.
  reply.processChunk(chunkBody, chunkLength,
                     endpointObj_->getDistributedMemberID(),
                     lastChunkAndSecurityFlags);
  // Return boolean indicating whether or not there are more chunks, i.e.
  // the *inverse* of the flag indicating this is the last chunk.  It's a
  // little confusing here, but makes calling code clearer.
  return (lastChunkAndSecurityFlags & LAST_CHUNK_MASK) ? false : true;
}

void TcrConnection::close() {
  auto cache = poolDM_->getConnectionManager().getCacheImpl();
  TcrMessageCloseConnection closeMsg{
      std::unique_ptr<DataOutput>(
          new DataOutput(cache->createDataOutput(poolDM_))),
      cache->isKeepAlive() || poolDM_->isKeepAlive()};

  try {
    if (!TcrConnectionManager::TEST_DURABLE_CLIENT_CRASH &&
        !connectionManager_.isNetDown()) {
      send(closeMsg.getMsgData(), closeMsg.getMsgLength(),
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
  if (msgLength <= 0) {
    return std::vector<int8_t>();
  }

  std::vector<int8_t> message(msgLength);

  if ((error = receiveData(reinterpret_cast<char*>(message.data()), msgLength,
                           connectTimeout)) != CONN_NOERR) {
    conn_.reset();
    if (error & CONN_TIMEOUT) {
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake"));
    } else {
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure"));
    }
  } else {
    return message;
  }
}

// read a byte array
int32_t TcrConnection::readHandshakeArraySize(
    std::chrono::microseconds connectTimeout) {
  auto arrayLenHeader = readHandshakeData(1, connectTimeout);

  int32_t arrayLength = static_cast<uint8_t>(arrayLenHeader[0]);
  if (static_cast<int8_t>(arrayLenHeader[0]) == -2) {
    auto arrayLengthBytes = readHandshakeData(2, connectTimeout);
    auto dataInput2 = connectionManager_.getCacheImpl()->createDataInput(
        reinterpret_cast<const uint8_t*>(arrayLengthBytes.data()),
        arrayLengthBytes.size());
    arrayLength = dataInput2.readInt16();
  } else if (static_cast<int8_t>(arrayLenHeader[0]) == -3) {
    auto arrayLengthBytes = readHandshakeData(4, connectTimeout);
    auto dataInput2 = connectionManager_.getCacheImpl()->createDataInput(
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

  // Geode 1.5+ - Ignore client ping task interval
  readHandShakeBytes(4, connectTimeout);
}
void TcrConnection::readHandShakeBytes(
    int numberOfBytes, std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;
  uint8_t* recvMessage;
  _GEODE_NEW(recvMessage, uint8_t[numberOfBytes]);

  if ((error = receiveData(reinterpret_cast<char*>(recvMessage), numberOfBytes,
                           connectTimeout)) != CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      conn_.reset();
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake"));
    } else {
      _GEODE_SAFE_DELETE_ARRAY(recvMessage);
      conn_.reset();
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure"));
    }
  }

  _GEODE_SAFE_DELETE_ARRAY(recvMessage);
}

std::shared_ptr<CacheableString> TcrConnection::readHandshakeString(
    std::chrono::microseconds connectTimeout) {
  ConnErrType error = CONN_NOERR;

  char cstypeid;
  if (receiveData(&cstypeid, 1, connectTimeout) != CONN_NOERR) {
    conn_.reset();
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
      auto lenDI = connectionManager_.getCacheImpl()->createDataInput(
          reinterpret_cast<const uint8_t*>(lenBytes.data()), lenBytes.size());
      length = lenDI.readInt16();

      break;
    }
    case DSCode::FixedIDDefault:
    case DSCode::FixedIDByte:
    case DSCode::FixedIDInt:
    case DSCode::FixedIDNone:
    case DSCode::FixedIDShort:
    case DSCode::CacheableLinkedList:
    case DSCode::Properties:
    case DSCode::PdxType:
    case DSCode::BooleanArray:
    case DSCode::CharArray:
    case DSCode::NullObj:
    case DSCode::CacheableString:
    case DSCode::Class:
    case DSCode::JavaSerializable:
    case DSCode::DataSerializable:
    case DSCode::CacheableBytes:
    case DSCode::CacheableInt16Array:
    case DSCode::CacheableInt32Array:
    case DSCode::CacheableInt64Array:
    case DSCode::CacheableFloatArray:
    case DSCode::CacheableDoubleArray:
    case DSCode::CacheableObjectArray:
    case DSCode::CacheableBoolean:
    case DSCode::CacheableCharacter:
    case DSCode::CacheableByte:
    case DSCode::CacheableInt16:
    case DSCode::CacheableInt32:
    case DSCode::CacheableInt64:
    case DSCode::CacheableFloat:
    case DSCode::CacheableDouble:
    case DSCode::CacheableDate:
    case DSCode::CacheableFileName:
    case DSCode::CacheableStringArray:
    case DSCode::CacheableArrayList:
    case DSCode::CacheableHashSet:
    case DSCode::CacheableHashMap:
    case DSCode::CacheableTimeUnit:
    case DSCode::CacheableHashTable:
    case DSCode::CacheableVector:
    case DSCode::CacheableIdentityHashMap:
    case DSCode::CacheableLinkedHashSet:
    case DSCode::CacheableStack:
    case DSCode::CacheableASCIIStringHuge:
    case DSCode::CacheableStringHuge:
    case DSCode::CacheableUserData:
    case DSCode::CacheableUserData2:
    case DSCode::CacheableUserData4:
    case DSCode::PDX:
    case DSCode::PDX_ENUM: {
      conn_.reset();
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure: Unexpected string type ID"));
    }
  }

  LOGDEBUG(" Received string len %d", length);

  if (length == 0) {
    return nullptr;
  }

  std::vector<char> recvMessage(length + 1);
  recvMessage[length] = '\0';

  if ((error = receiveData(recvMessage.data(), length, connectTimeout)) !=
      CONN_NOERR) {
    if (error & CONN_TIMEOUT) {
      conn_.reset();
      LOGFINE("Timeout receiving string data");
      throwException(
          TimeoutException("TcrConnection::TcrConnection: "
                           "Timeout in handshake reading string bytes"));
    } else {
      conn_.reset();
      LOGFINE("IO error receiving string data");
      throwException(
          GeodeIOException("TcrConnection::TcrConnection: "
                           "Handshake failure reading string bytes"));
    }
  } else {
    LOGDEBUG(" Received string data [%s]", recvMessage.data());
    auto retval =
        CacheableString::create(std::string(recvMessage.data(), length));
    return retval;
  }
}

bool TcrConnection::hasExpired(const std::chrono::milliseconds& expiryTime) {
  if (expiryTime <= std::chrono::milliseconds::zero()) {
    return false;
  }
  auto variadicExpiryTime =
      expiryTime + (expiryTime * expiryTimeVariancePercentage_) / 100;
  return (std::chrono::steady_clock::now() - creationTime_) >
         variadicExpiryTime;
}

bool TcrConnection::isIdle(const std::chrono::milliseconds& idleTime) {
  if (idleTime <= std::chrono::milliseconds::zero()) {
    return false;
  }

  return (std::chrono::steady_clock::now() - lastAccessed_) > idleTime;
}

void TcrConnection::touch() {
  lastAccessed_ = std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point TcrConnection::getLastAccessed() {
  return lastAccessed_;
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
  creationTime_ = std::chrono::steady_clock::now();
  touch();
}

TcrConnection::~TcrConnection() {}

bool TcrConnection::setAndGetBeingUsed(volatile bool isBeingUsed,
                                       bool forTransaction) {
  uint32_t currentValue = 0;

  if (!forTransaction) {
    if (isBeingUsed) {
      if (isUsed_ == 1 || isUsed_ == 2) return false;
      if (isUsed_.compare_exchange_strong(currentValue, 1)) return true;
      return false;
    } else {
      isUsed_ = 0;
      return true;
    }
  } else {
    if (isBeingUsed) {
      if (isUsed_ == 1) {  // already used
        return false;
      }
      if (isUsed_ == 2) {  // transaction thread has set, reused it
        return true;
      }
      if (isUsed_.compare_exchange_strong(currentValue,
                                          2 /*for transaction*/)) {
        return true;
      }
      return false;
    } else {
      // isUsed_ = 0;//this will done by releasing the connection by
      // transaction at the end of transaction
      return true;
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
