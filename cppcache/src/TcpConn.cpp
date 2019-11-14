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

#include <chrono>
#include <memory>
#include <thread>

#include <ace/INET_Addr.h>
#include <ace/OS.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_IO.h>

#include <geode/SystemProperties.hpp>
#include <geode/internal/chrono/duration.hpp>

#include "CacheImpl.hpp"
#include "DistributedSystem.hpp"
#include "config.h"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

void TcpConn::clearNagle(ACE_HANDLE sock) {
  int32_t val = 1;

  if (0 != ACE_OS::setsockopt(sock, IPPROTO_TCP, 1,
                              reinterpret_cast<const char *>(&val),
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
  if (size == 0) size = m_maxBuffSizePool;
  int32_t red = 0;
  int32_t lastRed = -1;
  while (lastRed != red) {
    lastRed = red;
    val += inc;
    if (0 != ACE_OS::setsockopt(sock, SOL_SOCKET, flag,
                                reinterpret_cast<const char *>(&val),
                                sizeof(val))) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR("Failed to set socket options. Errno: %d : %s ", lastError,
               ACE_OS::strerror(lastError));
    }
    int plen = sizeof(val);
    if (0 != ACE_OS::getsockopt(sock, SOL_SOCKET, flag,
                                reinterpret_cast<char *>(&val), &plen)) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR(
          "Failed to get buffer size for flag %d on socket. Errno: %d : %s",
          flag, lastError, ACE_OS::strerror(lastError));
    }
#ifdef _LINUX
    val /= 2;
#endif
    if ((val >= m_maxBuffSizePool) || (val >= size)) continue;
    red = val;
  }
  return val;
}

void TcpConn::createSocket(ACE_HANDLE sock) {
  LOGDEBUG("Creating plain socket stream");
  m_io = new ACE_SOCK_Stream(sock);
  // m_io->enable(ACE_NONBLOCK);
}

void TcpConn::init() {
#ifdef WITH_IPV6
  ACE_HANDLE sock = ACE_OS::socket(m_addr.get_type(), SOCK_STREAM, 0);
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

TcpConn::TcpConn(const char *ipaddr, std::chrono::microseconds waitSeconds,
                 int32_t maxBuffSizePool)
    : m_io(nullptr),
      m_addr(ipaddr),
      m_waitMilliSeconds(waitSeconds),
      m_maxBuffSizePool(maxBuffSizePool),
      m_chunkSize(getDefaultChunkSize()) {}

TcpConn::TcpConn(const char *hostname, int32_t port,
                 std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool)
    : m_io(nullptr),
      m_addr(port, hostname),
      m_waitMilliSeconds(waitSeconds),
      m_maxBuffSizePool(maxBuffSizePool),
      m_chunkSize(getDefaultChunkSize()) {}

void TcpConn::listen(const char *hostname, int32_t port,
                     std::chrono::microseconds waitSeconds) {
  ACE_INET_Addr addr(port, hostname);
  listen(addr, waitSeconds);
}

void TcpConn::listen(const char *ipaddr,
                     std::chrono::microseconds waitSeconds) {
  ACE_INET_Addr addr(ipaddr);
  listen(addr, waitSeconds);
}

void TcpConn::listen(ACE_INET_Addr addr,
                     std::chrono::microseconds waitSeconds) {
  using apache::geode::internal::chrono::duration::to_string;

  ACE_SOCK_Acceptor listener(addr, 1);
  int32_t retVal = 0;
  if (waitSeconds > std::chrono::microseconds::zero()) {
    ACE_Time_Value wtime(waitSeconds);
    retVal = listener.accept(*m_io, nullptr, &wtime);
  } else {
    retVal = listener.accept(*m_io, nullptr);
  }
  if (retVal == -1) {
    char msg[256];
    int32_t lastError = ACE_OS::last_error();
    if (lastError == ETIME || lastError == ETIMEDOUT) {
      throw TimeoutException(
          "TcpConn::listen Attempt to listen timed out after " +
          to_string(waitSeconds) + ".");
    }
    std::snprintf(msg, 256, "TcpConn::listen failed with errno: %d: %s",
                  lastError, ACE_OS::strerror(lastError));
    throw GeodeIOException(msg);
  }
}

void TcpConn::connect(const char *hostname, int32_t port,
                      std::chrono::microseconds waitSeconds) {
  ACE_INET_Addr addr(port, hostname);
  m_addr = addr;
  m_waitMilliSeconds = waitSeconds;
  connect();
}

void TcpConn::connect(const char *ipaddr,
                      std::chrono::microseconds waitSeconds) {
  ACE_INET_Addr addr(ipaddr);
  m_addr = addr;
  m_waitMilliSeconds = waitSeconds;
  connect();
}

void TcpConn::connect() {
  using apache::geode::internal::chrono::duration::to_string;

  ACE_INET_Addr ipaddr = m_addr;
  std::chrono::microseconds waitMicroSeconds = m_waitMilliSeconds;

  ACE_OS::signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe

  LOGFINER("Connecting plain socket stream to %s:%d waiting %s micro sec",
           ipaddr.get_host_name(), ipaddr.get_port_number(),
           to_string(waitMicroSeconds).c_str());

  ACE_SOCK_Connector conn;
  int32_t retVal = 0;
  if (waitMicroSeconds > std::chrono::microseconds::zero()) {
    // passing waittime as microseconds
    ACE_Time_Value wtime(waitMicroSeconds);
    retVal = conn.connect(*m_io, ipaddr, &wtime);
  } else {
    retVal = conn.connect(*m_io, ipaddr);
  }
  if (retVal == -1) {
    char msg[256];
    int32_t lastError = ACE_OS::last_error();
    if (lastError == ETIME || lastError == ETIMEDOUT) {
      //  this is only called by constructor, so we must delete m_io
      _GEODE_SAFE_DELETE(m_io);
      throw TimeoutException(
          "TcpConn::connect Attempt to connect timed out after" +
          to_string(waitMicroSeconds) + ".");
    }
    std::snprintf(msg, 256, "TcpConn::connect failed with errno: %d: %s",
                  lastError, ACE_OS::strerror(lastError));
    //  this is only called by constructor, so we must delete m_io
    close();
    throw GeodeIOException(msg);
  }
  int rc = this->m_io->enable(ACE_NONBLOCK);
  if (-1 == rc) {
    char msg[256];
    int32_t lastError = ACE_OS::last_error();
    std::snprintf(msg, 256, "TcpConn::NONBLOCK: %d: %s", lastError,
                  ACE_OS::strerror(lastError));

    LOGINFO(msg);
  }
}

void TcpConn::close() {
  if (m_io != nullptr) {
    m_io->close();
    _GEODE_SAFE_DELETE(m_io);
  }
}

size_t TcpConn::receive(char *buff, size_t len,
                        std::chrono::microseconds waitSeconds) {
  return socketOp(SOCK_READ, buff, len, waitSeconds);
}

size_t TcpConn::send(const char *buff, size_t len,
                     std::chrono::microseconds waitSeconds) {
  return socketOp(SOCK_WRITE, const_cast<char *>(buff), len, waitSeconds);
}

size_t TcpConn::socketOp(TcpConn::SockOp op, char *buff, size_t len,
                         std::chrono::microseconds waitDuration) {
  {
    ACE_Time_Value waitTime(waitDuration);
    auto endTime = std::chrono::steady_clock::now() + waitDuration;
    size_t readLen = 0;
    ssize_t retVal;
    bool errnoSet = false;

    auto sendlen = len;
    size_t totalsend = 0;

    while (len > 0 && waitTime > ACE_Time_Value::zero) {
      if (len > m_chunkSize) {
        sendlen = m_chunkSize;
        len -= m_chunkSize;
      } else {
        sendlen = len;
        len = 0;
      }
      do {
        if (op == SOCK_READ) {
          retVal = m_io->recv_n(buff, sendlen, &waitTime, &readLen);
        } else {
          retVal = m_io->send_n(buff, sendlen, &waitTime, &readLen);
        }
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

//  Return the local port for this TCP connection.
uint16_t TcpConn::getPort() {
  ACE_INET_Addr localAddr;
  m_io->get_local_addr(localAddr);
  return localAddr.get_port_number();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
