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

#include <chrono>
#include <memory>

#include <ace/SOCK_Stream.h>

#include <geode/internal/geode_globals.hpp>

#include "Connector.hpp"

namespace apache {
namespace geode {
namespace client {

inline std::string ACE_errno_to_string(decltype(ACE_OS::last_error()) error) {
  return std::to_string(error) + ": " + ACE_OS::strerror(error);
}

class TcpConn : public Connector {
 private:
  std::unique_ptr<ACE_SOCK_Stream> stream_;
  const int32_t maxBuffSizePool_;

  /**
   * Attempt to set chunk size to nearest OS page size for perf improvement
   */
  static size_t getDefaultChunkSize();

 protected:
  ACE_INET_Addr inetAddress_;
  std::chrono::microseconds timeout_;
  static const size_t kChunkSize;

  enum SockOp { SOCK_READ, SOCK_WRITE };

  void clearNagle(ACE_HANDLE sock);
  int32_t maxSize(ACE_HANDLE sock, int32_t flag, int32_t size);

  virtual size_t socketOp(SockOp op, char* buff, size_t len,
                          std::chrono::microseconds waitDuration);

  virtual void createSocket(ACE_HANDLE sock);

  virtual ssize_t doOperation(const SockOp& op, void* buff, size_t sendlen,
                              ACE_Time_Value& waitTime, size_t& readLen) const;

 public:
  TcpConn(const std::string& hostname, uint16_t port,
          std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool);

  TcpConn(const std::string& address, std::chrono::microseconds waitSeconds,
          int32_t maxBuffSizePool);

  ~TcpConn() override {}

  void close() override;

  void init() override;

  virtual void connect();

  size_t receive(char* buff, size_t len,
                 std::chrono::microseconds waitSeconds) override;

  size_t send(const char* buff, size_t len,
              std::chrono::microseconds waitSeconds) override;

  virtual uint16_t getPort() override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPCONN_H_
