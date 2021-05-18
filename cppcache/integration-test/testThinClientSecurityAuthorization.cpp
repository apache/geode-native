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
#include "fw_dunit.hpp"
#include <geode/FunctionService.hpp>
#include <geode/CqAttributesFactory.hpp>

#define ROOT_NAME "testThinClientSecurityAuthentication"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"
#include "ThinClientHelper.hpp"
#include <ace/Process.h>

#include "ThinClientSecurity.hpp"

using apache::geode::client::CqAttributesFactory;
using apache::geode::client::FunctionService;
using apache::geode::client::HashMapOfCacheable;
using apache::geode::client::QueryService;
using apache::geode::client::testframework::security::CredentialGenerator;
using apache::geode::client::testframework::security::OP_CONTAINS_KEY;
using apache::geode::client::testframework::security::OP_CREATE;
using apache::geode::client::testframework::security::OP_DESTROY;
using apache::geode::client::testframework::security::OP_EXECUTE_FUNCTION;
using apache::geode::client::testframework::security::OP_GET;
using apache::geode::client::testframework::security::OP_GETALL;
using apache::geode::client::testframework::security::OP_INVALIDATE;
using apache::geode::client::testframework::security::OP_KEY_SET;
using apache::geode::client::testframework::security::OP_PUTALL;
using apache::geode::client::testframework::security::OP_QUERY;
using apache::geode::client::testframework::security::OP_REGION_CLEAR;
using apache::geode::client::testframework::security::OP_REGISTER_CQ;
using apache::geode::client::testframework::security::OP_REGISTER_INTEREST;
using apache::geode::client::testframework::security::OP_UNREGISTER_INTEREST;
using apache::geode::client::testframework::security::OP_UPDATE;
using apache::geode::client::testframework::security::opCodeList;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
std::shared_ptr<CredentialGenerator> credentialGeneratorHandler;

std::string getXmlPath() {
  char xmlPath[1000] = {'\0'};
  const char *path = std::getenv("TESTSRC");
  ASSERT(path != nullptr,
         "Environment variable TESTSRC for test source directory is not set.");
  strncpy(xmlPath, path, strlen(path) - strlen("cppcache"));
  strncat(xmlPath, "xml/Security/", sizeof(xmlPath) - strlen(xmlPath) - 1);
  return std::string(xmlPath);
}

void initCredentialGenerator() {
  static int loopNum = 1;

  switch (loopNum) {
    case 1: {
      credentialGeneratorHandler = CredentialGenerator::create("DUMMY");
      break;
    }
    case 2: {
      credentialGeneratorHandler = CredentialGenerator::create("LDAP");
      break;
    }
    default:
    case 3: {
      credentialGeneratorHandler = CredentialGenerator::create("PKCS");
      break;
    }
  }

  if (credentialGeneratorHandler == nullptr) {
    FAIL("credentialGeneratorHandler is nullptr");
  }

  loopNum++;
  if (loopNum > 3) loopNum = 1;
}

opCodeList::value_type tmpRArr[] = {
    OP_GET,     OP_GETALL,      OP_REGISTER_INTEREST, OP_UNREGISTER_INTEREST,
    OP_KEY_SET, OP_CONTAINS_KEY};

opCodeList::value_type tmpWArr[] = {OP_CREATE,  OP_UPDATE,     OP_PUTALL,
                                    OP_DESTROY, OP_INVALIDATE, OP_REGION_CLEAR};

opCodeList::value_type tmpAArr[] = {OP_CREATE,       OP_UPDATE,
                                    OP_DESTROY,      OP_INVALIDATE,
                                    OP_REGION_CLEAR, OP_REGISTER_INTEREST,
                                    OP_GET,          OP_QUERY,
                                    OP_REGISTER_CQ,  OP_EXECUTE_FUNCTION};

#define HANDLE_NO_NOT_AUTHORIZED_EXCEPTION                        \
  catch (const apache::geode::client::NotAuthorizedException &) { \
    LOG("NotAuthorizedException Caught");                         \
    FAIL("should not have caught NotAuthorizedException");        \
  }                                                               \
  catch (const apache::geode::client::Exception &other) {         \
    LOG("Got apache::geode::client::Exception& other ");          \
    LOG(other.getStackTrace());                                   \
    FAIL(other.what());                                           \
  }

#define HANDLE_NOT_AUTHORIZED_EXCEPTION                           \
  catch (const apache::geode::client::NotAuthorizedException &) { \
    LOG("NotAuthorizedException Caught");                         \
    LOG("Success");                                               \
  }                                                               \
  catch (const apache::geode::client::Exception &other) {         \
    LOG(other.getStackTrace());                                   \
    FAIL(other.what());                                           \
  }

#define ADMIN_CLIENT s1p1
#define WRITER_CLIENT s1p2
#define READER_CLIENT s2p1

const char *regionNamesAuth[] = {"DistRegionAck"};

void initClientAuth(char UserType) {
  auto config = Properties::create();
  opCodeList wr(tmpWArr, tmpWArr + sizeof tmpWArr / sizeof *tmpWArr);
  opCodeList rt(tmpRArr, tmpRArr + sizeof tmpRArr / sizeof *tmpRArr);
  opCodeList ad(tmpAArr, tmpAArr + sizeof tmpAArr / sizeof *tmpAArr);
  credentialGeneratorHandler->getAuthInit(config);
  switch (UserType) {
    case 'W':
      credentialGeneratorHandler->getAllowedCredentialsForOps(wr, config,
                                                              nullptr);
      break;
    case 'R':
      credentialGeneratorHandler->getAllowedCredentialsForOps(rt, config,
                                                              nullptr);
      break;
    case 'A':
      credentialGeneratorHandler->getAllowedCredentialsForOps(ad, config,
                                                              nullptr);
      break;
    default:
      break;
  }

  auto alias = config->find("security-alias");
  auto uname = config->find("security-username");
  auto passwd = config->find("security-password");

  char msgAlias[100];
  char msgUname[100];
  char msgPasswd[100];

  sprintf(msgAlias, "PKCS alias is %s",
          alias == nullptr ? "null" : alias->value().c_str());
  sprintf(msgUname, "username is %s",
          uname == nullptr ? "null" : uname->value().c_str());
  sprintf(msgPasswd, "password is %s",
          passwd == nullptr ? "null" : passwd->value().c_str());

  LOG(msgAlias);
  LOG(msgUname);
  LOG(msgPasswd);

  try {
    initClient(true, config);
  } catch (...) {
    throw;
  }
}

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartServer1)
  {
    initCredentialGenerator();
    std::string cmdServerAuthenticator;

    if (isLocalServer) {
      cmdServerAuthenticator = credentialGeneratorHandler->getServerCmdParams(
          "authenticator:authorizer", getXmlPath());
      printf("string %s", cmdServerAuthenticator.c_str());
      CacheHelper::initServer(
          1, "cacheserver_notify_subscription.xml", locHostPort,
          const_cast<char *>(cmdServerAuthenticator.c_str()));
      LOG("Server1 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartServer2)
  {
    std::string cmdServerAuthenticator;

    if (isLocalServer) {
      cmdServerAuthenticator = credentialGeneratorHandler->getServerCmdParams(
          "authenticator:authorizer", getXmlPath());
      printf("string %s", cmdServerAuthenticator.c_str());
      CacheHelper::initServer(
          2, "cacheserver_notify_subscription2.xml", locHostPort,
          const_cast<char *>(cmdServerAuthenticator.c_str()));
      LOG("Server2 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartLocator)
  {
    if (isLocator) {
      CacheHelper::initLocator(1);
      LOG("Locator1 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StepOne)
  {
    initClientAuth('A');
    try {
      LOG("Tying Region creation");
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);

      auto regPtr = getHelper()->getRegion(regionNamesAuth[0]);

      LOG("Region created successfully");
      //---------------------for region clear tests-----
      regPtr->put(1, 1);
      regPtr->clear();

      auto getVal = regPtr->get(1);
      if (getVal == nullptr) {
        LOG("Get completed after region.clear successfully");
      } else {
        FAIL("Get did not complete successfully");
      }

      //---------------------------------------------------
      LOG("Tying Entry creation");
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      LOG("Entry created successfully");
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      LOG("Entry updated successfully");
      HashMapOfCacheable entrymap;
      entrymap.clear();
      for (int i = 0; i < 5; i++) {
        entrymap.emplace(CacheableKey::create(i), CacheableInt32::create(i));
      }

      regPtr->putAll(entrymap);
      LOG("PutAll completed successfully");
      for (int i = 0; i < 5; i++) {
        regPtr->invalidate(CacheableKey::create(i));
      }
      std::vector<std::shared_ptr<CacheableKey>> entrykeys;
      for (int i = 0; i < 5; i++) {
        entrykeys.push_back(CacheableKey::create(i));
      }
      const auto valuesMap = regPtr->getAll(entrykeys);
      if (valuesMap.size() > 0) {
        LOG("GetAll completed successfully");
      } else {
        FAIL("GetAll did not complete successfully");
      }

      LOG("GetServerKeys check started for ADMIN");
      auto keysvec = regPtr->serverKeys();
      LOG("GetServerKeys check passed for ADMIN");

      regPtr->query("1=1");
      LOG("Query completed successfully");
      auto pool =
          getHelper()->getCache()->getPoolManager().find(regionNamesAuth[0]);
      std::shared_ptr<QueryService> qs;
      if (pool != nullptr) {
        // Using region name as pool name
        qs = pool->getQueryService();
      } else {
        qs = getHelper()->cachePtr->getQueryService();
      }
      char queryString[100];
      sprintf(queryString, "select * from /%s", regionNamesAuth[0]);
      CqAttributesFactory cqFac;
      auto cqAttrs = cqFac.create();
      auto qry = qs->newCq("cq_security", queryString, cqAttrs);
      qs->executeCqs();
      qs->closeCqs();
      LOG("CQ completed successfully");
      if (pool != nullptr) {
        // TODO:
        FunctionService::onServer(pool).execute("securityTest")->getResult();
        LOG("Function execution completed successfully");
        FunctionService::onServers(pool).execute("securityTest")->getResult();
        LOG("Function execution completed successfully");
        FunctionService::onRegion(regPtr).execute("securityTest")->getResult();
        LOG("Function execution completed successfully");
        FunctionService::onRegion(regPtr).execute("FireNForget");
        LOG("Function execution with no result completed successfully");
      } else {
        LOG("Skipping function execution for non pool case");
      }
      invalidateEntry(regionNamesAuth[0], keys[0]);
      LOG("Entry invalidated successfully");
      verifyInvalid(regionNamesAuth[0], keys[0]);
      LOG("Entry invalidate-verified successfully");
      destroyEntry(regionNamesAuth[0], keys[0]);
      LOG("Entry destroyed successfully");
      verifyDestroyed(regionNamesAuth[0], keys[0]);
      LOG("Entry destroy-verified successfully");
      destroyRegion(regionNamesAuth[0]);
      LOG("Region destroy successfully");
      LOG("Tying Region creation");
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      LOG("Region created successfully");
      createEntry(regionNamesAuth[0], keys[2], vals[2]);
      LOG("Entry created successfully");
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      if (regPtr0 != nullptr) {
        LOG("Going to do registerAllKeys");
        regPtr0->registerAllKeys();
        LOG("Going to do unregisterAllKeys");
        regPtr0->unregisterAllKeys();
      }
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(WRITER_CLIENT, StepTwo)
  {
    initCredentialGenerator();
    initClientAuth('W');
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      LOG("Region created successfully");
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      LOG("Entry created successfully");
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      LOG("Entry updated successfully");
      HashMapOfCacheable entrymap;
      entrymap.clear();
      for (int i = 0; i < 5; i++) {
        entrymap.emplace(CacheableKey::create(i), CacheableInt32::create(i));
      }
      auto regPtr = getHelper()->getRegion(regionNamesAuth[0]);
      regPtr->putAll(entrymap);
      LOG("PutAll completed successfully");
      invalidateEntry(regionNamesAuth[0], keys[0]);
      LOG("Entry invalidated successfully");
      verifyInvalid(regionNamesAuth[0], keys[0]);
      LOG("Entry invalidate-verified successfully");
      destroyEntry(regionNamesAuth[0], keys[0]);
      LOG("Entry destroyed successfully");
      verifyDestroyed(regionNamesAuth[0], keys[0]);
      LOG("Entry destroy-verified successfully");
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      LOG("Entry created successfully");
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      LOG("Entry updated successfully");
      verifyEntry(regionNamesAuth[0], keys[0], nvals[0]);
      LOG("Entry updation-verified successfully");
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION
    try {
      auto regPtr = getHelper()->getRegion(regionNamesAuth[0]);
      LOG("containsKeyOnServer");
      regPtr->containsKeyOnServer(
          apache::geode::client::CacheableKey::create(keys[2]));
      FAIL("containsKeyOnServer should hav failed for writer");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      auto keyPtr = CacheableKey::create(keys[2]);
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr0->get(keyPtr));
      if (checkPtr != nullptr) {
        char buf[1024];
        sprintf(buf, "In net search, get returned %s for key %s",
                checkPtr->value().c_str(), keys[2]);
        LOG(buf);
        FAIL("Should not get the value");
      } else {
        LOG("checkPtr is nullptr");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION
    auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
    try {
      LOG("Going to do registerAllKeys");
      regPtr0->registerAllKeys();
      FAIL("Should not be able to do Register Interest");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      for (int i = 0; i < 5; i++) {
        regPtr0->invalidate(CacheableKey::create(i));
      }
      std::vector<std::shared_ptr<CacheableKey>> entrykeys;
      for (int i = 0; i < 5; i++) {
        entrykeys.push_back(CacheableKey::create(i));
      }
      const auto valuesMap = regPtr0->getAll(entrykeys);
      if (valuesMap.size() > 0) {
        FAIL("GetAll should not have completed successfully");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      regPtr0->query("1=1");
      FAIL("Query should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesAuth[0]);

    try {
      std::shared_ptr<QueryService> qs;
      if (pool != nullptr) {
        // Using region name as pool name
        qs = pool->getQueryService();
      } else {
        qs = getHelper()->cachePtr->getQueryService();
      }
      char queryString[100];
      sprintf(queryString, "select * from /%s", regionNamesAuth[0]);
      CqAttributesFactory cqFac;
      auto cqAttrs = cqFac.create();
      auto qry = qs->newCq("cq_security", queryString, cqAttrs);
      qs->executeCqs();
      FAIL("CQ should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("GetServerKeys check started for WRITER");
      auto keysvec = regPtr0->serverKeys();
      LOG("GetServerKeys check passed for WRITER");
      FAIL("GetServerKeys should not have completed successfully for WRITER");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    createEntry(regionNamesAuth[0], keys[2], vals[2]);
    LOG("Entry created successfully");

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER_CLIENT, StepThree)
  {
    initCredentialGenerator();
    initClientAuth('R');
    std::shared_ptr<Region> rptr;
    char buf[100];
    int value = 102;

    createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);

    rptr = getHelper()->getRegion(regionNamesAuth[0]);
    sprintf(buf, "%s: %d", rptr->getName().c_str(), value);
    auto key = CacheableKey::create(buf);
    sprintf(buf, "testUpdate::%s: value of %d", rptr->getName().c_str(), value);
    auto valuePtr = buf;
    try {
      LOG("Trying put Operation");
      rptr->put(key, valuePtr);
      LOG(" Put Operation Successful");
      FAIL("Should have got NotAuthorizedException during put");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("Trying createEntry");
      createEntry(regionNamesAuth[0], keys[2], vals[2]);
      FAIL("Should have got NotAuthorizedException during createEntry");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    ASSERT(!rptr->containsKey(keys[2]),
           "Key should not have been found in the region");
    try {
      LOG("containsKeyOnServer");
      rptr->containsKeyOnServer(
          apache::geode::client::CacheableKey::create(keys[2]));
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("Trying updateEntry");
      updateEntry(regionNamesAuth[0], keys[2], nvals[2], false, false);
      FAIL("Should have got NotAuthorizedException during updateEntry");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    ASSERT(!rptr->containsKey(keys[2]),
           "Key should not have been found in the region");

    try {
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      auto keyPtr = CacheableKey::create(keys[2]);
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr0->get(keyPtr));
      if (checkPtr != nullptr) {
        LOG("In net search, get returned " + checkPtr->value() + " for key " +
            keys[2]);
      } else {
        LOG("checkPtr is nullptr");
      }
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("Trying region clear..");
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      regPtr0->clear();
      FAIL("Should have got NotAuthorizedException for region.clear ops");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
    if (regPtr0 != nullptr) {
      try {
        LOG("Going to do registerAllKeys");
        regPtr0->registerAllKeys();
        LOG("Going to do unregisterAllKeys");
        regPtr0->unregisterAllKeys();
      }
      HANDLE_NO_NOT_AUTHORIZED_EXCEPTION
    }

    try {
      HashMapOfCacheable entrymap;
      entrymap.clear();
      for (int i = 0; i < 5; i++) {
        entrymap.emplace(CacheableKey::create(i), CacheableInt32::create(i));
      }
      regPtr0->putAll(entrymap);
      FAIL("PutAll should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("GetServerKeys check started for READER");
      auto keysvec = regPtr0->serverKeys();
      LOG("GetServerKeys check passed for READER");
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      std::vector<std::shared_ptr<CacheableKey>> entrykeys;
      for (int i = 0; i < 5; i++) {
        entrykeys.push_back(CacheableKey::create(i));
      }
      const auto valuesMap = regPtr0->getAll(entrykeys);
      if (valuesMap.size() > 0) {
        LOG("GetAll completed successfully");
      } else {
        FAIL("GetAll did not complete successfully");
      }
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      regPtr0->query("1=1");
      FAIL("Query should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    auto pool =
        getHelper()->getCache()->getPoolManager().find(regionNamesAuth[0]);

    try {
      std::shared_ptr<QueryService> qs;
      if (pool != nullptr) {
        // Using region name as pool name
        qs = pool->getQueryService();
      } else {
        qs = getHelper()->cachePtr->getQueryService();
      }
      char queryString[100];
      sprintf(queryString, "select * from /%s", regionNamesAuth[0]);
      CqAttributesFactory cqFac;
      auto cqAttrs = cqFac.create();
      auto qry = qs->newCq("cq_security", queryString, cqAttrs);
      qs->executeCqs();
      //    FAIL("CQ should not have completed successfully");
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    pool = getHelper()->getCache()->getPoolManager().find(regionNamesAuth[0]);

    try {
      if (pool != nullptr) {
        FunctionService::onServer(pool).execute("securityTest")->getResult();
        FAIL("Function execution should not have completed successfully");
      } else {
        LOG("Skipping function execution for non pool case");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      if (pool != nullptr) {
        FunctionService::onServer(pool).execute("securityTest")->getResult();
        FAIL("Function execution should not have completed successfully");
      } else {
        LOG("Skipping function execution for non pool case");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      if (pool != nullptr) {
        FunctionService::onServers(pool).execute("securityTest")->getResult();
        FAIL("Function execution should not have completed successfully");
      } else {
        LOG("Skipping function execution for non pool case");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      if (pool != nullptr) {
        regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
        FunctionService::onRegion(regPtr0).execute("securityTest")->getResult();
        FAIL("Function execution should not have completed successfully");
      } else {
        LOG("Skipping function execution for non pool case");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseServer1)
  {
    SLEEP(9000);
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
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

void doThinClientSecurityAuthorization() {
  CALL_TASK(StartLocator);
  CALL_TASK(StartServer1);
  CALL_TASK(StepOne);
  CALL_TASK(StepTwo);
  CALL_TASK(StartServer2);
  CALL_TASK(CloseServer1);
  CALL_TASK(StepThree);
  CALL_TASK(CloseCacheReader);
  CALL_TASK(CloseCacheWriter);
  CALL_TASK(CloseCacheAdmin);
  CALL_TASK(CloseServer2);
  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientSecurityAuthorization(); }
END_MAIN
