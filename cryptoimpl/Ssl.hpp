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

#ifndef GEODE_CRYPTOIMPL_SSL_H_
#define GEODE_CRYPTOIMPL_SSL_H_

#include <chrono>

#include <ace/INET_Addr.h>
#include <ace/OS.h>

namespace apache {
namespace geode {
namespace client {

class Ssl {
 public:
  virtual ~Ssl() noexcept = default;
  virtual int connect(ACE_INET_Addr, std::chrono::microseconds) = 0;
  virtual ssize_t recv(void*, size_t, const ACE_Time_Value*, size_t*) = 0;
  virtual ssize_t send(const void*, size_t, const ACE_Time_Value*, size_t*) = 0;
  virtual int getLocalAddr(ACE_Addr&) = 0;
  virtual void close() = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CRYPTOIMPL_SSL_H_
