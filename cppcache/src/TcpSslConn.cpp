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

#include "TcpSslConn.hpp"

#include <memory>

#include <ace/SSL/SSL_SOCK_Connector.h>

#include <geode/ExceptionTypes.hpp>
#include <geode/SystemProperties.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

std::atomic_flag TcpSslConn::initialized_ = ATOMIC_FLAG_INIT;

void TcpSslConn::createSocket(ACE_HANDLE sock) {
  LOGDEBUG("Creating SSL socket stream");
  stream_ = std::unique_ptr<ACE_SSL_SOCK_Stream>(new ACE_SSL_SOCK_Stream());
  stream_->set_handle(sock);
}

void TcpSslConn::connect() {
  using apache::geode::internal::chrono::duration::to_string;

  ACE_OS::signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe

  LOGFINER(std::string("Connecting SSL socket stream to ") +
           inetAddress_.get_host_name() + ":" +
           std::to_string(inetAddress_.get_port_number()) + " waiting " +
           to_string(timeout_));

  ACE_SSL_SOCK_Connector conn;
  ACE_Time_Value actTimeout(timeout_);
  if (ACE_SSL_SOCK_Connector{}.connect(
          *stream_, inetAddress_,
          timeout_ > std::chrono::microseconds::zero() ? &actTimeout
                                                       : nullptr) == -1) {
    const auto lastError = ACE_OS::last_error();
    if (lastError == ETIME || lastError == ETIMEDOUT) {
      throw TimeoutException(
          "TcpSslConn::connect Attempt to connect timed out after " +
          to_string(timeout_) + ".");
    }
    close();
    throw GeodeIOException("TcpSslConn::connect failed with errno: " +
                           ACE_errno_to_string(lastError));
  }
}

void TcpSslConn::close() {
  if (stream_) {
    stream_.release()->close();
  }
}

uint16_t TcpSslConn::getPort() {
  ACE_INET_Addr localAddr;
  stream_->get_local_addr(localAddr);
  return localAddr.get_port_number();
}

static int pem_passwd_cb(char* buf, int size, int /*rwflag*/, void* passwd) {
  strncpy(buf, (char*)passwd, size);
  buf[size - 1] = '\0';
  return static_cast<int>(strlen(buf));
}

void TcpSslConn::initSsl() {
  if (!TcpSslConn::initialized_.test_and_set()) {
    auto sslContext = ACE_SSL_Context::instance();

    SSL_CTX_set_cipher_list(sslContext->context(), "DEFAULT");
    sslContext->set_mode(ACE_SSL_Context::SSLv23_client);
    sslContext->set_verify_peer();
    if (sslContext->load_trusted_ca(trustStoreFile_.c_str()) != 0) {
      throw SslException("Failed to read SSL trust store.");
    }

    if (!password_.empty()) {
      SSL_CTX_set_default_passwd_cb(sslContext->context(), pem_passwd_cb);
      SSL_CTX_set_default_passwd_cb_userdata(
          sslContext->context(), const_cast<char*>(password_.c_str()));
    }

    if (!privateKeyFile_.empty()) {
      if (sslContext->certificate(privateKeyFile_.c_str()) != 0) {
        throw SslException("Failed to read SSL certificate.");
      }
      if (sslContext->private_key(privateKeyFile_.c_str()) != 0) {
        throw SslException("Invalid SSL keystore password.");
      }
      if (SSL_CTX_use_certificate_chain_file(sslContext->context(),
                                             privateKeyFile_.c_str()) <= 0) {
        throw SslException("Failed to read SSL certificate chain.");
      }
    }
  }
}

ssize_t TcpSslConn::doOperation(const TcpConn::SockOp& op, void* buff,
                                size_t sendlen, ACE_Time_Value& waitTime,
                                size_t& readLen) const {
  if (op == SOCK_READ) {
    return stream_->recv_n(buff, sendlen, &waitTime, &readLen);
  } else {
    return stream_->send_n(buff, sendlen, &waitTime, &readLen);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
