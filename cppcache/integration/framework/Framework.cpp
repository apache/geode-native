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

#include "Framework.h"

#include <boost/asio.hpp>

uint16_t Framework::getAvailablePort() {
  using boost::asio::io_service;
  using boost::asio::ip::tcp;

  io_service service;
  tcp::socket socket{service};
  socket.open(tcp::v4());
  socket.bind(tcp::endpoint{tcp::v4(), 0});
  auto port = socket.local_endpoint().port();
  socket.close();

  return port;
}

const std::string& Framework::getHostname() {
  static const auto hostname = initHostname();
  return hostname;
}

std::string Framework::initHostname() { return boost::asio::ip::host_name(); }
