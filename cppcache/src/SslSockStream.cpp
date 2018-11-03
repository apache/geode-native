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

#include "SslSockStream.hpp"

#include <ace/OS_NS_stdio.h>

#include <geode/ExceptionTypes.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

void *SslSockStream::getACESSLFuncPtr(const char *function_name) {
  void *func = m_dll.symbol(function_name);
  if (func == nullptr) {
    char msg[1000];
    std::snprintf(msg, 1000, "cannot find function %s in library %s",
                     function_name, "cryptoImpl");
    LOGERROR(msg);
    throw IllegalStateException(msg);
  }
  return func;
}

void SslSockStream::initACESSLFuncPtrs() {
  const char *libName = "cryptoImpl";
  if (m_dll.open(libName, ACE_DEFAULT_SHLIB_MODE, 0) == -1) {
    int32_t lastError = ACE_OS::last_error();
    LOGERROR("Failed to open cryptoImpl . Errno: %d : %s", lastError,
             ACE_OS::strerror(lastError));
    char msg[1000] = {0};
    std::snprintf(msg, 1000, "cannot open library: %s", libName);
    LOGERROR(msg);
    throw FileNotFoundException(msg);
  }

#define ASSIGN_SSL_FUNC_PTR(OrigName) \
  OrigName##_Ptr = (OrigName##_Type)getACESSLFuncPtr(#OrigName);

  ASSIGN_SSL_FUNC_PTR(gf_initSslImpl)
  ASSIGN_SSL_FUNC_PTR(gf_clearSslImpl)
  ASSIGN_SSL_FUNC_PTR(gf_set_option)
  ASSIGN_SSL_FUNC_PTR(gf_listen)
  ASSIGN_SSL_FUNC_PTR(gf_connect)
  ASSIGN_SSL_FUNC_PTR(gf_recv_n)
  ASSIGN_SSL_FUNC_PTR(gf_send_n)
  ASSIGN_SSL_FUNC_PTR(gf_get_local_addr)
}

SslSockStream::SslSockStream(ACE_HANDLE sock, const char *pubkey,
                             const char *privkey)
    : m_ctx(nullptr),
      m_sock(sock),
      m_pubkey(pubkey),
      m_privkey(privkey),
      gf_initSslImpl_Ptr(nullptr),
      gf_clearSslImpl_Ptr(nullptr),
      gf_set_option_Ptr(nullptr),
      gf_listen_Ptr(nullptr),
      gf_connect_Ptr(nullptr),
      gf_recv_n_Ptr(nullptr),
      gf_send_n_Ptr(nullptr),
      gf_get_local_addr_Ptr(nullptr) {}

void SslSockStream::init() {
  initACESSLFuncPtrs();
  m_ctx = gf_initSslImpl_Ptr(m_sock, m_pubkey, m_privkey);
  LOGDEBUG("Got %p as SSL socket context address", m_ctx);
}

int SslSockStream::set_option(int level, int option, void *optval,
                              int optlen) const {
  return gf_set_option_Ptr(m_ctx, level, option, optval, optlen);
}

int SslSockStream::listen(ACE_INET_Addr addr, unsigned waitSeconds) {
  return gf_listen_Ptr(m_ctx, addr, waitSeconds);
}

int SslSockStream::connect(ACE_INET_Addr ipaddr, unsigned waitSeconds) {
  return gf_connect_Ptr(m_ctx, ipaddr, waitSeconds);
}

ssize_t SslSockStream::recv_n(void *buf, size_t len,
                              const ACE_Time_Value *timeout,
                              size_t *bytes_transferred) const {
  return gf_recv_n_Ptr(m_ctx, buf, len, timeout, bytes_transferred);
}

ssize_t SslSockStream::send_n(const void *buf, size_t len,
                              const ACE_Time_Value *timeout,
                              size_t *bytes_transferred) const {
  return gf_send_n_Ptr(m_ctx, buf, len, timeout, bytes_transferred);
}

int SslSockStream::get_local_addr(ACE_Addr &addr) const {
  return gf_get_local_addr_Ptr(m_ctx, addr);
}

int SslSockStream::close() {
  gf_clearSslImpl_Ptr(m_ctx);
  m_ctx = nullptr;
  return 0;
}

SslSockStream::~SslSockStream() { close(); }
}  // namespace client
}  // namespace geode
}  // namespace apache
