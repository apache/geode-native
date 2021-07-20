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

#include "ArrayOfByte.hpp"

#include <fwklib/FwkException.hpp>
#include <fwklib/GsRandom.hpp>
#include <sstream>

namespace testobject {

using apache::geode::client::DataInputInternal;
using apache::geode::client::DataOutputInternal;
using apache::geode::client::Exception;
using apache::geode::client::testframework::FwkException;
using apache::geode::client::testframework::GsRandom;

std::shared_ptr<CacheableBytes> ArrayOfByte::init(int size, bool encodeKey,
                                                  bool encodeTimestamp) {
  if (encodeKey) {
    DataOutputInternal dos;
    try {
      int32_t index = 1234;
      dos.writeInt(index);
      if (encodeTimestamp) {
        dos.writeInt(
            std::chrono::system_clock::now().time_since_epoch().count());
      }
    } catch (Exception &e) {
      FWKEXCEPTION("Unable to write to stream " << e.what());
    }
    int32_t bufSize = size;
    char *buf = new char[bufSize];
    memset(buf, 'V', bufSize);
    int32_t rsiz = (bufSize <= 20) ? bufSize : 20;
    GsRandom::getAlphanumericString(rsiz, buf);
    memcpy(buf, dos.getBuffer(), dos.getBufferLength());
    return CacheableBytes::create(std::vector<int8_t>(buf, buf + bufSize));
  } else if (encodeTimestamp) {
    FWKEXCEPTION("Should not happen");
  } else {
    return CacheableBytes::create(std::vector<int8_t>(size));
  }
}

int64_t ArrayOfByte::getTimestamp(std::shared_ptr<CacheableBytes> bytes) {
  if (bytes == nullptr) {
    throw apache::geode::client::IllegalArgumentException(
        "the bytes arg was null");
  }
  DataInputInternal di(reinterpret_cast<const uint8_t *>(bytes->value().data()),
                       bytes->length(), nullptr);
  try {
    di.readInt32();
    int64_t timestamp = di.readInt64();
    if (timestamp == 0) {
      FWKEXCEPTION("Object is not configured to encode timestamp");
    }
    return timestamp;
  } catch (Exception &e) {
    FWKEXCEPTION("Unable to read from stream " << e.what());
  }
}

void ArrayOfByte::resetTimestamp(std::shared_ptr<CacheableBytes> bytes) {
  DataInputInternal di(reinterpret_cast<const uint8_t *>(bytes->value().data()),
                       bytes->length(), nullptr);
  int32_t index;
  try {
    index = di.readInt32();
    int64_t timestamp = di.readInt64();
    if (timestamp == 0) {
      return;
    }
  } catch (Exception &e) {
    FWKEXCEPTION("Unable to read from stream " << e.what());
  }
  DataOutputInternal dos;
  try {
    dos.writeInt(index);
    dos.writeInt(std::chrono::system_clock::now().time_since_epoch().count());
  } catch (Exception &e) {
    FWKEXCEPTION("Unable to write to stream " << e.what());
  }
}

}  // namespace testobject
