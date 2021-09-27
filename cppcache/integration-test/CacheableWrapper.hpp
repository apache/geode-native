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

#ifndef GEODE_INTEGRATION_TEST_CACHEABLEWRAPPER_H_
#define GEODE_INTEGRATION_TEST_CACHEABLEWRAPPER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>

#include <geode/CacheableBuiltins.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testing {

using apache::geode::client::Cacheable;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::internal::DSCode;

class CacheableWrapper {
 protected:
  std::shared_ptr<Cacheable> m_cacheableObject;

 public:
  explicit CacheableWrapper(const std::shared_ptr<Cacheable> cacheableObject)
      : m_cacheableObject(cacheableObject) {}

  virtual std::shared_ptr<Cacheable> getCacheable() const {
    return m_cacheableObject;
  }

  virtual int maxKeys() const {
    throw IllegalArgumentException("Cannot call maxKeys.");
  }

  virtual void initKey(int32_t keyIndex, int32_t maxSize);

  virtual void initRandomValue(int maxSize) = 0;

  virtual uint32_t getCheckSum(
      const std::shared_ptr<Cacheable> object) const = 0;

  uint32_t getCheckSum() const { return getCheckSum(m_cacheableObject); }

  virtual ~CacheableWrapper() {}

protected:
  std::string zeroPaddedStringFromIndex(int32_t indexVal) {
    std::ostringstream indexStr;
    indexStr << std::setw(10) << std::setfill('0') << indexVal;
    return indexStr.str();
  }
};

typedef CacheableWrapper* (*CacheableWrapperFunc)(void);

class CacheableWrapperFactory {
 public:
  static CacheableWrapper* createInstance(DSCode typeId);

  static void registerType(DSCode typeId, const std::string wrapperType,
                           const CacheableWrapperFunc wrapperFunc,
                           const bool isKey);

  static std::vector<DSCode> getRegisteredKeyTypes();

  static std::vector<DSCode> getRegisteredValueTypes();

  static std::string getTypeForId(DSCode typeId);

 private:
  static std::map<DSCode, CacheableWrapperFunc> m_registeredKeyMap;
  static std::map<DSCode, CacheableWrapperFunc> m_registeredValueMap;
  static std::map<DSCode, std::string> m_typeIdNameMap;
};

}  // namespace testing
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTEGRATION_TEST_CACHEABLEWRAPPER_H_
