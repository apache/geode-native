#pragma once

#ifndef GEODE_TCPSSLCONN_H_
#define GEODE_TCPSSLCONN_H_

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
#include <ace/DLL.h>
#include "../../cryptoimpl/Ssl.hpp"

namespace apache {
namespace geode {
namespace client {

class TcpSslConn : public TcpConn {
 private:
  Ssl* m_ssl;
  ACE_DLL m_dll;
  const char* m_pubkeyfile;
  const char* m_privkeyfile;
  const char* m_pemPassword;
  // adongre: Added for Ticket #758
  // Pass extra parameter for the password
  typedef void* (*gf_create_SslImpl)(ACE_SOCKET, const char*, const char*,
                                     const char*);
  typedef void (*gf_destroy_SslImpl)(void*);

  Ssl* getSSLImpl(ACE_SOCKET sock, const char* pubkeyfile,
                  const char* privkeyfile);

 protected:
  int32_t socketOp(SockOp op, char* buff, int32_t len, uint32_t waitSeconds);

  void createSocket(ACE_SOCKET sock);

 public:
  TcpSslConn(const char* hostname, int32_t port, uint32_t waitSeconds,
             int32_t maxBuffSizePool, const char* pubkeyfile,
             const char* privkeyfile, const char* pemPassword)
      : TcpConn(hostname, port, waitSeconds, maxBuffSizePool),
        m_ssl(nullptr),
        m_pubkeyfile(pubkeyfile),
        m_privkeyfile(privkeyfile),
        m_pemPassword(pemPassword){};

  TcpSslConn(const char* ipaddr, uint32_t waitSeconds, int32_t maxBuffSizePool,
             const char* pubkeyfile, const char* privkeyfile,
             const char* pemPassword)
      : TcpConn(ipaddr, waitSeconds, maxBuffSizePool),
        m_ssl(nullptr),
        m_pubkeyfile(pubkeyfile),
        m_privkeyfile(privkeyfile),
        m_pemPassword(pemPassword){};

  // TODO:  Watch out for virt dtor calling virt methods!

  virtual ~TcpSslConn() {}

  // Close this tcp connection
  void close();

  // Listen
  void listen(ACE_INET_Addr addr,
              uint32_t waitSeconds = DEFAULT_READ_TIMEOUT_SECS);

  // connect
  void connect();

  void setOption(int32_t level, int32_t option, void* val, int32_t len) {
    GF_DEV_ASSERT(m_ssl != nullptr);

    if (m_ssl->setOption(level, option, val, len) == -1) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR("Failed to set option, errno: %d: %s", lastError,
               ACE_OS::strerror(lastError));
    }
  }

  uint16_t getPort();
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPSSLCONN_H_
