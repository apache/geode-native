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

#include "fw_helper.hpp"

#include <geode/SystemProperties.hpp>
#include <geode/Properties.hpp>
#include <geode/CacheableString.hpp>

#ifndef WIN32
#include <unistd.h>
#endif

using apache::geode::client::LogLevel;
using apache::geode::client::Properties;
using apache::geode::client::SystemProperties;

bool checkSecurityProperties(std::shared_ptr<Properties> securityProperties,
                             const char *key, const char *value) {
  bool flag;
  if (key == nullptr || value == nullptr) {
    return false;
  }
  auto tempValue = securityProperties->find(key);
  if (tempValue == nullptr) {
    return (false);
  }
  flag = strcmp(tempValue->value().c_str(), value);
  return (!flag);
}

BEGIN_TEST(DEFAULT)
  {
    SystemProperties *systemProperties =
        new SystemProperties(nullptr, "./non-existent");
    ASSERT(
        systemProperties->statisticsSampleInterval() == std::chrono::seconds(1),
        "expected 1");
    ASSERT(systemProperties->statisticsEnabled() == false, "expected false");
    LOG(systemProperties->statisticsArchiveFile());
    auto &&statisticsArchiveFileName =
        systemProperties->statisticsArchiveFile();
    ASSERT(statisticsArchiveFileName == "statArchive.gfs",
           "Expected statisticsArchiveFileName == \"statArchive.gfs\"");
    delete systemProperties;
  }
END_TEST(DEFAULT)

BEGIN_TEST(NEW_CONFIG)
  {
    // When the tests are run from the build script the environment variable
    // TESTSRC is set.
    std::string testSource(std::getenv("TESTSRC"));
    std::string filePath = testSource + "/resources/system.properties";

    SystemProperties *systemProperties =
        new SystemProperties(nullptr, filePath);

    ASSERT(systemProperties->statisticsSampleInterval() ==
               std::chrono::seconds(700),
           "expected 700");

    ASSERT(systemProperties->statisticsEnabled() == true, "expected true");

    ASSERT(systemProperties->threadPoolSize() == 96,
           "max-fe-thread should be 96");

    auto &&statisticsArchiveFileName =
        systemProperties->statisticsArchiveFile();
    ASSERT1(statisticsArchiveFileName == "stats.gfs");

    auto &&logFilename = systemProperties->logFilename();
    ASSERT1(logFilename == "geode-native.log");

    auto &&name = systemProperties->name();
    ASSERT1(name == "system");

    auto &&cacheXMLFileName = systemProperties->cacheXMLFile();
    ASSERT1(cacheXMLFileName == "cache.xml");

    ASSERT(systemProperties->pingInterval() == std::chrono::seconds(123),
           "expected 123 pingInterval");
    ASSERT(systemProperties->redundancyMonitorInterval() ==
               std::chrono::seconds(456),
           "expected 456s redundancyMonitorInterval");

    ASSERT(systemProperties->heapLRULimit() == 100, "expected 100");
    ASSERT(systemProperties->heapLRUDelta() == 10, "expected 10");

    ASSERT(systemProperties->notifyAckInterval() ==
               std::chrono::milliseconds(1234),
           "expected 1234 notifyAckInterval");
    ASSERT(systemProperties->notifyDupCheckLife() ==
               std::chrono::milliseconds(4321),
           "expected 4321 notifyDupCheckLife");

    ASSERT(systemProperties->logFileSizeLimit() == 1024000000,
           "expected 1024000000");

    ASSERT(systemProperties->statsFileSizeLimit() == 1024000000,
           "expected 1024000000");

    auto &&durableId = systemProperties->durableClientId();
    ASSERT1(durableId == "testDurableId");

    ASSERT(systemProperties->durableTimeout() == std::chrono::seconds(123),
           "expected 123 durableTimeOut");

    ASSERT(systemProperties->connectTimeout() == std::chrono::milliseconds(345),
           "expected 345 for connect timeout");

    auto securityProperties = systemProperties->getSecurityProperties();
    ASSERT(checkSecurityProperties(securityProperties, "security-username",
                                   "username") == true,
           "SecurityProperties Not Stored");
    ASSERT(checkSecurityProperties(securityProperties, "security-password",
                                   "password") == true,
           "SecurityProperties Not Stored");

    delete systemProperties;
  }
END_TEST(NEW_CONFIG)
