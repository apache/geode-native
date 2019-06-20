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
  BucketServerLocation();

  explicit BucketServerLocation(std::string host);

  BucketServerLocation(int bucketId, int port, std::string host, bool isPrimary,
                       int8_t version);

  BucketServerLocation(int bucketId, int port, std::string host, bool isPrimary,
                       int8_t version, std::vector<std::string> serverGroups);

  int getBucketId() const;

  bool isPrimary() const;

  int8_t getVersion() const;

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  size_t objectSize() const override;

  BucketServerLocation& operator=(const BucketServerLocation& rhs);

  BucketServerLocation(const BucketServerLocation& rhs);

  std::shared_ptr<CacheableStringArray> getServerGroups();
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
