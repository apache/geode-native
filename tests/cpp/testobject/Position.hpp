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

#ifndef GEODE_TESTOBJECT_POSITION_H_
#define GEODE_TESTOBJECT_POSITION_H_

/*
 * @brief User class for testing the put functionality for object.
 */

#include <string>

#include <geode/CacheableString.hpp>

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
  std::wstring secType;
  int32_t sharesOutstanding;
  std::shared_ptr<CacheableString> underlyer;
  int64_t volatility;
  int32_t pid;

  inline size_t getObjectSize(const std::shared_ptr<Serializable>& obj) const {
    return (obj == nullptr ? 0 : obj->objectSize());
  }

 public:
  static int32_t cnt;

  Position();
  Position(const char* id, int32_t out);
  // This constructor is just for some internal data validation test
  Position(int32_t iForExactVal);
  virtual ~Position();
  virtual void toData(apache::geode::client::DataOutput& output) const;
  virtual void fromData(apache::geode::client::DataInput& input);
  virtual int32_t classId() const { return 0x02; }
  std::string toString() const;

  virtual size_t objectSize() const {
    auto objectSize = sizeof(Position);
    objectSize += getObjectSize(bondRating);
    objectSize += getObjectSize(country);
    objectSize += getObjectSize(secId);
    objectSize += getObjectSize(secLinks);
    objectSize += secType.size() * sizeof(decltype(secType)::value_type);
    objectSize += getObjectSize(underlyer);
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

#endif  // GEODE_TESTOBJECT_POSITION_H_
