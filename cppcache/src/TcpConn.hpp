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

#include <ace/SOCK_Stream.h>
#include <boost/interprocess/mapped_region.hpp>

#include <geode/internal/geode_globals.hpp>

#include "Connector.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

#ifdef WIN32

#define TCPLEVEL IPPROTO_TCP

#else

#include <sys/socket.h>
#include <sys/types.h>

#define TCPLEVEL SOL_TCP

#endif

class APACHE_GEODE_EXPORT TcpConn : public Connector {
 private:
  ACE_SOCK_Stream* m_io;

 protected:
  ACE_INET_Addr m_addr;
  std::chrono::microseconds m_waitMilliSeconds;

  int32_t m_maxBuffSizePool;

  enum SockOp { SOCK_READ, SOCK_WRITE };

  void clearNagle(ACE_HANDLE sock);
  int32_t maxSize(ACE_HANDLE sock, int32_t flag, int32_t size);

  virtual size_t socketOp(SockOp op, char* buff, size_t len,
                          std::chrono::microseconds waitDuration);

  virtual void createSocket(ACE_HANDLE sock);

 public:
  size_t m_chunkSize;

  static size_t getDefaultChunkSize() {
    // Attempt to set chunk size to nearest OS page size
    // for perf improvement
    auto pageSize = boost::interprocess::mapped_region::get_page_size();
    if (pageSize > 16000000) {
      return 16000000;
    } else if (pageSize > 0) {
      return pageSize + (16000000 / pageSize) * pageSize;
    }

    return 16000000;
  }

  TcpConn(const char* hostname, int32_t port,
          std::chrono::microseconds waitSeconds, int32_t maxBuffSizePool);
  TcpConn(const char* ipaddr, std::chrono::microseconds waitSeconds,
          int32_t maxBuffSizePool);

  virtual ~TcpConn() override { close(); }

  // Close this tcp connection
  virtual void close() override;

  void init() override;

  // Listen
  void listen(
      const char* hostname, int32_t port,
      std::chrono::microseconds waitSeconds = DEFAULT_READ_TIMEOUT_SECS);
  void listen(const char* ipaddr, std::chrono::microseconds waitSeconds =
                                      DEFAULT_READ_TIMEOUT_SECS);

  virtual void listen(
      ACE_INET_Addr addr,
      std::chrono::microseconds waitSeconds = DEFAULT_READ_TIMEOUT_SECS);

  // connect
  void connect(const char* hostname, int32_t port,
               std::chrono::microseconds waitSeconds = DEFAULT_CONNECT_TIMEOUT);
  void connect(const char* ipaddr,
               std::chrono::microseconds waitSeconds = DEFAULT_CONNECT_TIMEOUT);

  virtual void connect();

  size_t receive(char* buff, size_t len,
                 std::chrono::microseconds waitSeconds) override;
  size_t send(const char* buff, size_t len,
              std::chrono::microseconds waitSeconds) override;

  virtual void setOption(int32_t level, int32_t option, void* val, size_t len) {
    if (m_io->set_option(level, option, val, static_cast<int32_t>(len)) == -1) {
      int32_t lastError = ACE_OS::last_error();
      LOGERROR("Failed to set option, errno: %d: %s", lastError,
               ACE_OS::strerror(lastError));
    }
  }

  void setIntOption(int32_t level, int32_t option, int32_t val) {
    setOption(level, option, &val, sizeof(int32_t));
  }

  void setBoolOption(int32_t level, int32_t option, bool val) {
    setOption(level, option, &val, sizeof(bool));
  }

  virtual uint16_t getPort() override;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCPCONN_H_
