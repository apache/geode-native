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

#include <openssl/err.h>
#include <openssl/x509.h>

#include <chrono>
#include <thread>

#include <boost/exception/diagnostic_information.hpp>

#include <geode/SystemProperties.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

TcpSslConn::TcpSslConn(const std::string& hostname, uint16_t,
                       const std::string& sniProxyHostname,
                       uint16_t sniProxyPort,
                       std::chrono::microseconds connect_timeout,
                       int32_t maxBuffSizePool, const std::string& pubkeyfile,
                       const std::string& privkeyfile,
                       const std::string& pemPassword)
    : TcpConn{sniProxyHostname, sniProxyPort, connect_timeout, maxBuffSizePool},
      ssl_context_{boost::asio::ssl::context::sslv23_client} {
  init(pubkeyfile, privkeyfile, pemPassword, hostname);
}

TcpSslConn::TcpSslConn(const std::string& hostname, uint16_t port,
                       std::chrono::microseconds connect_timeout,
                       int32_t maxBuffSizePool, const std::string& pubkeyfile,
                       const std::string& privkeyfile,
                       const std::string& pemPassword)
    : TcpConn{hostname, port, connect_timeout, maxBuffSizePool},
      ssl_context_{boost::asio::ssl::context::sslv23_client} {
  init(pubkeyfile, privkeyfile, pemPassword);
}

TcpSslConn::TcpSslConn(const std::string& ipaddr,
                       std::chrono::microseconds connect_timeout,
                       int32_t maxBuffSizePool, const std::string& pubkeyfile,
                       const std::string& privkeyfile,
                       const std::string& pemPassword)
    : TcpSslConn{
          ipaddr.substr(0, ipaddr.find(':')),
          static_cast<uint16_t>(std::stoi(ipaddr.substr(ipaddr.find(':') + 1))),
          connect_timeout,
          maxBuffSizePool,
          pubkeyfile,
          privkeyfile,
          pemPassword} {}

TcpSslConn::TcpSslConn(const std::string& ipaddr,
                       std::chrono::microseconds connect_timeout,
                       int32_t maxBuffSizePool,
                       const std::string& sniProxyHostname,
                       uint16_t sniProxyPort, const std::string& pubkeyfile,
                       const std::string& privkeyfile,
                       const std::string& pemPassword)
    : TcpSslConn{
          ipaddr.substr(0, ipaddr.find(':')),
          static_cast<uint16_t>(std::stoi(ipaddr.substr(ipaddr.find(':') + 1))),
          sniProxyHostname,
          sniProxyPort,
          connect_timeout,
          maxBuffSizePool,
          pubkeyfile,
          privkeyfile,
          pemPassword} {}

void TcpSslConn::init(const std::string& pubkeyfile,
                      const std::string& privkeyfile,
                      const std::string& pemPassword,
                      const std::string& sniHostname) {
  // Most of the SSL configuration provided *through* Asio is on the context.
  // This configuration is copied into each SSL instance upon construction.
  // That means you need to get your configuration in order before you
  // construct the stream and connect the socket.
  LOGDEBUG(
      "*** TcpSslConn init, pubkeyfile = %s, pemPassword = %s, sniHostname = "
      "%s",
      pubkeyfile.c_str(), pemPassword.c_str(), sniHostname.c_str());

  try {
    ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_context_.load_verify_file(pubkeyfile);

    ssl_context_.set_password_callback(
        [pemPassword](std::size_t /*max_length*/,
                      boost::asio::ssl::context::password_purpose /*purpose*/) {
          return pemPassword;
        });

    if (!privkeyfile.empty()) {
      ssl_context_.use_certificate_chain_file(privkeyfile);
      ssl_context_.use_private_key_file(
          privkeyfile, boost::asio::ssl::context::file_format::pem);
    }

    auto stream = std::unique_ptr<ssl_stream_type>(
        new ssl_stream_type{socket_, ssl_context_});

    SSL_set_tlsext_host_name(stream->native_handle(), sniHostname.c_str());

    stream->handshake(ssl_stream_type::client);

    std::stringstream ss;
    ss << "Setup SSL " << socket_.local_endpoint() << " -> "
       << socket_.remote_endpoint();
    LOGINFO(ss.str());

    ss.clear();
    ss << "SNI hostname: " << sniHostname;
    LOGINFO(ss.str());

    socket_stream_ = std::move(stream);
  } catch (const boost::exception& ex) {
    // error handling
    std::string info = boost::diagnostic_information(ex);
    LOGDEBUG("caught boost exception: %s", info.c_str());
  }
}

TcpSslConn::~TcpSslConn() {
  std::stringstream ss;
  ss << "Teardown SSL " << socket_.local_endpoint() << " -> "
     << socket_.remote_endpoint();
  LOGFINE(ss.str());
}

size_t TcpSslConn::receive(char* buff, const size_t len,
                           std::chrono::milliseconds) {
  auto start = std::chrono::system_clock::now();

  return boost::asio::read(*socket_stream_, boost::asio::buffer(buff, len),
                           [len, start](boost::system::error_code& ec,
                                        const std::size_t n) -> std::size_t {
                             if (ec && ec != boost::asio::error::eof) {
                               // Quit if we encounter an error.
                               // Defer EOF to timeout.
                               return 0;
                             } else if (start + std::chrono::milliseconds(25) <=
                                        std::chrono::system_clock::now()) {
                               // Sometimes we don't know how much data to
                               // expect, so we're reading into an oversized
                               // buffer without knowing when to quit other than
                               // by timeout. Typically, if we timeout, we also
                               // have an EOF, meaning the connection is likely
                               // broken and will have to be closed. But if we
                               // have bytes, we may have just done a
                               // dumb/blind/hail mary receive, so defer broken
                               // connection handling until the next IO
                               // operation.
                               if (n) {
                                 // This prevents the timeout from being an
                                 // error condition.
                                 ec = boost::system::error_code{};
                               }
                               // But if n == 0 when we timeout, it's just a
                               // broken connection.

                               return 0;
                             }

                             return len - n;
                           });
}

size_t TcpSslConn::send(const char* buff, const size_t len,
                        std::chrono::milliseconds) {
  return boost::asio::write(*socket_stream_, boost::asio::buffer(buff, len));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
