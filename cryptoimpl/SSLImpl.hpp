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

#ifndef GEODE_CRYPTOIMPL_SSLIMPL_H_
#define GEODE_CRYPTOIMPL_SSLIMPL_H_

#include <atomic>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4311)
#pragma warning(disable : 4302)
#endif

#pragma pack(push)

#ifdef _WIN32
#pragma error_messages(off, macroredef)
#endif

#include <ace/INET_Addr.h>
#include <ace/OS.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/SOCK_IO.h>
#include <ace/SSL/SSL_SOCK_Acceptor.h>
#include <ace/SSL/SSL_SOCK_Connector.h>
#include <ace/Time_Value.h>

#ifdef _WIN32
#pragma error_messages(on, macroredef)
#endif

#pragma pack(pop)

#include "Ssl.hpp"
#include "cryptoimpl_export.h"

namespace apache {
namespace geode {
namespace client {

class SSLImpl : public apache::geode::client::Ssl {
 private:
  static std::atomic_flag initialized_;
  std::unique_ptr<ACE_SSL_SOCK_Stream> m_io;

 public:
  SSLImpl(ACE_HANDLE sock, const char* pubkeyfile, const char* privkeyfile,
          const char* password);
  ~SSLImpl() noexcept override = default;
  int connect(ACE_INET_Addr, std::chrono::microseconds) override;
  ssize_t recv(void*, size_t, const ACE_Time_Value*, size_t*) override;
  ssize_t send(const void*, size_t, const ACE_Time_Value*, size_t*) override;
  int getLocalAddr(ACE_Addr&) override;
  void close() override;
};

extern "C" {
CRYPTOIMPL_EXPORT void* gf_create_SslImpl(ACE_HANDLE sock,
                                          const char* pubkeyfile,
                                          const char* privkeyfile,
                                          const char* pemPassword);
CRYPTOIMPL_EXPORT void gf_destroy_SslImpl(void* impl);
}

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CRYPTOIMPL_SSLIMPL_H_
