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

/*
 * @brief User class for testing the put functionality for object.
 */

#pragma once

#ifndef __POSITION_HPP__
#define __POSITION_HPP__

#include <string>

#ifdef _WIN32
#ifdef BUILD_TESTOBJECT
#define TESTOBJECT_EXPORT LIBEXP
#else
#define TESTOBJECT_EXPORT LIBIMP
#endif
#else
#define TESTOBJECT_EXPORT
#endif

using namespace apache::geode::client;

namespace testobject {

class TESTOBJECT_EXPORT Position : public apache::geode::client::Serializable {
 private:
  int64_t avg20DaysVol;
  std::shared_ptr<CacheableString> bondRating;
  double convRatio;
  std::shared_ptr<CacheableString> country;
  double delta;
  int64_t industry;
  int64_t issuer;
  double mktValue;
  double qty;
  std::shared_ptr<CacheableString> secId;
  std::shared_ptr<CacheableString> secLinks;
  // wchar_t* secType;
  wchar_t* secType;
  int32_t sharesOutstanding;
  std::shared_ptr<CacheableString> underlyer;
  int64_t volatility;
  int32_t pid;

 public:
  static int32_t cnt;

  Position();
  Position(const char* id, int32_t out);
  virtual ~Position();
  virtual void toData(apache::geode::client::DataOutput& output) const;
  virtual void fromData(apache::geode::client::DataInput& input);
  virtual int32_t classId() const { return 0x02; }
  std::string toString() const;

  virtual uint32_t objectSize() const {
    uint32_t objectSize = sizeof(Position);
    objectSize += bondRating->objectSize();
    objectSize += country->objectSize();
    objectSize += secId->objectSize();
    objectSize += secLinks->objectSize();
    objectSize += (uint32_t)(sizeof(wchar_t) * wcslen(secType));
    objectSize += underlyer->objectSize();
    return objectSize;
  }

  static void resetCounter() { cnt = 0; }
  std::shared_ptr<CacheableString> getSecId() { return secId; }
  int32_t getId() { return pid; }
  int32_t getSharesOutstanding() { return sharesOutstanding; }
  static apache::geode::client::Serializable* createDeserializable() {
    return new Position();
  }

 private:
  void init();
};

}  // namespace testobject
#endif
