#pragma once

#ifndef GEODE_FWKLIB_TCPIPC_H_
#define GEODE_FWKLIB_TCPIPC_H_

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

#include <cstdint>
#include <string>

#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>

#include <geode/internal/geode_base.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testframework {

class TcpIpc {
 private:
  ACE_SOCK_Stream* m_io;
  std::string m_ipaddr;

  void init(int32_t sockBufferSize = 0);
  void clearNagle(ACE_HANDLE sock);
  int32_t setSize(ACE_HANDLE sock, int32_t flag, int32_t size);
  int32_t getSize(ACE_HANDLE sock, int32_t flag);

 public:
  explicit TcpIpc(std::string& ipaddr, int32_t sockBufferSize = 0)
      : m_ipaddr(ipaddr) {
    init(sockBufferSize);
  }
  explicit TcpIpc(char* ipaddr, int32_t sockBufferSize = 0) : m_ipaddr(ipaddr) {
    init(sockBufferSize);
  }

  explicit TcpIpc(int32_t sockBufferSize = 0) { init(sockBufferSize); }

  ~TcpIpc();

  void close();

  bool accept(ACE_SOCK_Acceptor* acceptor, int32_t waitSecs = 0);
  bool connect(int32_t waitSecs = 0);

  int32_t readBuffer(char** buffer, int32_t waitSecs = 0);
  int32_t sendBuffer(char* buffer, int32_t length, int32_t waitSecs = 0) {
    char* buffs[1];
    buffs[0] = buffer;
    int32_t lengths[1];
    lengths[0] = length;
    return sendBuffers(1, buffs, lengths, waitSecs);
  }
  int32_t sendBuffers(int32_t cnt, char* buffers[], int32_t lengths[],
                      int32_t waitSecs = 0);
};

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FWKLIB_TCPIPC_H_
