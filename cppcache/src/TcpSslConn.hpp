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

#include <ace/DLL.h>

#include "../../cryptoimpl/Ssl.hpp"
#include "TcpConn.hpp"

namespace apache {
namespace geode {
namespace client {

class TcpSslConn : public TcpConn {
 private:
  static const std::string kLibraryName;
  std::unique_ptr<Ssl> ssl_;
  ACE_DLL dll_;
  const std::string publicKeyFile_;
  const std::string privateKeyFile_;
  const std::string password_;

  typedef void* (*gf_create_SslImpl)(ACE_HANDLE, const char*, const char*,
                                     const char*);
  typedef void (*gf_destroy_SslImpl)(void*);

  Ssl* getSSLImpl(ACE_HANDLE sock, const char* pubkeyfile,
                  const char* privkeyfile);

 protected:
  size_t socketOp(SockOp op, char* buff, size_t len,
                  std::chrono::microseconds waitDuration) override;

  void createSocket(ACE_HANDLE sock) override;

 public:
  TcpSslConn(const std::string& hostname, uint16_t port,
             std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool,
             std::string publicKeyFile, std::string privateKeyFile,
             std::string password)
      : TcpConn(hostname, port, waitSeconds, maxBuffSizePool),
        publicKeyFile_(std::move(publicKeyFile)),
        privateKeyFile_(std::move(privateKeyFile)),
        password_(std::move(password)){};

  TcpSslConn(const std::string& address, std::chrono::microseconds waitSeconds,
             int32_t maxBuffSizePool, std::string publicKeyFile,
             std::string privateKeyFile, std::string password)
      : TcpConn(address, waitSeconds, maxBuffSizePool),
        publicKeyFile_(std::move(publicKeyFile)),
        privateKeyFile_(std::move(privateKeyFile)),
        password_(std::move(password)){};

  virtual ~TcpSslConn() override {}

  void close() override;

  void connect() override;

  uint16_t getPort() override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPSSLCONN_H_
