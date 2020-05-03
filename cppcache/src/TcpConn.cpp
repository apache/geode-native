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

#include "TcpConn.hpp"

#include <thread>

#include <ace/SOCK_Connector.h>
#include <boost/interprocess/mapped_region.hpp>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/chrono/duration.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

const size_t TcpConn::kChunkSize = TcpConn::getDefaultChunkSize();

void TcpConn::clearNagle(ACE_HANDLE sock) {
  int32_t val = 1;

  if (0 != ACE_OS::setsockopt(sock, IPPROTO_TCP, 1,
                              reinterpret_cast<const char*>(&val),
                              sizeof(val))) {
    int32_t lastError = ACE_OS::last_error();
    LOGERROR("Failed to set TCP_NODELAY on socket. Errno: %d: %s", lastError,
             ACE_OS::strerror(lastError));
  }
}

int32_t TcpConn::maxSize(ACE_HANDLE sock, int32_t flag, int32_t size) {
  int32_t val = 0;

  int32_t inc = 32120;
  val = size - (3 * inc);
  if (val < 0) val = 0;
  if (size == 0) size = maxBuffSizePool_;
  int32_t red = 0;
  int32_t lastRed = -1;
  while (lastRed != red) {
    lastRed = red;
    val += inc;
    if (0 != ACE_OS::setsockopt(sock, SOL_SOCKET, flag,
                                reinterpret_cast<const char*>(&val),
                                sizeof(val))) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR("Failed to set socket options. Errno: %d : %s ", lastError,
               ACE_OS::strerror(lastError));
    }
    int plen = sizeof(val);
    if (0 != ACE_OS::getsockopt(sock, SOL_SOCKET, flag,
                                reinterpret_cast<char*>(&val), &plen)) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR(
          "Failed to get buffer size for flag %d on socket. Errno: %d : %s",
          flag, lastError, ACE_OS::strerror(lastError));
    }
#ifdef _LINUX
    val /= 2;
#endif
    if ((val >= maxBuffSizePool_) || (val >= size)) continue;
    red = val;
  }
  return val;
}

void TcpConn::createSocket(ACE_HANDLE sock) {
  LOGDEBUG("Creating plain socket stream");
  stream_ = std::unique_ptr<ACE_SOCK_Stream>(new ACE_SOCK_Stream(sock));
}

void TcpConn::init() {
#ifdef WITH_IPV6
  ACE_HANDLE sock = ACE_OS::socket(inetAddress_.get_type(), SOCK_STREAM, 0);
#else
  ACE_HANDLE sock = ACE_OS::socket(AF_INET, SOCK_STREAM, 0);
#endif
  if (sock == ACE_INVALID_HANDLE) {
    int32_t lastError = ACE_OS::last_error();
    LOGERROR("Failed to create socket. Errno: %d: %s", lastError,
             ACE_OS::strerror(lastError));
    char msg[256];
    std::snprintf(msg, 256, "TcpConn::connect failed with errno: %d: %s",
                  lastError, ACE_OS::strerror(lastError));
    throw GeodeIOException(msg);
  }

  clearNagle(sock);

  int32_t readSize = 0;
  int32_t writeSize = 0;
  int32_t originalReadSize = readSize;
  readSize = maxSize(sock, SO_SNDBUF, readSize);
  if (originalReadSize != readSize) {
    // This should get logged once at startup and again only if it changes
    LOGFINEST("Using socket send buffer size of %d.", readSize);
  }
  int32_t originalWriteSize = writeSize;
  writeSize = maxSize(sock, SO_RCVBUF, writeSize);
  if (originalWriteSize != writeSize) {
    // This should get logged once at startup and again only if it changes
    LOGFINEST("Using socket receive buffer size of %d.", writeSize);
  }

  createSocket(sock);

  connect();
}

TcpConn::TcpConn(const std::string& address,
                 std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool)
    : stream_(nullptr),
      maxBuffSizePool_(maxBuffSizePool),
      inetAddress_(address.c_str()),
      timeout_(waitSeconds) {}

TcpConn::TcpConn(const std::string& hostname, uint16_t port,
                 std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool)
    : stream_(nullptr),
      maxBuffSizePool_(maxBuffSizePool),
      inetAddress_(port, hostname.c_str()),
      timeout_(waitSeconds) {}

void TcpConn::connect() {
  using apache::geode::internal::chrono::duration::to_string;

  ACE_OS::signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe

  LOGFINER(std::string("Connecting plain socket stream to ") +
           inetAddress_.get_host_name() + ":" +
           std::to_string(inetAddress_.get_port_number()) + " waiting " +
           to_string(timeout_));

  const ACE_Time_Value aceTimeout(timeout_);
  const auto timeout =
      (timeout_ > std::chrono::microseconds::zero()) ? &aceTimeout : nullptr;
  if (ACE_SOCK_Connector{}.connect(*stream_, inetAddress_, timeout) == -1) {
    const auto lastError = ACE_OS::last_error();
    if (lastError == ETIME || lastError == ETIMEDOUT) {
      throw TimeoutException(
          "TcpConn::connect Attempt to connect timed out after " +
          to_string(timeout_) + ".");
    }
    close();
    throw GeodeIOException("TcpConn::connect failed with errno: " +
                           ACE_errno_to_string(lastError));
  }

  if (stream_->enable(ACE_NONBLOCK)) {
    LOGINFO("TcpConn::NONBLOCK: " + ACE_errno_to_string(ACE_OS::last_error()));
  }
}

void TcpConn::close() {
  if (stream_) {
    stream_.release()->close();
  }
}

size_t TcpConn::receive(char* buff, size_t len,
                        std::chrono::microseconds waitSeconds) {
  return socketOp(SOCK_READ, buff, len, waitSeconds);
}

size_t TcpConn::send(const char* buff, size_t len,
                     std::chrono::microseconds waitSeconds) {
  return socketOp(SOCK_WRITE, const_cast<char*>(buff), len, waitSeconds);
}

size_t TcpConn::socketOp(TcpConn::SockOp op, char* buff, size_t len,
                         std::chrono::microseconds waitDuration) {
  {
    ACE_Time_Value waitTime(waitDuration);
    auto endTime = std::chrono::steady_clock::now() + waitDuration;
    size_t readLen = 0;
    ssize_t retVal;
    bool errnoSet = false;

    size_t totalsend = 0;
    while (len > 0 && waitTime > ACE_Time_Value::zero) {
      size_t sendlen;
      if (len > kChunkSize) {
        sendlen = kChunkSize;
        len -= kChunkSize;
      } else {
        sendlen = len;
        len = 0;
      }
      do {
        retVal = doOperation(op, buff, sendlen, waitTime, readLen);
        sendlen -= readLen;
        totalsend += readLen;
        if (retVal < 0) {
          int32_t lastError = ACE_OS::last_error();
          if (lastError == EAGAIN) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
          } else {
            errnoSet = true;
            break;
          }
        } else if (retVal == 0 && readLen == 0) {
          ACE_OS::last_error(EPIPE);
          errnoSet = true;
          break;
        }

        buff += readLen;
        if (sendlen == 0) break;
        waitTime = endTime - std::chrono::steady_clock::now();
        if (waitTime <= ACE_Time_Value::zero) break;
      } while (sendlen > 0);
      if (errnoSet) break;
    }

    if (len > 0 && !errnoSet) {
      ACE_OS::last_error(ETIME);
    }

    return totalsend;
  }
}
ssize_t TcpConn::doOperation(const TcpConn::SockOp& op, void* buff,
                             size_t sendlen, ACE_Time_Value& waitTime,
                             size_t& readLen) const {
  if (op == SOCK_READ) {
    return stream_->recv_n(buff, sendlen, &waitTime, &readLen);
  } else {
    return stream_->send_n(buff, sendlen, &waitTime, &readLen);
  }
}

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() {
  ACE_INET_Addr localAddr;
  stream_->get_local_addr(localAddr);
  return localAddr.get_port_number();
}

size_t TcpConn::getDefaultChunkSize() {
  //
  auto pageSize = boost::interprocess::mapped_region::get_page_size();
  if (pageSize > 16000000) {
    return 16000000;
  } else if (pageSize > 0) {
    return pageSize + (16000000 / pageSize) * pageSize;
  }

  return 16000000;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
