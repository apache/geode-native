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

#ifndef GEODE_TCPCONN_H_
#define GEODE_TCPCONN_H_

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <geode/internal/geode_globals.hpp>

#include "Connector.hpp"

namespace apache {
namespace geode {
namespace client {
class APACHE_GEODE_EXPORT TcpConn : public Connector {
  size_t receive(char*, size_t, std::chrono::milliseconds) override;
  size_t receive_nothrowiftimeout(char*, size_t,
                                  std::chrono::milliseconds) override;
  size_t send(const char*, size_t, std::chrono::milliseconds) override;

  uint16_t getPort() override final;

 protected:
  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket socket_;

  boost::asio::ip::tcp::resolver::results_type resolve(
      const std::string hostname, uint16_t port,
      std::chrono::microseconds timeout);

  void connect(boost::asio::ip::tcp::resolver::results_type r,
               std::chrono::microseconds connect_timeout);

  size_t receive(char*, size_t, std::chrono::milliseconds,
                 bool throwTimeoutException);

  virtual void prepareAsyncRead(
      char* buff, size_t len,
      boost::optional<boost::system::error_code>& read_result,
      std::size_t& bytes_read);

  virtual void prepareAsyncWrite(
      const char* buff, size_t len,
      boost::optional<boost::system::error_code>& write_result,
      std::size_t& bytes_written);

 public:
  TcpConn(const std::string ipaddr, std::chrono::microseconds connect_timeout,
          int32_t maxBuffSizePool);

  TcpConn(const std::string hostname, uint16_t port,
          std::chrono::microseconds connect_timeout, int32_t maxBuffSizePool);

  TcpConn(const std::string ipaddr, std::chrono::microseconds connect_timeout,
          int32_t maxBuffSizePool, std::chrono::microseconds send_timeout,
          std::chrono::microseconds receive_timeout);

  ~TcpConn() override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPCONN_H_
