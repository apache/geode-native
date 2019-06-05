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

#include "BucketServerLocation.hpp"

namespace apache {
namespace geode {
namespace client {
BucketServerLocation::BucketServerLocation()
      : ServerLocation(),
        m_bucketId(-1),
        m_isPrimary(false),
        m_version(0),
        m_serverGroups(nullptr),
        m_numServerGroups(static_cast<int8_t>(0)) {}

BucketServerLocation::BucketServerLocation(std::string host)
      : ServerLocation(host),
        m_bucketId(-1),
        m_isPrimary(false),
        m_version(0),
        m_serverGroups(nullptr),
        m_numServerGroups(static_cast<int8_t>(0)) {}

BucketServerLocation::BucketServerLocation(int bucketId, int port, std::string host, bool isPrimary,
                       int8_t version)
      : ServerLocation(host, port),
        m_bucketId(bucketId),
        m_isPrimary(isPrimary),
        m_version(version),
        m_serverGroups(nullptr),
        m_numServerGroups(static_cast<int8_t>(0)) {}

BucketServerLocation::BucketServerLocation(int bucketId, int port, std::string host, bool isPrimary,
                       int8_t version, std::vector<std::string> serverGroups)
      : ServerLocation(host, port),
        m_bucketId(bucketId),
        m_isPrimary(isPrimary),
        m_version(version) {
    int32_t size = static_cast<int32_t>(serverGroups.size());
    std::shared_ptr<CacheableString>* ptrArr = nullptr;
    if (size > 0) {
      ptrArr = new std::shared_ptr<CacheableString>[size];
      for (int i = 0; i < size; i++) {
        ptrArr[i] = CacheableString::create(serverGroups[i]);
      }
    }
    if (size > 0) {
      if (size > 0x7f) {
        // TODO:  should fail here since m_numServerGroups is int8_t?
      }
      m_serverGroups = CacheableStringArray::create(
          std::vector<std::shared_ptr<CacheableString>>(ptrArr, ptrArr + size));
      m_numServerGroups = static_cast<int8_t>(size);
    } else {
      m_serverGroups = nullptr;
      m_numServerGroups = static_cast<int8_t>(0);
    }
  }

int BucketServerLocation::getBucketId() const { return m_bucketId; }

bool BucketServerLocation::isPrimary() const { return m_isPrimary; }

int8_t BucketServerLocation::getVersion() const { return m_version; }

void BucketServerLocation::toData(DataOutput& output) const {
    ServerLocation::toData(output);
    output.writeInt(m_bucketId);
    output.writeBoolean(m_isPrimary);
    output.write(m_version);
    output.write(static_cast<int8_t>(m_numServerGroups));
    if (m_numServerGroups > 0) {
      for (int i = 0; i < m_numServerGroups; i++) {
        output.writeObject(m_serverGroups->value()[i]);
      }
    }
  }

void BucketServerLocation::fromData(DataInput& input) {
    ServerLocation::fromData(input);
    m_bucketId = input.readInt32();
    m_isPrimary = input.readBoolean();
    m_version = input.read();
    m_numServerGroups = input.read();
    std::shared_ptr<CacheableString>* serverGroups = nullptr;
    if (m_numServerGroups > 0) {
      serverGroups = new std::shared_ptr<CacheableString>[m_numServerGroups];
      for (int i = 0; i < m_numServerGroups; i++) {
        serverGroups[i] = CacheableString::create(input.readString());
      }
    }
    if (m_numServerGroups > 0) {
      m_serverGroups = CacheableStringArray::create(
          std::vector<std::shared_ptr<CacheableString>>(
              serverGroups, serverGroups + m_numServerGroups));
    }
  }

size_t BucketServerLocation::objectSize() const {
    return sizeof(int32_t) + sizeof(bool) + sizeof(int8_t);
  }

BucketServerLocation& BucketServerLocation::operator=(const BucketServerLocation& rhs) {
    if (this == &rhs) return *this;
    m_serverName = rhs.m_serverName;
    m_port = rhs.m_port;
    //(ServerLocation&)*this = rhs;
    m_bucketId = rhs.m_bucketId;
    m_isPrimary = rhs.m_isPrimary;
    m_version = rhs.m_version;
    m_numServerGroups = rhs.m_numServerGroups;
    m_serverGroups = rhs.m_serverGroups;
    return *this;
  }

BucketServerLocation::BucketServerLocation(
      const BucketServerLocation&
          rhs)  //:ServerLocation(rhs.getServerName(),rhs.getPort())
  {
    m_serverName = rhs.m_serverName;
    m_port = rhs.m_port;
    m_bucketId = rhs.m_bucketId;
    m_isPrimary = rhs.m_isPrimary;
    m_version = rhs.m_version;
    m_numServerGroups = rhs.m_numServerGroups;
    m_serverGroups = rhs.m_serverGroups;
  }

std::shared_ptr<CacheableStringArray> BucketServerLocation::getServerGroups() {
    return m_serverGroups;
  }

}  // namespace client
}  // namespace geode
}  // namespace apache
