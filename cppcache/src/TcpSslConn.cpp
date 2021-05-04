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
#include <iostream>
#include <thread>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/optional.hpp>

#include <geode/ExceptionTypes.hpp>
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
      ssl_context_{boost::asio::ssl::context::sslv23_client},
      strand_(io_context_) {
  init(pubkeyfile, privkeyfile, pemPassword, hostname);
}

TcpSslConn::TcpSslConn(const std::string& hostname, uint16_t port,
                       std::chrono::microseconds connect_timeout,
                       int32_t maxBuffSizePool, const std::string& pubkeyfile,
                       const std::string& privkeyfile,
                       const std::string& pemPassword)
    : TcpConn{hostname, port, connect_timeout, maxBuffSizePool},
      ssl_context_{boost::asio::ssl::context::sslv23_client},
      strand_(io_context_) {
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
  LOG_DEBUG(
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
    LOG_INFO(ss.str());

    ss.clear();
    ss << "SNI hostname: " << sniHostname;
    LOG_INFO(ss.str());

    socket_stream_ = std::move(stream);
  } catch (const boost::exception& ex) {
    // error handling
    std::string info = boost::diagnostic_information(ex);
    LOG_DEBUG("caught boost exception: %s", info.c_str());
    throw apache::geode::client::SslException(info.c_str());
  }
}

TcpSslConn::~TcpSslConn() {
  std::stringstream ss;
  ss << "Teardown SSL " << socket_.local_endpoint() << " -> ";
  try {
    ss << socket_.remote_endpoint();
  } catch (...) {
  }
  LOG_FINE(ss.str());
}

void TcpSslConn::prepareAsyncRead(
    char* buff, size_t len,
    boost::optional<boost::system::error_code>& read_result,
    std::size_t& bytes_read) {
  boost::asio::async_read(
      *socket_stream_, boost::asio::buffer(buff, len),
      boost::asio::bind_executor(
          strand_, [&read_result, &bytes_read](
                       const boost::system::error_code& ec, const size_t n) {
            bytes_read = n;

            // EOF itself occurs when there is no data available on the socket
            // at the time of the read. It may simply imply data has yet to
            // arrive. Do nothing. Defer to timeout rather than assume a broken
            // connection.
            if (ec != boost::asio::error::eof &&
                ec != boost::asio::error::try_again) {
              read_result = ec;
              return;
            }
          }));
}

void TcpSslConn::prepareAsyncWrite(
    const char* buff, size_t len,
    boost::optional<boost::system::error_code>& write_result,
    std::size_t& bytes_written) {
  boost::asio::async_write(
      *socket_stream_, boost::asio::buffer(buff, len),
      boost::asio::bind_executor(
          strand_, [&write_result, &bytes_written](
                       const boost::system::error_code& ec, const size_t n) {
            bytes_written = n;

            if (ec != boost::asio::error::eof &&
                ec != boost::asio::error::try_again) {
              write_result = ec;
              return;
            }
          }));
}

}  // namespace client
}  // namespace geode
}  // namespace apache
