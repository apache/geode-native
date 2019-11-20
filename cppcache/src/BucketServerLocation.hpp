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

#ifndef GEODE_BUCKETSERVERLOCATION_H_
#define GEODE_BUCKETSERVERLOCATION_H_

#include <string>

#include <geode/CacheableBuiltins.hpp>

#include "ServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {

class BucketServerLocation : public ServerLocation {
 private:
  int m_bucketId;
  bool m_isPrimary;
  int8_t m_version;
  std::shared_ptr<CacheableStringArray> m_serverGroups;
  int8_t m_numServerGroups;

 public:
  BucketServerLocation()
      : ServerLocation(),
        m_bucketId(-1),
        m_isPrimary(false),
        m_version(0),
        m_serverGroups(nullptr),
        m_numServerGroups(static_cast<int8_t>(0)) {}

  explicit BucketServerLocation(std::string host)
      : ServerLocation(host),
        m_bucketId(-1),
        m_isPrimary(false),
        m_version(0),
        m_serverGroups(nullptr),
        m_numServerGroups(static_cast<int8_t>(0)) {}

  BucketServerLocation(int bucketId, int port, std::string host, bool isPrimary,
                       int8_t version)
      : ServerLocation(host, port),
        m_bucketId(bucketId),
        m_isPrimary(isPrimary),
        m_version(version),
        m_serverGroups(nullptr),
        m_numServerGroups(static_cast<int8_t>(0)) {}

  BucketServerLocation(int bucketId, int port, std::string host, bool isPrimary,
                       int8_t version, std::vector<std::string> serverGroups)
      : ServerLocation(host, port),
        m_bucketId(bucketId),
        m_isPrimary(isPrimary),
        m_version(version) {
    int32_t size = static_cast<int32_t>(serverGroups.size());
    if (size > 0) {
      std::vector<std::shared_ptr<CacheableString>> tmpServerGroups;
      tmpServerGroups.reserve(size);
      for (auto&& serverGroup : serverGroups) {
        tmpServerGroups.emplace_back(CacheableString::create(serverGroup));
      }
      if (size > 0x7f) {
        // TODO:  should fail here since m_numServerGroups is int8_t?
      }
      m_serverGroups = CacheableStringArray::create(std::move(tmpServerGroups));
      m_numServerGroups = static_cast<int8_t>(size);
    } else {
      m_serverGroups = nullptr;
      m_numServerGroups = static_cast<int8_t>(0);
    }
  }

  inline int getBucketId() const { return m_bucketId; }

  inline bool isPrimary() const { return m_isPrimary; }

  inline int8_t getVersion() const { return m_version; }

  void toData(DataOutput& output) const override {
    ServerLocation::toData(output);
    output.writeInt(m_bucketId);
    output.writeBoolean(m_isPrimary);
    output.write(m_version);
    output.write(static_cast<int8_t>(m_numServerGroups));
    if (m_numServerGroups > 0) {
      for (auto&& serverGroup : m_serverGroups->value()) {
        output.writeObject(serverGroup);
      }
    }
  }

  void fromData(DataInput& input) override {
    ServerLocation::fromData(input);
    m_bucketId = input.readInt32();
    m_isPrimary = input.readBoolean();
    m_version = input.read();
    m_numServerGroups = input.read();

    if (m_numServerGroups > 0) {
      std::vector<std::shared_ptr<CacheableString>> serverGroups;
      serverGroups.reserve(m_numServerGroups);
      for (int i = 0; i < m_numServerGroups; i++) {
        serverGroups.emplace_back(CacheableString::create(input.readString()));
      }
      m_serverGroups = CacheableStringArray::create(std::move(serverGroups));
    }
  }

  size_t objectSize() const override {
    return sizeof(int32_t) + sizeof(bool) + sizeof(int8_t);
  }

  BucketServerLocation& operator=(const BucketServerLocation& rhs) {
    if (this == &rhs) return *this;
    this->m_serverName = rhs.m_serverName;
    this->m_port = rhs.m_port;
    //(ServerLocation&)*this = rhs;
    this->m_bucketId = rhs.m_bucketId;
    this->m_isPrimary = rhs.m_isPrimary;
    this->m_version = rhs.m_version;
    this->m_numServerGroups = rhs.m_numServerGroups;
    this->m_serverGroups = rhs.m_serverGroups;
    return *this;
  }

  BucketServerLocation(
      const BucketServerLocation&
          rhs)  //:ServerLocation(rhs.getServerName(),rhs.getPort())
  {
    this->m_serverName = rhs.m_serverName;
    this->m_port = rhs.m_port;
    this->m_bucketId = rhs.m_bucketId;
    this->m_isPrimary = rhs.m_isPrimary;
    this->m_version = rhs.m_version;
    this->m_numServerGroups = rhs.m_numServerGroups;
    this->m_serverGroups = rhs.m_serverGroups;
  }

  inline std::shared_ptr<CacheableStringArray> getServerGroups() {
    return m_serverGroups;
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::BucketServerLocation> {
  typedef apache::geode::client::BucketServerLocation argument_type;
  typedef size_t result_type;
  size_t operator()(
      const apache::geode::client::BucketServerLocation& val) const {
    return val.hashcode();
  }
};

}  // namespace std

#endif  // GEODE_BUCKETSERVERLOCATION_H_
