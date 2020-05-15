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

#include "TcpIpc.hpp"

#include <errno.h>
#include <memory.h>

#include <chrono>
#include <fwklib/FwkLog.hpp>
#include <thread>

#include <ace/INET_Addr.h>
#include <ace/OS.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_IO.h>

#include "config.h"

namespace apache {
namespace geode {
namespace client {
namespace testframework {

void TcpIpc::clearNagle(ACE_HANDLE sock) {
  int32_t val = 1;
  char *param = reinterpret_cast<char *>(&val);
  int32_t plen = sizeof(val);

  if (0 != ACE_OS::setsockopt(sock, IPPROTO_TCP, 1, param, plen)) {
    FWKSEVERE("Failed to set NAGLE on socket.  Errno: " << errno);
  }
}

int32_t TcpIpc::getSize(ACE_HANDLE sock, int32_t flag) {
  int32_t val = 0;
  auto *param = reinterpret_cast<char *>(&val);
  int32_t plen = sizeof(val);

  if (0 != ACE_OS::getsockopt(sock, SOL_SOCKET, flag, param, &plen)) {
    FWKSEVERE("Failed to get buff size for flag "
              << flag << " on socket.  Errno: " << errno);
  }
#ifdef _LINUX
  val /= 2;
#endif
  return val;
}

int32_t TcpIpc::setSize(ACE_HANDLE sock, int32_t flag, int32_t size) {
  int32_t val = 0;
  if (size <= 0) return 0;

  auto *param = reinterpret_cast<char *>(&val);
  int32_t plen = sizeof(val);

  int32_t inc = 32120;
  val = size - (3 * inc);
  if (val < 0) val = 0;
  int32_t red = 0;
  int32_t lastRed = -1;
  while (lastRed != red) {
    lastRed = red;
    val += inc;
    ACE_OS::setsockopt(sock, SOL_SOCKET, flag, param, plen);
    if (0 != ACE_OS::getsockopt(sock, SOL_SOCKET, flag, param, &plen)) {
      FWKSEVERE("Failed to get buff size for flag "
                << flag << " on socket.  Errno: " << errno);
    }
#ifdef _LINUX
    val /= 2;
#endif
    if (val < size) red = val;
  }
  return val;
}

void TcpIpc::init(int32_t sockBufferSize) {
  auto sock = ACE_OS::socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    FWKSEVERE("Failed to create socket.  Errno: " << errno);
  }

  if (sockBufferSize > 0) {
    clearNagle(sock);

    setSize(sock, SO_SNDBUF, sockBufferSize);
    setSize(sock, SO_RCVBUF, sockBufferSize);
  }
  m_io = new ACE_SOCK_Stream(sock);
  ACE_OS::signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe
}

bool TcpIpc::accept(ACE_SOCK_Acceptor *acceptor, int32_t waitSecs) {
  if (acceptor->accept(*m_io, nullptr, new ACE_Time_Value(waitSecs)) != 0) {
    FWKSEVERE("Accept failed with errno: " << errno);
    return false;
  }
  return true;
}

bool TcpIpc::connect(int32_t waitSecs) {
  if (m_ipaddr.empty()) {
    FWKSEVERE("Connect failed, address not set.");
    return false;
  }
  ACE_INET_Addr driver(m_ipaddr.c_str());
  ACE_SOCK_Connector conn;
  int32_t retVal = -1;
  while ((retVal == -1) && (waitSecs-- > 0)) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    errno = 0;
    retVal = conn.connect(*m_io, driver);
  }
  if (retVal == -1) {
    FWKSEVERE("Attempt to connect failed, errno: " << errno);
    return false;
  }
  return true;
}

TcpIpc::~TcpIpc() { close(); }

void TcpIpc::close() {
  if (m_io != nullptr) {
    m_io->close();
    delete m_io;
    m_io = nullptr;
  }
}

int32_t TcpIpc::readBuffer(char **buffer, int32_t waitSecs) {
  ACE_Time_Value wtime(waitSecs);
  iovec buffs;
  buffs.iov_base = nullptr;
  buffs.iov_len = 0;
  int32_t red = static_cast<int32_t>(m_io->recvv(&buffs, &wtime));
  if ((red == -1) && ((errno == ECONNRESET) || (errno == EPIPE))) {
    FWKEXCEPTION("During attempt to read: Connection failure errno: " << errno);
  }
  if (red == -1) {
  }
  *buffer = reinterpret_cast<char *>(buffs.iov_base);
  return buffs.iov_len;
}

int32_t TcpIpc::sendBuffers(int32_t cnt, char *buffers[], int32_t lengths[],
                            int32_t waitSecs) {
  ACE_Time_Value wtime(waitSecs);
  int32_t tot = 0;
  if (cnt > 2) {
    FWKEXCEPTION("During attempt to write: Too many buffers passed in.");
  }
  iovec buffs[2];
  for (int32_t idx = 0; idx < cnt; idx++) {
    buffs[idx].iov_base = buffers[idx];
    buffs[idx].iov_len = lengths[idx];
    tot += lengths[idx];
  }
  int32_t wrote = static_cast<int32_t>(m_io->sendv(buffs, cnt, &wtime));
  if ((wrote == -1) && ((errno == ECONNRESET) || (errno == EPIPE))) {
    FWKEXCEPTION(
        "During attempt to write: Connection failure errno: " << errno);
  }
  if (tot != wrote) {
    FWKSEVERE("Failed to write all bytes attempted, wrote "
              << wrote << ", attempted " << tot);
  }
  return wrote;
}

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache
