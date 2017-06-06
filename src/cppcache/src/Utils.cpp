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

#include <cstdio>
#include <chrono>

#include <ace/OS.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/INET_Addr.h>

#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

int RandGen::operator()(size_t max) {
  unsigned int seed = static_cast<unsigned int>(
      std::chrono::system_clock::now().time_since_epoch().count());
  return ACE_OS::rand_r(&seed) % max;
}

int32_t Utils::getLastError() { return ACE_OS::last_error(); }
std::string Utils::getEnv(const char* varName) {
#ifdef _WIN32
  DWORD dwRet;
  char varValue[8192];
  dwRet = ::GetEnvironmentVariable(varName, varValue, 8192);
  if (dwRet == 0 && (::GetLastError() == ERROR_ENVVAR_NOT_FOUND)) {
    return "";
  }
  return varValue;
#else
  char* varValue = ACE_OS::getenv(varName);
  if (varValue == nullptr) {
    return "";
  }
  return varValue;
#endif
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
  ACE_TCHAR hostName[256], fullName[256];
  pos = endpointsStr.find(':', 0);
  if (pos != std::string::npos) {
    endpoint = endpointsStr.substr(0, pos);
    pos += 1;  // skip ':'
    length -= (pos);
    endpointsStr = endpointsStr.substr(pos, length);
  } else {
    hostString = "";
    port = 0;
    return "";
  }
  hostString = endpoint;
  port = atoi(endpointsStr.c_str());
  if (strcmp(hostString.c_str(), "localhost") == 0) {
    ACE_OS::hostname(hostName, sizeof(hostName) - 1);
    struct hostent* host;
    host = ACE_OS::gethostbyname(hostName);
    if (host) {
      ACE_OS::snprintf(fullName, 256, "%s:%d", host->h_name, port);
      return fullName;
    }
  } else {
    pos = endpointsStr1.find('.', 0);
    if (pos != std::string::npos) {
      ACE_INET_Addr addr(endpoints);
      addr.get_host_name(hostName, 256);
      ACE_OS::snprintf(fullName, 256, "%s:%d", hostName, port);
      return fullName;
    }
  }
  return endpoints;
}

void Utils::parseEndpointNamesString(
    const char* endpoints, std::unordered_set<std::string>& endpointNames) {
  std::string endpointsStr(endpoints);
  // Parse this string to get all hostnames and port numbers.
  std::string endpoint;
  std::string::size_type length = endpointsStr.size();
  std::string::size_type pos = 0;
  do {
    pos = endpointsStr.find(',', 0);
    if (pos != std::string::npos) {
      endpoint = endpointsStr.substr(0, pos);
      pos += 1;  // skip ','
      length -= (pos);
      endpointsStr = endpointsStr.substr(pos, length);
    } else {
      endpoint = endpointsStr;
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

CacheableStringPtr Utils::convertBytesToString(const uint8_t* bytes,
                                               int32_t length,
                                               size_t maxLength) {
  if (bytes != nullptr) {
    std::string str;
    size_t totalBytes = 0;
    char byteStr[20];
    for (int32_t index = 0; index < length; ++index) {
      int len = ACE_OS::snprintf(byteStr, 20, "%d ", bytes[index]);
      totalBytes += len;
      // no use going beyond maxLength since LOG* methods will truncate
      // in any case
      if (maxLength > 0 && totalBytes > maxLength) {
        break;
      }
      str.append(byteStr, len);
    }
    return CacheableString::create(str.data(),
                                   static_cast<int32_t>(str.size()));
  }
  return CacheableString::create("");
}

int32_t Utils::logWideString(char* buf, size_t maxLen, const wchar_t* wStr) {
  if (wStr != nullptr) {
    mbstate_t state;
    ACE_OS::memset(&state, 0, sizeof(mbstate_t));
    const char* bufStart = buf;
    do {
      if (maxLen < static_cast<size_t>(MB_CUR_MAX)) {
        break;
      }
      size_t numChars = wcrtomb(buf, *wStr, &state);
      if (numChars == static_cast<size_t>(-1)) {
        // write short when conversion cannot be done
        numChars = ACE_OS::snprintf(buf, maxLen, "<%u>", *wStr);
      }
      buf += numChars;
      if (numChars >= maxLen) {
        break;
      }
      maxLen -= numChars;
    } while (*wStr++ != L'\0');
    return static_cast<int32_t>(buf - bufStart);
  } else {
    return ACE_OS::snprintf(buf, maxLen, "null");
  }
}

int64_t Utils::startStatOpTime() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

void Utils::updateStatOpTime(statistics::Statistics* m_regionStats, int32_t statId,
                             int64_t start) {
  m_regionStats->incLong(statId, startStatOpTime() - start);
}

}  // namespace client
}  // namespace geode
}  // namespace apache