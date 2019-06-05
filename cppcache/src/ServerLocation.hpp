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

class APACHE_GEODE_EXPORT ServerLocation
    : public internal::DataSerializableInternal {
 public:
  ServerLocation(std::string serverName, int port);

  ServerLocation();

   ServerLocation(std::string name);

  const std::string& getServerName() const;

  void setServername(std::string serverName);

  int getPort() const;

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  size_t objectSize() const override;

  void printInfo();

  bool operator<(const ServerLocation rhs) const;

  bool operator==(const ServerLocation& rhs) const;

  bool isValid() const;

  const std::string& getEpString();

  int hashcode() const;

  void makeEpString();

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
