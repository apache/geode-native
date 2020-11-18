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

#pragma once

#ifndef GEODE_TCPSSLCONN_H_
#define GEODE_TCPSSLCONN_H_

#include <boost/asio/ssl.hpp>

#include "TcpConn.hpp"

namespace apache {
namespace geode {
namespace client {

class TcpSslConn : public TcpConn {
 protected:
  using ssl_stream_type =
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>;

  boost::asio::ssl::context ssl_context_;
  std::unique_ptr<ssl_stream_type> socket_stream_;
  boost::asio::io_context::strand strand_;

  void prepareAsyncRead(char* buff, size_t len,
                        boost::optional<boost::system::error_code>& read_result,
                        std::size_t& bytes_read) override;

  void prepareAsyncWrite(
      const char* buff, size_t len,
      boost::optional<boost::system::error_code>& write_result,
      std::size_t& bytes_written) override;

 public:
  TcpSslConn(const std::string& hostname, uint16_t port,
             const std::string& sniProxyHostname, uint16_t sniProxyPort,
             std::chrono::microseconds connect_timeout, int32_t maxBuffSizePool,
             const std::string& pubkeyfile, const std::string& privkeyfile,
             const std::string& pemPassword);

  TcpSslConn(const std::string& hostname, uint16_t port,
             std::chrono::microseconds connect_timeout, int32_t maxBuffSizePool,
             const std::string& pubkeyfile, const std::string& privkeyfile,
             const std::string& pemPassword);

  TcpSslConn(const std::string& ipaddr,
             std::chrono::microseconds connect_timeout, int32_t maxBuffSizePool,
             const std::string& pubkeyfile, const std::string& privkeyfile,
             const std::string& pemPassword);

  TcpSslConn(const std::string& ipaddr, std::chrono::microseconds waitSeconds,
             int32_t maxBuffSizePool, const std::string& sniProxyHostname,
             uint16_t sniProxyPort, const std::string& publicKeyFile,
             const std::string& privateKeyFile, const std::string& password);

  ~TcpSslConn() override;

 private:
  void init(const std::string& pubkeyfile, const std::string& privkeyfile,
            const std::string& pemPassword,
            const std::string& sniHostname = "");
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPSSLCONN_H_
