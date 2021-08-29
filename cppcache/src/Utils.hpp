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

  static std::error_code getLastError();

#ifdef __GNUC__
  inline static char* _gnuDemangledName(const char* typeIdName, size_t& len) {
    int status;
    char* demangledName =
        abi::__cxa_demangle(typeIdName, nullptr, &len, &status);
    if (status == 0 && demangledName != nullptr) {
      return demangledName;
    }
    return nullptr;
  }
#endif

  inline static void demangleTypeName(const char* typeIdName,
                                      std::string& str) {
#ifdef __GNUC__
    size_t len;
    char* demangledName = _gnuDemangledName(typeIdName, len);
    if (demangledName != nullptr) {
      str.append(demangledName, len);
      free(demangledName);
      return;
    }
#endif
    str.append(typeIdName);
  }

  inline static std::string demangleTypeName(const char* typeIdName) {
#ifdef __GNUC__
    size_t len;
    char* demangledName = _gnuDemangledName(typeIdName, len);
    if (demangledName != nullptr) {
      auto str = std::string(demangledName, len);
      free(demangledName);
      return str;
    }
#endif
    return std::string(typeIdName);
  }

  /**
   * The only operations that is well defined on the result is "asChar".
   */
  inline static std::string nullSafeToString(
      const std::shared_ptr<CacheableKey>& key) {
    std::string result;
    if (key) {
      result = key->toString();
    } else {
      result = "(null)";
    }
    return result;
  }

  static std::string nullSafeToString(const std::shared_ptr<Cacheable>& val) {
    std::string result;
    if (val) {
      if (const auto key = std::dynamic_pointer_cast<CacheableKey>(val)) {
        result = nullSafeToString(key);
      } else {
        return result = val->toString();
      }
    } else {
      result = "(null)";
    }

    return result;
  }

  static int64_t startStatOpTime();

  // Check objectSize() implementation return value and log a warning at most
  // once.
  inline static size_t checkAndGetObjectSize(
      const std::shared_ptr<Cacheable>& theObject) {
    auto objectSize = theObject->objectSize();
    static bool youHaveBeenWarned = false;
    if (objectSize == 0 && !youHaveBeenWarned) {
      LOGWARN(
          "Object size for Heap LRU returned by %s is 0 (zero). Even for empty "
          "objects the size returned should be at least one (1 byte).",
          theObject->toString().c_str());
      youHaveBeenWarned = true;
    }
    return objectSize;
  }

  static void updateStatOpTime(statistics::Statistics* m_regionStats,
                               int32_t statId, int64_t start);

  static void parseEndpointNamesString(
      std::string endpoints, std::unordered_set<std::string>& endpointNames);

  static void parseEndpointString(const char* endpoints, std::string& host,
                                  uint16_t& port);

  static std::string convertHostToCanonicalForm(const char* endpoints);

  static std::string getSystemInfo();

  static char* copyString(const char* str);

  /**
   * Convert the byte array to a string as "%d %d ...".
   * <code>maxLength</code> as zero implies no limit.
   */
  static std::string convertBytesToString(
      const uint8_t* bytes, size_t length,
      size_t maxLength = Utils::bytesToStringMessageLimit_);

  /**
   * lib should be in the form originally required by ACE_DLL, typically just
   * like specifying a lib in java System.loadLibrary( "x" ); Where x is a
   * component of the name lib<x>.so on unix, or <x>.dll on windows.
   */
  template <typename T>
  static T* getFactoryFunction(const std::string& libraryName,
                               const std::string& functionName) {
    return reinterpret_cast<T*>(
        getFactoryFunctionVoid(libraryName, functionName));
  }

  /**
   * Convert the byte array to a string as "%d %d ...".
   * <code>maxLength</code> as zero implies no limit.
   */
  static std::string convertBytesToString(
      const int8_t* bytes, size_t length,
      size_t maxLength = Utils::bytesToStringMessageLimit_);

  /**
   * Convert the byte array to a string as "%d %d ...".
   * <code>maxLength</code> as zero implies no limit.
   */
  inline static std::string convertBytesToString(
      const char* bytes, size_t length,
      size_t maxLength = Utils::bytesToStringMessageLimit_) {
    return convertBytesToString(reinterpret_cast<const uint8_t*>(bytes), length,
                                maxLength);
  }

 private:
  static void* getFactoryFunctionVoid(const std::string& lib,
                                      const std::string& funcName);

  static const int bytesToStringMessageLimit_ = 8192;
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
