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

#include <ace/DLL.h>
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

void* Utils::getFactoryFunction(const std::string& lib,
                                const std::string& funcName) {
  ACE_DLL dll;
  if (dll.open(lib.c_str(), ACE_DEFAULT_SHLIB_MODE, 0) == -1) {
    throw IllegalArgumentException("cannot open library: " + lib);
  }
  void* func = dll.symbol(funcName.c_str());
  if (func == nullptr) {
    throw IllegalArgumentException("cannot find factory function " + funcName +
                                   " in library " + lib);
  }
  return func;
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

std::string Utils::convertBytesToString(const int8_t* bytes, size_t length,
                                        size_t maxLength) {
  return Utils::convertBytesToString(reinterpret_cast<const uint8_t*>(bytes),
                                     length, maxLength);
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

}  // namespace client
}  // namespace geode
}  // namespace apache
