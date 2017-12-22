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

#define ROOT_NAME "testSystemProperties"

#include "fw_helper.hpp"
#include "geode/SystemProperties.hpp"
#include "geode/Properties.hpp"

#ifndef WIN32
#include <unistd.h>
#endif

using namespace apache::geode::client;

const bool checkSecurityProperties(
    std::shared_ptr<Properties> securityProperties, const char* key,
    const char* value) {
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
    SystemProperties* sp = new SystemProperties(nullptr, "./non-existent");
    ASSERT(sp->statisticsSampleInterval() == std::chrono::seconds(1),
           "expected 1");
    ASSERT(sp->statisticsEnabled() == true, "expected true");
    LOG(sp->statisticsArchiveFile());
    auto&& safname = sp->statisticsArchiveFile();
    ASSERT(safname == "statArchive.gfs",
           "Expected safname == \"statArchive.gfs\"");
    auto&& ll = Log::levelToChars(sp->logLevel());
    ASSERT_STREQ("config", ll);
    delete sp;
  }
END_TEST(DEFAULT)

BEGIN_TEST(NEW_CONFIG)
  {
    // When the tests are run from the build script the environment variable
    // TESTSRC is set.
    std::string testsrc(ACE_OS::getenv("TESTSRC"));
    std::string filePath = testsrc + "/resources/system.properties";

    // Make sure product can at least log to stdout.
    Log::init(Config, nullptr, 0);

    SystemProperties* sp = new SystemProperties(nullptr, filePath.c_str());

    ASSERT(sp->statisticsSampleInterval() == std::chrono::seconds(700),
           "expected 700");

    ASSERT(sp->statisticsEnabled() == false, "expected false");

    ASSERT(sp->threadPoolSize() == 96, "max-fe-thread should be 96");

    auto&& safname = sp->statisticsArchiveFile();
    ASSERT1(safname == "stats.gfs");

    auto&& logfname = sp->logFilename();
    ASSERT1(logfname == "gfcpp.log");

    // Log::LogLevel ll = sp->logLevel();
    // ASSERT( ll == Log::Debug, "expected Log::Debug" );

    auto&& name = sp->name();
    ASSERT1(name == "system");

    auto&& cxml = sp->cacheXMLFile();
    ASSERT1(cxml == "cache.xml");

    ASSERT(sp->pingInterval() == std::chrono::seconds(123),
           "expected 123 pingInterval");
    ASSERT(sp->redundancyMonitorInterval() == std::chrono::seconds(456),
           "expected 456s redundancyMonitorInterval");

    ASSERT(sp->heapLRULimit() == 100, "expected 100");
    ASSERT(sp->heapLRUDelta() == 10, "expected 10");

    ASSERT(sp->notifyAckInterval() == std::chrono::milliseconds(1234),
           "expected 1234 notifyAckInterval");
    ASSERT(sp->notifyDupCheckLife() == std::chrono::milliseconds(4321),
           "expected 4321 notifyDupCheckLife");

    ASSERT(sp->logFileSizeLimit() == 1024000000, "expected 1024000000");

    ASSERT(sp->statsFileSizeLimit() == 1024000000, "expected 1024000000");

    auto&& durableId = sp->durableClientId();
    ASSERT1(durableId == "testDurableId");

    ASSERT(sp->durableTimeout() == std::chrono::seconds(123),
           "expected 123 durableTimeOut");

    ASSERT(sp->connectTimeout() == std::chrono::milliseconds(345),
           "expected 345 for connect timeout");

    auto securityProperties = sp->getSecurityProperties();
    ASSERT(checkSecurityProperties(securityProperties, "security-username",
                                   "username") == true,
           "SecurityProperties Not Stored");
    ASSERT(checkSecurityProperties(securityProperties, "security-password",
                                   "password") == true,
           "SecurityProperties Not Stored");

    delete sp;
  }
END_TEST(NEW_CONFIG)
