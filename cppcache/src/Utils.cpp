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

#include "Utils.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include <ace/INET_Addr.h>
#include <ace/OS.h>

namespace apache {
namespace geode {
namespace client {

int32_t Utils::getLastError() { return ACE_OS::last_error(); }

std::string Utils::getEnv(const char* varName) {
  std::string env;
  if (const auto varValue = std::getenv(varName)) {
    env = varValue;
  }
  return env;
}

void Utils::parseEndpointString(const char* endpoints, std::string& host,
                                uint16_t& port) {
  std::string endpointsStr(endpoints);
  LOGFINE("Parsing endpoint string [%s]", endpointsStr.c_str());
  // Parse this string to get all hostnames and port numbers.
  std::string endpoint;
  std::string::size_type length = endpointsStr.size();
  std::string::size_type pos = 0;
  pos = endpointsStr.find(':', 0);
  if (pos != std::string::npos) {
    endpoint = endpointsStr.substr(0, pos);
    pos += 1;  // skip ':'
    length -= (pos);
    endpointsStr = endpointsStr.substr(pos, length);
  } else {
    host = "";
    port = 0;
    return;
  }
  // trim white spaces.
  std::string::size_type wpos = endpoint.find_last_not_of(' ');
  if (wpos != std::string::npos) {
    endpoint.erase(wpos + 1);
    wpos = endpoint.find_first_not_of(' ');
    if (wpos != std::string::npos) endpoint.erase(0, wpos);
  }
  host = endpoint;
  port = atoi(endpointsStr.c_str());
}

std::string Utils::convertHostToCanonicalForm(const char* endpoints) {
  if (endpoints == nullptr) return nullptr;
  std::string hostString("");
  uint16_t port = 0;
  std::string endpointsStr(endpoints);
  std::string endpointsStr1(endpoints);
  // Parse this string to get all hostnames and port numbers.
  std::string endpoint;
  std::string::size_type length = endpointsStr.size();
  std::string::size_type pos = 0;
  ACE_TCHAR hostName[256], fullName[512];
  pos = endpointsStr.find(':', 0);
  if (pos != std::string::npos) {
    endpoint = endpointsStr.substr(0, pos);
    pos += 1;  // skip ':'
    length -= (pos);
    endpointsStr = endpointsStr.substr(pos, length);
  } else {
    hostString = "";
    return "";
  }
  hostString = endpoint;
  port = atoi(endpointsStr.c_str());
  if (strcmp(hostString.c_str(), "localhost") == 0) {
    ACE_OS::hostname(hostName, sizeof(hostName) - 1);
    struct hostent* host;
    host = ACE_OS::gethostbyname(hostName);
    if (host) {
      std::snprintf(fullName, sizeof(fullName), "%s:%d", host->h_name, port);
      return fullName;
    }
  } else {
    pos = endpointsStr1.find('.', 0);
    if (pos != std::string::npos) {
      ACE_INET_Addr addr(endpoints);
      addr.get_host_name(hostName, 256);
      std::snprintf(fullName, sizeof(fullName), "%s:%d", hostName, port);
      return fullName;
    }
  }
  return endpoints;
}

void Utils::parseEndpointNamesString(
    std::string endpoints, std::unordered_set<std::string>& endpointNames) {
  // Parse this string to get all hostnames and port numbers.
  std::string endpoint;
  std::string::size_type length = endpoints.size();
  std::string::size_type pos = 0;
  do {
    pos = endpoints.find(',', 0);
    if (pos != std::string::npos) {
      endpoint = endpoints.substr(0, pos);
      pos += 1;  // skip ','
      length -= (pos);
      endpoints = endpoints.substr(pos, length);
    } else {
      endpoint = endpoints;
    }
    // trim white spaces.
    std::string::size_type wpos = endpoint.find_last_not_of(' ');
    if (wpos != std::string::npos) {
      endpoint.erase(wpos + 1);
      wpos = endpoint.find_first_not_of(' ');
      if (wpos != std::string::npos) endpoint.erase(0, wpos);
      endpointNames.insert(endpoint);
    }
  } while (pos != std::string::npos);
}

char* Utils::copyString(const char* str) {
  char* resStr = nullptr;
  if (str != nullptr) {
    size_t strSize = strlen(str) + 1;
    resStr = new char[strSize];
    memcpy(resStr, str, strSize);
  }
  return resStr;
}

std::string Utils::convertBytesToString(const uint8_t* bytes, size_t length,
                                        size_t maxLength) {
  if (bytes != nullptr) {
    length = std::min(length, maxLength);
    std::stringstream ss;
    ss << std::setfill('0') << std::hex;
    for (size_t i = 0; i < length; ++i) {
      ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
  }
  return "";
}

int64_t Utils::startStatOpTime() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

void Utils::updateStatOpTime(statistics::Statistics* m_regionStats,
                             int32_t statId, int64_t start) {
  m_regionStats->incLong(statId, startStatOpTime() - start);
}

#ifdef __GNUC__
char* Utils::_gnuDemangledName(const char* typeIdName, size_t& len) {
  int status;
  char* demangledName = abi::__cxa_demangle(typeIdName, nullptr, &len, &status);
  if (status == 0 && demangledName != nullptr) {
    return demangledName;
  }
  return nullptr;
}
#endif

void Utils::demangleTypeName(const char* typeIdName, std::string& str) {
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

std::string Utils::demangleTypeName(const char* typeIdName) {
#ifdef __GNUC__
  size_t len;
  char* demangledName = _gnuDemangledName(typeIdName, len);
  if (demangledName != nullptr) {
    return std::string(demangledName, len);
  }
#endif
  return std::string(typeIdName);
}

std::string Utils::nullSafeToString(const std::shared_ptr<CacheableKey>& key) {
  std::string result;
  if (key) {
    result = key->toString();
  } else {
    result = "(null)";
  }
  return result;
}

std::string Utils::nullSafeToString(const std::shared_ptr<Cacheable>& val) {
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

size_t Utils::checkAndGetObjectSize(
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

std::string Utils::convertBytesToString(const char* bytes, size_t length,
                                        size_t maxLength) {
  return convertBytesToString(reinterpret_cast<const uint8_t*>(bytes), length,
                              maxLength);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
