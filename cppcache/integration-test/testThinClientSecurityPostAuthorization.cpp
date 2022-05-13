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
/**
 * @file testThinClientSecurityPostAuthorization.cpp
 *
 * @brief Currently this tests for getAll() with only some keys allowed
 *        so expecting some authorization failure exceptions in the result
 *
 *
 */

#include "fw_dunit.hpp"
#include "CacheHelper.hpp"
#include "ThinClientHelper.hpp"

#include <string>

#include "ThinClientSecurity.hpp"

using apache::geode::client::Exception;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::HashMapOfException;
using apache::geode::client::NotAuthorizedException;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

#define HANDLE_NO_NOT_AUTHORIZED_EXCEPTION                 \
  catch (const NotAuthorizedException &) {                 \
    LOG("NotAuthorizedException Caught");                  \
    FAIL("should not have caught NotAuthorizedException"); \
  }                                                        \
  catch (const Exception &other) {                         \
    LOG("Got apache::geode::client::Exception& other ");   \
    LOG(other.getStackTrace().c_str());                    \
    FAIL(other.what());                                    \
  }

#define HANDLE_NOT_AUTHORIZED_EXCEPTION    \
  catch (const NotAuthorizedException &) { \
    LOG("NotAuthorizedException Caught");  \
    LOG("Success");                        \
  }                                        \
  catch (const Exception &other) {         \
    LOG(other.getStackTrace().c_str());    \
    FAIL(other.what());                    \
  }

#define ADMIN_CLIENT s1p1
#define WRITER_CLIENT s1p2
#define READER_CLIENT s2p1
#define READER2_CLIENT s2p2

const char *regionNamesAuth[] = {"DistRegionAck"};

void initClientAuth(char userType, int clientNum = 1) {
  auto config = Properties::create();
  config->insert("security-client-auth-library", "securityImpl");
  config->insert("security-client-auth-factory",
                 "createUserPasswordAuthInitInstance");
  switch (userType) {
    case 'W': {
      config->insert("security-username", "geode9");
      config->insert("security-password", "geode9");
      break;
    }
    case 'R': {
      auto clientStr = std::string("geode") + std::to_string(clientNum);
      config->insert("security-username", clientStr);
      config->insert("security-password", clientStr);
      break;
    }
    case 'A': {
      config->insert("security-username", "geode1");
      config->insert("security-password", "geode1");
      break;
    }
    default: {
      break;
    }
  }
  initClient(true, config);
}

const char *getServerSecurityParams() {
  static std::string serverSecurityParams;

  if (serverSecurityParams.size() == 0) {
    serverSecurityParams =
        "security-client-authenticator="
        "javaobject.LdapUserAuthenticator.create "
        "security-client-accessor="
        "org.apache.geode.internal.security.FilterPreAuthorization.create "
        "security-client-accessor-pp="
        "org.apache.geode.internal.security.FilterPostAuthorization.create "
        "log-level=fine security-log-level=finest";

    char *ldapSrv = std::getenv("LDAP_SERVER");
    serverSecurityParams += std::string(" security-ldap-server=") +
                            (ldapSrv != nullptr ? ldapSrv : "ldap");

    char *ldapRoot = std::getenv("LDAP_BASEDN");
    serverSecurityParams +=
        std::string(" security-ldap-basedn=") +
        (ldapRoot != nullptr ? ldapRoot
                             : "ou=ldapTesting,dc=ldap,dc=apache,dc=org");

    char *ldapSSL = std::getenv("LDAP_USESSL");
    serverSecurityParams += std::string(" security-ldap-usessl=") +
                            (ldapSSL != nullptr ? ldapSSL : "false");
  }
  return serverSecurityParams.c_str();
}

void getKeysVector(std::vector<std::shared_ptr<CacheableKey>> &keysVec,
                   int numKeys) {
  for (int index = 0; index < numKeys; ++index) {
    keysVec.push_back(CacheableString::create(keys[index]));
  }
}

void checkValuesMap(HashMapOfCacheable &values, int clientNum, int numKeys) {
  size_t expectedNum = 0;
  std::shared_ptr<CacheableKey> key;
  std::shared_ptr<CacheableString> val;
  std::shared_ptr<CacheableString> expectedVal;
  for (int index = clientNum - 1; index < numKeys; index += clientNum) {
    ++expectedNum;
    key = CacheableString::create(keys[index]);
    const auto &iter = values.find(key);
    ASSERT(iter != values.end(), "key not found in values map");
    val = std::dynamic_pointer_cast<CacheableString>(iter->second);
    expectedVal = CacheableString::create(nvals[index]);
    ASSERT(*val == *expectedVal, "unexpected value in values map");
  }
  std::cout << "Expected number of values: " << expectedNum
            << "; got values: " << values.size() << "\n";
  ASSERT(values.size() == expectedNum, "unexpected number of values");
}

void checkExceptionsMap(HashMapOfException &exceptions, int clientNum,
                        int numKeys) {
  size_t expectedNum = 0;
  std::shared_ptr<CacheableKey> key;
  for (int index = 0; index < numKeys; ++index) {
    if ((index + 1) % clientNum != 0) {
      ++expectedNum;
      key = CacheableString::create(keys[index]);
      const auto &iter = exceptions.find(key);
      ASSERT(iter != exceptions.end(), "key not found in exceptions map");
      ASSERT(std::dynamic_pointer_cast<std::shared_ptr<NotAuthorizedException>>(
                 iter->second),
             "unexpected exception type in exception map");
      std::cout << "Got expected NotAuthorizedException: "
                << iter->second->what() << "\n";
    }
  }
  std::cout << "Expected number of exceptions: " << expectedNum
            << "; got exceptions: " << exceptions.size() << "\n";
  ASSERT(exceptions.size() == expectedNum, "unexpected number of exceptions");
}

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartLocator)
  {
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                              locHostPort, getServerSecurityParams());
      LOG("Server1 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml",
                              locHostPort, getServerSecurityParams());
      LOG("Server2 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StepOne)
  {
    initClientAuth('A');
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      invalidateEntry(regionNamesAuth[0], keys[0]);
      verifyInvalid(regionNamesAuth[0], keys[0]);
      destroyEntry(regionNamesAuth[0], keys[0]);
      verifyDestroyed(regionNamesAuth[0], keys[0]);
      destroyRegion(regionNamesAuth[0]);

      auto key0 = CacheableKey::create(keys[0]);
      auto val0 = CacheableString::create(vals[0]);
      auto key2 = CacheableKey::create(keys[2]);
      auto val2 = CacheableString::create(nvals[2]);

      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      createEntry(regionNamesAuth[0], keys[2], nvals[2]);
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      if (regPtr0 != nullptr) {
        regPtr0->registerAllKeys();
        regPtr0->unregisterAllKeys();
      }

      std::vector<std::shared_ptr<CacheableKey>> keysVec;
      keysVec.push_back(key0);
      keysVec.push_back(key2);
      auto values = regPtr0->getAll(keysVec);
      ASSERT(values.size() == 2, "Expected 2 entries");
      auto res0 = std::dynamic_pointer_cast<CacheableString>(values[key0]);
      auto res2 = std::dynamic_pointer_cast<CacheableString>(values[key2]);
      ASSERT(*res0 == *val0, "Unexpected value for key");
      ASSERT(*res2 == *val2, "Unexpected value for key");
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(WRITER_CLIENT, StepTwo)
  {
    initClientAuth('W');
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      invalidateEntry(regionNamesAuth[0], keys[0]);
      verifyInvalid(regionNamesAuth[0], keys[0]);
      destroyEntry(regionNamesAuth[0], keys[0]);
      verifyDestroyed(regionNamesAuth[0], keys[0]);
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      verifyEntry(regionNamesAuth[0], keys[0], nvals[0]);
      destroyRegion(regionNamesAuth[0]);
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      auto keyPtr = CacheableKey::create(keys[0]);
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr0->get(keyPtr));
      FAIL("Should get NotAuthorizedException");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      createEntry(regionNamesAuth[0], keys[1], nvals[1]);
      createEntry(regionNamesAuth[0], keys[2], nvals[2]);
      createEntry(regionNamesAuth[0], keys[3], nvals[3]);
      createEntry(regionNamesAuth[0], keys[4], nvals[4]);
      createEntry(regionNamesAuth[0], keys[5], nvals[5]);
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER_CLIENT, StepThree)
  {
    initClientAuth('R', 2);
    createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);

    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    getKeysVector(keysVector, 6);
    auto rptr = getHelper()->getRegion(regionNamesAuth[0]);
    auto values = rptr->getAll(keysVector);

    checkValuesMap(values, 2, 6);

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER2_CLIENT, StepFour)
  {
    initClientAuth('R', 3);
    createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);

    std::vector<std::shared_ptr<CacheableKey>> keysVector;
    getKeysVector(keysVector, 6);
    auto rptr = getHelper()->getRegion(regionNamesAuth[0]);
    auto values = rptr->getAll(keysVector);

    checkValuesMap(values, 3, 6);

    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("Server1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("Server2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseCacheAdmin)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(WRITER_CLIENT, CloseCacheWriter)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER_CLIENT, CloseCacheReader)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER2_CLIENT, CloseCacheReader2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

void doThinClientSecurityPostAuthorization() {
  CALL_TASK(StartLocator);
  CALL_TASK(StartServer1);
  CALL_TASK(StartServer2);
  CALL_TASK(StepOne);
  CALL_TASK(StepTwo);
  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseServer2);
  CALL_TASK(CloseCacheAdmin);
  CALL_TASK(CloseCacheWriter);
  CALL_TASK(CloseCacheReader);
  CALL_TASK(CloseCacheReader2);
  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientSecurityPostAuthorization(); }
END_MAIN
