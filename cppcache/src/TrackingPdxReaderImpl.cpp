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

#include "TrackingPdxReaderImpl.hpp"

#include "PdxType.hpp"
#include "PdxUnreadData.hpp"

namespace apache {
namespace geode {
namespace client {

TrackingPdxReaderImpl::TrackingPdxReaderImpl() : PdxReaderImpl{} {}

TrackingPdxReaderImpl::TrackingPdxReaderImpl(
    DataInput &input, std::shared_ptr<PdxType> remoteType, int32_t pdxLen)
    : PdxReaderImpl{input, remoteType, pdxLen} {
  auto n = pdxType_->getFieldsCount();
  unreadIndexes_.reserve(n);

  for (auto i = 0; i < n;) {
    unreadIndexes_.insert(i++);
  }
}

TrackingPdxReaderImpl::~TrackingPdxReaderImpl() = default;

char16_t TrackingPdxReaderImpl::readChar(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return '\0';
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readChar(field);
}

bool TrackingPdxReaderImpl::readBoolean(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return false;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readBoolean(field);
}

int8_t TrackingPdxReaderImpl::readByte(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readByte(field);
}

int16_t TrackingPdxReaderImpl::readShort(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readShort(field);
}

int32_t TrackingPdxReaderImpl::readInt(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readInt(field);
}

int64_t TrackingPdxReaderImpl::readLong(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readLong(field);
}

float TrackingPdxReaderImpl::readFloat(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0.f;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readFloat(field);
}

double TrackingPdxReaderImpl::readDouble(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return 0.;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readDouble(field);
}

std::shared_ptr<CacheableDate> TrackingPdxReaderImpl::readDate(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::shared_ptr<CacheableDate>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readDate(field);
}

std::string TrackingPdxReaderImpl::readString(const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::string{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readString(field);
}

std::shared_ptr<Serializable> TrackingPdxReaderImpl::readObject(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::shared_ptr<Serializable>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readObject(field);
}

std::vector<char16_t> TrackingPdxReaderImpl::readCharArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<char16_t>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readCharArray(field);
}

std::vector<bool> TrackingPdxReaderImpl::readBooleanArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<bool>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readBooleanArray(field);
}

std::vector<int8_t> TrackingPdxReaderImpl::readByteArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int8_t>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readByteArray(field);
}

std::vector<int16_t> TrackingPdxReaderImpl::readShortArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int16_t>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readShortArray(field);
}

std::vector<int32_t> TrackingPdxReaderImpl::readIntArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int32_t>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readIntArray(field);
}

std::vector<int64_t> TrackingPdxReaderImpl::readLongArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<int64_t>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readLongArray(field);
}

std::vector<float> TrackingPdxReaderImpl::readFloatArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<float>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readFloatArray(field);
}

std::vector<double> TrackingPdxReaderImpl::readDoubleArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<double>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readDoubleArray(field);
}

std::vector<std::string> TrackingPdxReaderImpl::readStringArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::vector<std::string>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readStringArray(field);
}

std::shared_ptr<CacheableObjectArray> TrackingPdxReaderImpl::readObjectArray(
    const std::string &name) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return std::shared_ptr<CacheableObjectArray>{};
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readObjectArray(field);
}

int8_t **TrackingPdxReaderImpl::readArrayOfByteArrays(const std::string &name,
                                                      int32_t &arrayLength,
                                                      int32_t **elementLength) {
  auto field = pdxType_->getField(name);
  if (!field) {
    return nullptr;
  }

  unreadIndexes_.erase(field->getIndex());
  return PdxReaderImpl::readArrayOfByteArrays(field, arrayLength,
                                              elementLength);
}

std::shared_ptr<PdxUnreadFields> TrackingPdxReaderImpl::readUnreadFields() {
  return getUnreadData();
}

std::shared_ptr<PdxUnreadData> TrackingPdxReaderImpl::getUnreadData() const {
  if(unreadIndexes_.empty()) {
    return std::shared_ptr<PdxUnreadData>{};
  }

  std::vector<std::vector<uint8_t>> data;
  std::vector<int32_t> indexes{unreadIndexes_.begin(), unreadIndexes_.end()};

  data.reserve(indexes.size());
  for (auto idx : indexes) {
    data.emplace_back(PdxReaderImpl::getRawFieldData(idx));
  }

  return std::make_shared<PdxUnreadData>(pdxType_, indexes, data);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
