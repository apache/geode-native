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

#include "FixedPartitionAttributesImpl.hpp"

namespace apache {
namespace geode {
namespace client {
FixedPartitionAttributesImpl::FixedPartitionAttributesImpl()
    : Serializable(),
      m_partitionName(nullptr),
      m_isPrimary(false),
      m_numBuckets(1),
      m_startingBucketId(-1) {}

const std::string& FixedPartitionAttributesImpl::getPartitionName() {
  return m_partitionName;
}

int FixedPartitionAttributesImpl::getNumBuckets() const { return m_numBuckets; }

int FixedPartitionAttributesImpl::isPrimary() const { return m_isPrimary; }

void FixedPartitionAttributesImpl::toData(DataOutput& output) const {
  output.writeString(m_partitionName);
  output.writeBoolean(m_isPrimary);
  output.writeInt(m_numBuckets);
  output.writeInt(m_startingBucketId);
}

void FixedPartitionAttributesImpl::fromData(DataInput& input) {
  m_partitionName = input.readString();
  m_isPrimary = input.readBoolean();
  m_numBuckets = input.readInt32();
  m_startingBucketId = input.readInt32();
}

size_t FixedPartitionAttributesImpl::objectSize() const {
  return sizeof(int) + sizeof(int) + sizeof(bool) +
         (m_partitionName.length() *
          sizeof(decltype(m_partitionName)::value_type));
}

FixedPartitionAttributesImpl& FixedPartitionAttributesImpl::operator=(
    const FixedPartitionAttributesImpl& rhs) {
  if (this == &rhs) return *this;
  m_partitionName = rhs.m_partitionName;
  m_isPrimary = rhs.m_isPrimary;
  m_numBuckets = rhs.m_numBuckets;
  m_startingBucketId = rhs.m_startingBucketId;
  return *this;
}

FixedPartitionAttributesImpl::FixedPartitionAttributesImpl(
    const FixedPartitionAttributesImpl& rhs) {
  m_partitionName = rhs.m_partitionName;
  m_isPrimary = rhs.m_isPrimary;
  m_numBuckets = rhs.m_numBuckets;
  m_startingBucketId = rhs.m_startingBucketId;
}

int FixedPartitionAttributesImpl::getStartingBucketID() const {
  return m_startingBucketId;
}

int FixedPartitionAttributesImpl::getLastBucketID() const {
  return m_startingBucketId + m_numBuckets - 1;
}

bool FixedPartitionAttributesImpl::hasBucket(int bucketId) {
  return getStartingBucketID() <= bucketId && bucketId <= getLastBucketID();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
