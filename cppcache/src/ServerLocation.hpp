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

#ifndef GEODE_SERVERLOCATION_H_
#define GEODE_SERVERLOCATION_H_

#include <string>

#include <geode/CacheableBuiltins.hpp>
#include <geode/CacheableString.hpp>
#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/DataSerializableInternal.hpp>

#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

class ServerLocation : public internal::DataSerializableInternal {
 public:
  ServerLocation(std::string serverName, int port)
      : Serializable(), m_serverName(std::move(serverName)), m_port(port) {
    LOG_DEBUG(
        "ServerLocation::ServerLocation(): creating ServerLocation for %s:%d",
        m_serverName.c_str(), port);
    makeEpString();
  }

  ServerLocation()
      : Serializable(),
        m_serverName(),
        m_port(-1)  // Default constructor for deserialiozation.
  {}

  explicit ServerLocation(std::string name) {
    /*
    name = Utils::convertHostToCanonicalForm(name.c_str());
    */
    auto position = name.find_last_of(":");
    m_serverName = name.substr(0, position);
    m_port = std::stoi(name.substr(position + 1));
    makeEpString();
  }

  const std::string& getServerName() const { return m_serverName; }

  int getPort() const { return m_port; }

  void toData(DataOutput& output) const override {
    output.writeString(m_serverName);
    output.writeInt(m_port);
  }

  void fromData(DataInput& input) override {
    m_serverName = input.readString();
    m_port = input.readInt32();
    makeEpString();
  }

  size_t objectSize() const override {
    size_t size = sizeof(ServerLocation);
    size += m_serverName.length();
    return size;
  }

  void printInfo() {
    LOG_DEBUG(" Got Host \"%s\", and port %d", getServerName().c_str(), m_port);
  }

  bool operator<(const ServerLocation rhs) const {
    if (m_serverName < rhs.m_serverName) {
      return true;
    } else if (m_serverName == rhs.m_serverName) {
      return (m_port < rhs.m_port);
    } else {
      return false;
    }
  }

  bool operator==(const ServerLocation& rhs) const {
    return (m_serverName == rhs.m_serverName) && (m_port == rhs.m_port);
  }

  inline bool isValid() const { return !m_serverName.empty() && m_port >= 0; }

  inline const std::string& getEpString() { return m_epString; }

  inline int hashcode() const {
    int prime = 31;
    int result = 1;
    result = prime * result +
             static_cast<int>(std::hash<std::string>{}(m_serverName));
    result = prime * result + m_port;
    return result;
  }

  inline void makeEpString() {
    m_epString = m_serverName + ":" + std::to_string(m_port);
  }

 protected:
  std::string m_serverName;
  int m_port;
  std::string m_epString;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::ServerLocation> {
  typedef apache::geode::client::ServerLocation argument_type;
  typedef size_t result_type;
  size_t operator()(const apache::geode::client::ServerLocation& val) const {
    return val.hashcode();
  }
};

}  // namespace std

#endif  // GEODE_SERVERLOCATION_H_
