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

#include "SSLImpl.hpp"

#include <cstdint>
#include <stdexcept>

#include <ace/Guard_T.h>

namespace apache {
namespace geode {
namespace client {

void* gf_create_SslImpl(ACE_HANDLE sock, const char* pubkeyfile,
                        const char* privkeyfile, const char* pemPassword) {
  return reinterpret_cast<void*>(
      new SSLImpl(sock, pubkeyfile, privkeyfile, pemPassword));
}

void gf_destroy_SslImpl(void* impl) {
  SSLImpl* theLib = reinterpret_cast<SSLImpl*>(impl);
  delete theLib;
}

extern "C" {
static int pem_passwd_cb(char* buf, int size, int /*rwflag*/, void* passwd) {
  strncpy(buf, (char*)passwd, size);
  buf[size - 1] = '\0';
  return static_cast<int>(strlen(buf));
}
}

std::atomic_flag SSLImpl::initialized_ = ATOMIC_FLAG_INIT;

SSLImpl::SSLImpl(ACE_HANDLE sock, const char* pubkeyfile,
                 const char* privkeyfile, const char* password) {
  if (!SSLImpl::initialized_.test_and_set()) {
    ACE_SSL_Context* sslContext = ACE_SSL_Context::instance();

    SSL_CTX_set_cipher_list(sslContext->context(), "DEFAULT");
    sslContext->set_mode(ACE_SSL_Context::SSLv23_client);
    sslContext->set_verify_peer();
    if (sslContext->load_trusted_ca(pubkeyfile) != 0) {
      throw std::invalid_argument("Failed to read SSL trust store.");
    }

    if (strlen(password) > 0) {
      SSL_CTX_set_default_passwd_cb(sslContext->context(), pem_passwd_cb);
      SSL_CTX_set_default_passwd_cb_userdata(sslContext->context(),
                                             const_cast<char*>(password));
    }

    if (privkeyfile && *privkeyfile) {
      if (sslContext->certificate(privkeyfile) != 0) {
        throw std::invalid_argument("Failed to read SSL certificate.");
      }
      if (sslContext->private_key(privkeyfile) != 0) {
        throw std::invalid_argument("Invalid SSL keystore password.");
      }
      if (::SSL_CTX_use_certificate_chain_file(sslContext->context(),
                                               privkeyfile) <= 0) {
        throw std::invalid_argument("Failed to read SSL certificate chain.");
      }
    }
  }

  m_io = std::unique_ptr<ACE_SSL_SOCK_Stream>(new ACE_SSL_SOCK_Stream());
  m_io->set_handle(sock);
}

void SSLImpl::close() {
  if (m_io) {
    m_io.release()->close();
  }
}

int SSLImpl::connect(ACE_INET_Addr inetAddress,
                     std::chrono::microseconds timeout) {
  ACE_SSL_SOCK_Connector conn;
  ACE_Time_Value actTimeout(timeout);
  return ACE_SSL_SOCK_Connector{}.connect(
      *m_io, inetAddress,
      timeout > std::chrono::microseconds::zero() ? &actTimeout : nullptr);
}

ssize_t SSLImpl::recv(void* buf, size_t len, const ACE_Time_Value* timeout,
                      size_t* bytes_transferred) {
  return m_io->recv_n(buf, len, 0, timeout, bytes_transferred);
}

ssize_t SSLImpl::send(const void* buf, size_t len,
                      const ACE_Time_Value* timeout,
                      size_t* bytes_transferred) {
  return m_io->send_n(buf, len, 0, timeout, bytes_transferred);
}

int SSLImpl::getLocalAddr(ACE_Addr& addr) { return m_io->get_local_addr(addr); }

}  // namespace client
}  // namespace geode
}  // namespace apache
