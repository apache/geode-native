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

#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

ServerLocation::ServerLocation(std::string serverName, int port)
    : Serializable(), m_serverName(std::move(serverName)), m_port(port) {
  LOGDEBUG(
      "ServerLocation::ServerLocation(): creating ServerLocation for %s:%d",
      serverName.c_str(), port);
  makeEpString();
}

ServerLocation::ServerLocation()
    : Serializable(),
      m_serverName(),
      m_port(-1)  // Default constructor for deserialiozation.
{}

ServerLocation::ServerLocation(std::string name) {
  auto position = name.find_first_of(":");
  m_serverName = name.substr(0, position);
  m_port = std::stoi(name.substr(position + 1));
  makeEpString();
}

const std::string& ServerLocation::getServerName() const {
  return m_serverName;
}

void ServerLocation::setServername(std::string serverName) {
  m_serverName = std::move(serverName);
  makeEpString();
}

int ServerLocation::getPort() const { return m_port; }

void ServerLocation::toData(DataOutput& output) const {
  output.writeString(m_serverName);
  output.writeInt(m_port);
}

void ServerLocation::fromData(DataInput& input) {
  m_serverName = input.readString();
  m_port = input.readInt32();
  makeEpString();
}

size_t ServerLocation::objectSize() const {
  size_t size = sizeof(ServerLocation);
  size += m_serverName.length();
  return size;
}

void ServerLocation::printInfo() {
  LOGDEBUG(" Got Host %s, and port %d", getServerName().c_str(), m_port);
}

bool ServerLocation::operator<(const ServerLocation rhs) const {
  if (m_serverName < rhs.m_serverName) {
    return true;
  } else if (m_serverName == rhs.m_serverName) {
    return (m_port < rhs.m_port);
  } else {
    return false;
  }
}

bool ServerLocation::operator==(const ServerLocation& rhs) const {
  return (m_serverName == rhs.m_serverName) && (m_port == rhs.m_port);
}

bool ServerLocation::isValid() const {
  return !m_serverName.empty() && m_port >= 0;
}

const std::string& ServerLocation::getEpString() { return m_epString; }

int ServerLocation::hashcode() const {
  int prime = 31;
  int result = 1;
  result =
      prime * result + static_cast<int>(std::hash<std::string>{}(m_serverName));
  result = prime * result + m_port;
  return result;
}

void ServerLocation::makeEpString() {
  m_epString = m_serverName + ":" + std::to_string(m_port);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
