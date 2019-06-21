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

#ifndef GEODE_INTERNAL_UTILS_H_
#define GEODE_INTERNAL_UTILS_H_

#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <typeinfo>
#include <unordered_set>

#include <geode/CacheableString.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/internal/geode_base.hpp>
#include <geode/internal/geode_globals.hpp>

#include "DistributedSystem.hpp"
#include "statistics/Statistics.hpp"
#include "util/Log.hpp"

#ifdef __GNUC__
extern "C" {
#include <cxxabi.h>
}
#endif

namespace apache {
namespace geode {
namespace client {
class APACHE_GEODE_EXPORT Utils {
  /**
   * utilities
   *
   */
 public:
  /**
   * Get the value of an environment variable.
   * On windows the maximum length of value supported is 8191.
   */
  static std::string getEnv(const char* varName);
  static int32_t getLastError();

#ifdef __GNUC__
  static char* _gnuDemangledName(const char* typeIdName, size_t& len);
#endif

  static void demangleTypeName(const char* typeIdName, std::string& str);

  static std::string demangleTypeName(const char* typeIdName);

  /**
   * The only operations that is well defined on the result is "asChar".
   */
  static std::string nullSafeToString(const std::shared_ptr<CacheableKey>& key);

  static std::string nullSafeToString(const std::shared_ptr<Cacheable>& val);

  static int64_t startStatOpTime();

  // Check objectSize() implementation return value and log a warning at most
  // once.
  static size_t checkAndGetObjectSize(
      const std::shared_ptr<Cacheable>& theObject);

  static void updateStatOpTime(statistics::Statistics* m_regionStats,
                               int32_t statId, int64_t start);

  static void parseEndpointNamesString(
      std::string endpoints, std::unordered_set<std::string>& endpointNames);

  static void parseEndpointString(const char* endpoints, std::string& host,
                                  uint16_t& port);

  static std::string convertHostToCanonicalForm(const char* endpoints);

  static char* copyString(const char* str);

  /**
   * Convert the byte array to a string as "%d %d ...".
   * <code>maxLength</code> as zero implies no limit.
   */
  static std::string convertBytesToString(const uint8_t* bytes, size_t length,
                                          size_t maxLength = _GF_MSG_LIMIT);

  /**
   * Convert the byte array to a string as "%d %d ...".
   * <code>maxLength</code> as zero implies no limit.
   */
  static std::string convertBytesToString(const char* bytes, size_t length,
                                          size_t maxLength = _GF_MSG_LIMIT);
};

// Generate random numbers 0 to max-1
class RandGen {
 public:
  template <typename T, class G = std::default_random_engine>
  inline T operator()(T max) {
    return std::uniform_int_distribution<T>{0, max - 1}(generator<G>());
  }

 private:
  template <class G>
  inline G& generator() {
    static thread_local G generator(std::random_device{}());
    return generator;
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTERNAL_UTILS_H_
