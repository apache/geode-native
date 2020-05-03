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

#include <atomic>
#include <chrono>
#include <string>

#pragma warning(push)
#pragma warning(disable : 4311)
#pragma warning(disable : 4302)
#pragma pack(push)
#include <ace/SSL/SSL_SOCK_Stream.h>
#pragma pack(pop)
#pragma warning(pop)

#include "TcpConn.hpp"

namespace apache {
namespace geode {
namespace client {

class TcpSslConn : public TcpConn {
 private:
  static std::atomic_flag initialized_;
  const std::string trustStoreFile_;
  const std::string privateKeyFile_;
  const std::string password_;
  std::unique_ptr<ACE_SSL_SOCK_Stream> stream_;

 protected:
  void createSocket(ACE_HANDLE sock) override;

  ssize_t doOperation(const SockOp& op, void* buff, size_t sendlen,
                      ACE_Time_Value& waitTime, size_t& readLen) const override;

  void initSsl();

 public:
  TcpSslConn(const std::string& hostname, uint16_t port,
             std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool,
             std::string publicKeyFile, std::string privateKeyFile,
             std::string password)
      : TcpConn(hostname, port, waitSeconds, maxBuffSizePool),
        trustStoreFile_(std::move(publicKeyFile)),
        privateKeyFile_(std::move(privateKeyFile)),
        password_(std::move(password)) {
    initSsl();
  };

  TcpSslConn(const std::string& address, std::chrono::microseconds waitSeconds,
             int32_t maxBuffSizePool, std::string publicKeyFile,
             std::string privateKeyFile, std::string password)
      : TcpConn(address, waitSeconds, maxBuffSizePool),
        trustStoreFile_(std::move(publicKeyFile)),
        privateKeyFile_(std::move(privateKeyFile)),
        password_(std::move(password)) {
    initSsl();
  }

  virtual ~TcpSslConn() noexcept override = default;

  void close() override;

  void connect() override;

  uint16_t getPort() override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPSSLCONN_H_
