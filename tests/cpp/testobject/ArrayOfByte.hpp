#pragma once

#ifndef GEODE_TESTOBJECT_ARRAYOFBYTE_H_
#define GEODE_TESTOBJECT_ARRAYOFBYTE_H_

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

#include <string>
#include "fwklib/FwkLog.hpp"
#include "fwklib/FrameworkTest.hpp"
#include <ace/Time_Value.h>

#include "SerializationRegistry.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"

#ifdef _WIN32
#ifdef BUILD_TESTOBJECT
#define TESTOBJECT_EXPORT _GEODE_LIBEXP
#else
#define TESTOBJECT_EXPORT _GEODE_LIBIMP
#endif
#else
#define TESTOBJECT_EXPORT
#endif

using namespace apache::geode::client;
using namespace testframework;

namespace testobject {

class TESTOBJECT_EXPORT ArrayOfByte {
 public:
  static std::shared_ptr<CacheableBytes> init(int size, bool encodeKey,
                                              bool encodeTimestamp) {
    if (encodeKey) {
      DataOutputInternal dos;
      try {
        int32_t index = 1234;
        dos.writeInt(index);
        if (encodeTimestamp) {
          ACE_Time_Value startTime;
          startTime = ACE_OS::gettimeofday();
          ACE_UINT64 tusec = 0;
          startTime.to_usec(tusec);
          int64_t timestamp = tusec * 1000;
          dos.writeInt(timestamp);
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
      return CacheableBytes::create(std::vector<int8_t>(buf, buf +
                                          bufSize));
    } else if (encodeTimestamp) {
      FWKEXCEPTION("Should not happen");
    } else {
      return CacheableBytes::create(std::vector<int8_t>(size));
    }
  }

  static int64_t getTimestamp(std::shared_ptr<CacheableBytes> bytes,
                              SerializationRegistry &serializationRegistry) {
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

  static void resetTimestamp(std::shared_ptr<CacheableBytes> bytes,
                             SerializationRegistry &serializationRegistry) {
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
      ACE_Time_Value startTime;
      startTime = ACE_OS::gettimeofday();
      ACE_UINT64 tusec = 0;
      startTime.to_usec(tusec);
      int64_t timestamp = tusec * 1000;
      dos.writeInt(timestamp);
    } catch (Exception &e) {
      FWKEXCEPTION("Unable to write to stream " << e.what());
    }
  }
};
}  // namespace testobject

#endif  // GEODE_TESTOBJECT_ARRAYOFBYTE_H_
