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
#include <geode/Execution.hpp>
#include <geode/UserFunctionExecutionException.hpp>
#include <geode/RegionAttributesFactory.hpp>
#include <geode/CqAttributesFactory.hpp>

#define ROOT_NAME "testThinClientSecurityAuthenticationMU"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"
#include "ThinClientHelper.hpp"
#include "ace/Process.h"

#include "ThinClientSecurity.hpp"

using namespace apache::geode::client::testframework::security;
using namespace apache::geode::client;

const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
std::shared_ptr<CredentialGenerator> credentialGeneratorHandler;
char* exFuncNameSendException = (char*)"executeFunction_SendException";

std::string getXmlPath() {
  char xmlPath[1000] = {'\0'};
  const char* path = ACE_OS::getenv("TESTSRC");
  ASSERT(path != nullptr,
         "Environment variable TESTSRC for test source directory is not set.");
  strncpy(xmlPath, path, strlen(path) - strlen("cppcache"));
  strcat(xmlPath, "xml/Security/");
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
  if (loopNum > 2) loopNum = 1;
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

#define HANDLE_NO_NOT_AUTHORIZED_EXCEPTION                       \
  catch (const apache::geode::client::NotAuthorizedException&) { \
    LOG("NotAuthorizedException Caught");                        \
    FAIL("should not have caught NotAuthorizedException");       \
  }                                                              \
  catch (const apache::geode::client::Exception& other) {        \
    LOG("Got apache::geode::client::Exception& other ");         \
    LOG(other.getStackTrace().c_str());                          \
    FAIL(other.what());                                          \
  }

#define HANDLE_NOT_AUTHORIZED_EXCEPTION                          \
  catch (const apache::geode::client::NotAuthorizedException&) { \
    LOG("NotAuthorizedException Caught");                        \
    LOG("Success");                                              \
  }                                                              \
  catch (const apache::geode::client::Exception& other) {        \
    LOG(other.getStackTrace().c_str());                          \
    FAIL(other.what());                                          \
  }

#define ADMIN_CLIENT s1p1
#define WRITER_CLIENT s1p2
#define READER_CLIENT s2p1
//#define USER_CLIENT s2p2

#define TYPE_ADMIN_CLIENT 'A'
#define TYPE_WRITER_CLIENT 'W'
#define TYPE_READER_CLIENT 'R'
#define TYPE_USER_CLIENT 'U'

const char* regionNamesAuth[] = {"DistRegionAck"};
std::shared_ptr<Properties> userCreds;
void initClientAuth(char UserType) {
  userCreds = Properties::create();
  auto config = Properties::create();
  opCodeList wr(tmpWArr, tmpWArr + sizeof tmpWArr / sizeof *tmpWArr);
  opCodeList rt(tmpRArr, tmpRArr + sizeof tmpRArr / sizeof *tmpRArr);
  opCodeList ad(tmpAArr, tmpAArr + sizeof tmpAArr / sizeof *tmpAArr);
  credentialGeneratorHandler->getAuthInit(config);
  switch (UserType) {
    case 'W':
      credentialGeneratorHandler->getAllowedCredentialsForOps(wr, userCreds,
                                                              nullptr);
      break;
    case 'R':
      credentialGeneratorHandler->getAllowedCredentialsForOps(rt, userCreds,
                                                              nullptr);
      break;
    case 'A':
      credentialGeneratorHandler->getAllowedCredentialsForOps(ad, userCreds,
                                                              nullptr);
    default:
      break;
  }

  auto alias = userCreds->find("security-alias");
  auto uname = userCreds->find("security-username");
  auto passwd = userCreds->find("security-password");

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
          const_cast<char*>(cmdServerAuthenticator.c_str()));
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
          const_cast<char*>(cmdServerAuthenticator.c_str()));
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
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true, nullptr, false,
                              -1, true, 0);
      LOG("Region created successfully");
      LOG("Tying Entry creation");
      std::shared_ptr<RegionService> virtualCache;
      std::shared_ptr<Region> regionPtr;
      auto pool = getPool(regionNamesAuth[0]);
      LOG(" 6");
      if (pool != nullptr) {
        LOG(" 7");
        virtualCache = getVirtualCache(userCreds, pool);
        LOG(" 8");
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
        LOG("Operation allowed, something is wrong.");
      } else {
        LOG("Pool is nullptr");
      }

      //---------------------for region clear tests-----
      regionPtr->put(1, 1);
      regionPtr->clear();

     auto getVal = regionPtr->get(1);
      if (getVal == nullptr) {
        LOG("Get completed after region.clear successfully");
      } else {
        FAIL("Get did not complete successfully");
      }

      //---------------------------------------------------

      // createEntry( regionNamesAuth[0], keys[0], vals[0] );
      regionPtr->create(keys[0], vals[0]);
      LOG("Entry created successfully");
      // updateEntry( regionNamesAuth[0], keys[0], nvals[0] );
      regionPtr->put(keys[0], nvals[0]);
      LOG("Entry updated successfully");
      HashMapOfCacheable entrymap;
      entrymap.clear();
      for (int i = 0; i < 5; i++) {
        entrymap.emplace(CacheableKey::create(i), CacheableInt32::create(i));
      }
      //auto regPtr = getHelper()->getRegion(regionNamesAuth[0]);
      regionPtr->putAll(entrymap);
      LOG("PutAll completed successfully");
      /*for (int i=0; i<5; i++) {
        regPtr->invalidate(CacheableKey::create(i));
      }*/

      LOG("GetServerKeys check started for ADMIN");
      auto keysvec = regionPtr->serverKeys();
      LOG("GetServerKeys check passed for ADMIN");

      std::vector<std::shared_ptr<CacheableKey>>  entrykeys;
      for (int i = 0; i < 5; i++) {
        entrykeys.push_back(CacheableKey::create(i));
      }

      const auto valuesMap = regionPtr->getAll(entrykeys);
      if (valuesMap.size() > 0) {
        LOG("GetAll completed successfully");
      } else {
        FAIL("GetAll did not complete successfully");
      }
      regionPtr->query("1=1");
      LOG("Query completed successfully");

      std::shared_ptr<QueryService> qs;
      // Using region name as pool name
      try {
        qs = pool->getQueryService();
        FAIL("Pool should not return queryservice in multiusermode");
      } catch (const apache::geode::client::UnsupportedOperationException&) {
        LOG("UnsupportedOperationException Caught for pool.getQuerySerice in "
            "multiusermode");
        LOG("Success");
      } catch (const apache::geode::client::Exception& other) {
        LOG(other.getStackTrace().c_str());
        FAIL(other.what());
      }

      qs = virtualCache->getQueryService();

      char queryString[100];
      sprintf(queryString, "select * from /%s", regionNamesAuth[0]);
      auto qry = qs->newQuery(queryString);

      std::shared_ptr<SelectResults> results;
      printf(" before query executing\n");
      results = qry->execute(std::chrono::seconds(850));
      LOG("Query completed successfully");

      sprintf(queryString, "select * from /%s", regionNamesAuth[0]);
      CqAttributesFactory cqFac;
      auto cqAttrs = cqFac.create();
      auto cqQry = qs->newCq("cq_security", queryString, cqAttrs);
      cqQry->execute();
      cqQry->close();
      LOG("CQ completed successfully");

      if (pool != nullptr) {
        // TODO:
        // FunctionService::onServer(pool)->execute("securityTest",
        // true)->getResult();
        // auto funcServ = virtualCache->getFunctionService();
        // funcServ->onServer()->execute("securityTest", true)->getResult();
        FunctionService::onServer(virtualCache)
            ->execute("securityTest")
            ->getResult();
        LOG("onServer executed successfully.");
        // funcServ->onServers()->execute("securityTest", true)->getResult();
        FunctionService::onServers(virtualCache)
            ->execute("securityTest")
            ->getResult();
        LOG("onServerS executed successfully.");
        FunctionService::onRegion(regionPtr)
            ->execute("securityTest")
            ->getResult();
        LOG("FunctionService::onRegion executed successfully.");
        FunctionService::onRegion(regionPtr)->execute("FireNForget");
        LOG("Function execution with no result completed successfully");

        //-----------------------Test with
        // sendException-------------------------------//
        LOG("Function execution with sendException");
        char buf[128];
        for (int i = 1; i <= 200; i++) {
          auto value = CacheableInt32::create(i);

          sprintf(buf, "execKey-%d", i);
          auto key = CacheableKey::create(buf);
          regionPtr->put(key, value);
        }
        LOG("Put for execKey's on region complete.");

        LOG("Adding filter");
        auto arrList = CacheableArrayList::create();
        for (int i = 100; i < 120; i++) {
          sprintf(buf, "execKey-%d", i);
          auto key = CacheableKey::create(buf);
          arrList->push_back(key);
        }

       auto filter = CacheableVector::create();
        for (int i = 100; i < 120; i++) {
          sprintf(buf, "execKey-%d", i);
          auto key = CacheableKey::create(buf);
          filter->push_back(key);
        }
        LOG("Adding filter done.");

       auto args = CacheableBoolean::create(1);
        // UNUSED bool getResult = true;

       auto funcExec = FunctionService::onRegion(regionPtr);
        ASSERT(funcExec != nullptr, "onRegion Returned nullptr");

        auto collector = funcExec->withArgs(args).withFilter(filter).execute(
            exFuncNameSendException, std::chrono::seconds(15));
        ASSERT(collector != nullptr, "onRegion collector nullptr");

        auto result = collector->getResult();

        if (result == nullptr) {
          ASSERT(false, "echo String : result is nullptr");
        } else {
          try {
            for (int i = 0; i < result->size(); i++) {
              auto uFEPtr =
                  std::dynamic_pointer_cast<UserFunctionExecutionException>(
                      result->operator[](i));
              ASSERT(uFEPtr != nullptr, "uFEPtr exception is nullptr");
              LOGINFO("Done casting to uFEPtr");
              LOGINFO("Read expected uFEPtr exception %s ",
                      uFEPtr->getMessage().c_str());
            }
          } catch (ClassCastException& ex) {
            std::string logmsg = "";
            logmsg += ex.getName();
            logmsg += ": ";
            logmsg += ex.what();
            LOG(logmsg.c_str());
            LOG(ex.getStackTrace().c_str());
            FAIL(
                "exFuncNameSendException casting to string for bool arguement "
                "exception.");
          } catch (...) {
            FAIL(
                "exFuncNameSendException casting to string for bool arguement "
                "Unknown exception.");
          }
        }

        LOG("exFuncNameSendException done for bool arguement.");

        collector = funcExec->withArgs(arrList).withFilter(filter).execute(
            exFuncNameSendException, std::chrono::seconds(15));
        ASSERT(collector != nullptr, "onRegion collector for arrList nullptr");

        result = collector->getResult();
        ASSERT(result->size() == arrList->size() + 1,
               "region get: resultList count is not as arrayList count + "
               "exception");

        for (int i = 0; i < result->size(); i++) {
          try {
            auto intValue = std::dynamic_pointer_cast<CacheableInt32>(
                result->operator[](i));
            ASSERT(intValue != nullptr, "int value is nullptr");
            LOGINFO("intValue is %d ", intValue->value());
          } catch (ClassCastException& ex) {
            LOG("exFuncNameSendException casting to int for arrayList "
                "arguement "
                "exception.");
            std::string logmsg = "";
            logmsg += ex.getName();
            logmsg += ": ";
            logmsg += ex.what();
            LOG(logmsg.c_str());
            LOG(ex.getStackTrace().c_str());
            auto uFEPtr =
                std::dynamic_pointer_cast<UserFunctionExecutionException>(
                    result->operator[](i));
            ASSERT(uFEPtr != nullptr, "uFEPtr exception is nullptr");
            LOGINFO("Done casting to uFEPtr");
            LOGINFO("Read expected uFEPtr exception %s ",
                    uFEPtr->getMessage().c_str());
          } catch (...) {
            FAIL(
                "exFuncNameSendException casting to string for bool arguement "
                "Unknown exception.");
          }
        }

        LOG("exFuncNameSendException done for arrayList arguement.");

        LOG("Function execution with sendException successfull");
        //----------------------------------------------------------------------------------------------//

        LOG("Function execution completed successfully");
      } else {
        LOG("Skipping function execution for non pool case");
      }
      // invalidateEntry( regionNamesAuth[0], keys[0] );
      LOG("Entry invalidated successfully");
      // verifyInvalid( regionNamesAuth[0], keys[0] );
      LOG("Entry invalidate-verified successfully");
      // destroyEntry( regionNamesAuth[0], keys[0] );
      regionPtr->destroy(keys[0]);
      LOG("Entry destroyed successfully");
      // verifyDestroyed( regionNamesAuth[0], keys[0] );
      LOG("Entry destroy-verified successfully");
      destroyRegion(regionNamesAuth[0]);
      LOG("Region destroy successfully");
      LOG("Tying Region creation");
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, false, nullptr,
                              false, -1, true, 0);
      char buf[100] = {'\0'};
      static int indexForPool = 0;
      sprintf(buf, "%s_%d", regionNamesAuth[0], indexForPool++);
      pool = getPool(buf);
      LOG(" 6");
      if (pool != nullptr) {
        LOG(" 7");
        virtualCache = getVirtualCache(userCreds, pool);
        LOG(" 8");
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
        LOG("Operation allowed, something is wrong.");
      } else {
        LOG("Pool is nullptr");
      }

      LOG("Region created successfully");
      // createEntry( regionNamesAuth[0], keys[2], vals[2] );
      regionPtr->create(keys[2], vals[2]);
      LOG("Entry created successfully");
      virtualCache->close();
      LOG("Cache close successfully");
      // auto regPtr0 = getHelper()->getRegion( regionNamesAuth[0] );
      /*if (regPtr != nullptr ) {
        LOG("Going to do registerAllKeys");
       // regPtr->registerAllKeys();
        LOG("Going to do unregisterAllKeys");
       // regPtr->unregisterAllKeys();
      }*/
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("Trying operation using real region in multiusersecure mode");
      auto regPtr = getHelper()->getRegion(regionNamesAuth[0]);
      regPtr->put("key", "val");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(WRITER_CLIENT, StepTwo)
  {
    initCredentialGenerator();
    initClientAuth('W');
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true, nullptr, false,
                              -1, true, 0);
      LOG("Region created successfully");
      std::shared_ptr<RegionService> virtualCache;
      std::shared_ptr<Region> regionPtr;
      auto pool = getPool(regionNamesAuth[0]);
      LOG(" 6");
      if (pool != nullptr) {
        LOG(" 7");
        virtualCache = getVirtualCache(userCreds, pool);
        LOG(" 8");
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
        LOG("Operation allowed, something is wrong.");
      } else {
        LOG("Pool is nullptr");
      }

      // createEntry( regionNamesAuth[0], keys[0], vals[0] );
      regionPtr->create(keys[0], vals[0]);
      LOG("Entry created successfully");
      // updateEntry( regionNamesAuth[0], keys[0], nvals[0] );
      regionPtr->put(keys[0], nvals[0]);
      LOG("Entry updated successfully");
      HashMapOfCacheable entrymap;
      entrymap.clear();
      for (int i = 0; i < 5; i++) {
        entrymap.emplace(CacheableKey::create(i), CacheableInt32::create(i));
      }
      // auto regPtr = getHelper()->getRegion(regionNamesAuth[0]);
      regionPtr->putAll(entrymap);
      LOG("PutAll completed successfully");
      // invalidateEntry( regionNamesAuth[0], keys[0] );
      LOG("Entry invalidated successfully");
      // verifyInvalid( regionNamesAuth[0], keys[0] );
      LOG("Entry invalidate-verified successfully");
      // destroyEntry( regionNamesAuth[0], keys[0] );
      regionPtr->destroy(keys[0]);
      LOG("Entry destroyed successfully");
      // verifyDestroyed( regionNamesAuth[0], keys[0] );
      LOG("Entry destroy-verified successfully");
      // createEntry( regionNamesAuth[0], keys[0], vals[0] );
      regionPtr->create(keys[0], vals[0]);
      LOG("Entry created successfully");
      // updateEntry( regionNamesAuth[0], keys[0], nvals[0] );
      regionPtr->put(keys[0], nvals[0]);
      LOG("Entry updated successfully");
      // verifyEntry( regionNamesAuth[0], keys[0], nvals[0] );
      LOG("Entry updation-verified successfully");
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      std::shared_ptr<Region> regionPtr;
      auto pool = getPool(regionNamesAuth[0]);
      if (pool != nullptr) {
        virtualCache = getVirtualCache(userCreds, pool);
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
      } else {
        LOG("Pool is nullptr");
      }
      LOG("GetServerKeys check started for WRITER");
      auto keysvec = regionPtr->serverKeys();
      LOG("GetServerKeys check passed for WRITER");
      FAIL("GetServerKeys should not have completed successfully for WRITER");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      // auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      std::shared_ptr<RegionService> virtualCache;
      std::shared_ptr<Region> regionPtr;
      auto pool = getPool(regionNamesAuth[0]);
      LOG(" 6");
      if (pool != nullptr) {
        LOG(" 7");
        virtualCache = getVirtualCache(userCreds, pool);
        LOG(" 8");
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
        LOG("Operation allowed, something is wrong.");
      } else {
        LOG("Pool is nullptr");
      }
      auto keyPtr = CacheableKey::create(keys[2]);
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regionPtr->get(keyPtr));
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
    // auto regPtr0 = getHelper()->getRegion( regionNamesAuth[0] );

    try {
      LOG("Going to do registerAllKeys");
      // regionPtr->registerAllKeys();
      // FAIL("Should not be able to do Register Interest");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      std::shared_ptr<Region> regionPtr;
      auto pool = getPool(regionNamesAuth[0]);
      LOG(" 6");
      if (pool != nullptr) {
        LOG(" 7");
        virtualCache = getVirtualCache(userCreds, pool);
        LOG(" 8");
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
        LOG("Operation allowed, something is wrong.");
      } else {
        LOG("Pool is nullptr");
      }

      /* for (int i=0; i<5; i++) {
         regPtr0->invalidate(CacheableKey::create(i));
       }*/
      std::vector<std::shared_ptr<CacheableKey>> entrykeys;
      for (int i = 0; i < 5; i++) {
        entrykeys.push_back(CacheableKey::create(i));
      }

      const auto valuesMap = regionPtr->getAll(entrykeys);
      if (valuesMap.size() > 0) {
        FAIL("GetAll should not have completed successfully");
      }
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      std::shared_ptr<Region> regionPtr;
      auto pool = getPool(regionNamesAuth[0]);
      LOG(" 6");
      if (pool != nullptr) {
        LOG(" 7");
        virtualCache = getVirtualCache(userCreds, pool);
        LOG(" 8");
        regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
        LOG("Operation allowed, something is wrong.");
      } else {
        LOG("Pool is nullptr");
      }
      regionPtr->query("1=1");
      FAIL("Query should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      auto pool = getPool(regionNamesAuth[0]);
      virtualCache = getVirtualCache(userCreds, pool);
      auto qs = virtualCache->getQueryService();

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
      std::shared_ptr<RegionService> virtualCache;
      auto pool = getPool(regionNamesAuth[0]);
      virtualCache = getVirtualCache(userCreds, pool);
      // FunctionService::onServer(pool)->execute("securityTest",
      // true)->getResult();
      // FAIL("Function execution should not have completed successfully");
      // auto funcServ = virtualCache->getFunctionService();
      // funcServ->onServer()->execute("securityTest", true)->getResult();
      FunctionService::onServer(virtualCache)
          ->execute("securityTest")
          ->getResult();
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      auto pool = getPool(regionNamesAuth[0]);
      virtualCache = getVirtualCache(userCreds, pool);
      // FunctionService::onServer(pool)->execute("securityTest",
      // true)->getResult();
      // FAIL("Function execution should not have completed successfully");
      // auto funcServ = virtualCache->getFunctionService();
      // funcServ->onServers()->execute("securityTest", true)->getResult();
      FunctionService::onServers(virtualCache)
          ->execute("securityTest")
          ->getResult();
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      auto pool = getPool(regionNamesAuth[0]);
      virtualCache = getVirtualCache(userCreds, pool);
      std::shared_ptr<Region> regionPtr;
      regionPtr = virtualCache->getRegion(regionNamesAuth[0]);

      //-----------------------Test with
      // sendException-------------------------------//
      LOG("Function execution with sendException with expected Authorization "
          "exception");
      char buf[128];
      for (int i = 1; i <= 200; i++) {
        auto value = CacheableInt32::create(i);

        sprintf(buf, "execKey-%d", i);
        auto key = CacheableKey::create(buf);
        regionPtr->put(key, value);
      }
      LOG("Put for execKey's on region complete.");

      LOG("Adding filter");
      auto arrList = CacheableArrayList::create();
      for (int i = 100; i < 120; i++) {
        sprintf(buf, "execKey-%d", i);
        auto key = CacheableKey::create(buf);
        arrList->push_back(key);
      }

      auto filter = CacheableVector::create();
      for (int i = 100; i < 120; i++) {
        sprintf(buf, "execKey-%d", i);
        auto key = CacheableKey::create(buf);
        filter->push_back(key);
      }
      LOG("Adding filter done.");

      auto args = CacheableBoolean::create(1);
      // UNUSED bool getResult = true;

      LOG("OnServers with sendException");

      auto funcExec = FunctionService::onServers(virtualCache);

      auto collector = funcExec->withArgs(args).execute(
          exFuncNameSendException, std::chrono::seconds(15));

      //----------------------------------------------------------------------------------------------//
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
      auto pool = getPool(regionNamesAuth[0]);
      virtualCache = getVirtualCache(userCreds, pool);
      std::shared_ptr<Region> regionPtr;
      regionPtr = virtualCache->getRegion(regionNamesAuth[0]);

      // FunctionService::onServer(pool)->execute("securityTest",
      // true)->getResult();
      // FAIL("Function execution should not have completed successfully");
      FunctionService::onRegion(regionPtr)
          ->execute("securityTest")
          ->getResult();
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      std::shared_ptr<RegionService> virtualCache;
     auto pool = getPool(regionNamesAuth[0]);
     virtualCache = getVirtualCache(userCreds, pool);
     std::shared_ptr<Region> regionPtr;
     regionPtr = virtualCache->getRegion(regionNamesAuth[0]);

     //-----------------------Test with
     // sendException-------------------------------//
     LOG("Function execution with sendException with expected Authorization "
         "exception with onRegion");
     char buf[128];
     for (int i = 1; i <= 200; i++) {
       auto value = CacheableInt32::create(i);

       sprintf(buf, "execKey-%d", i);
       auto key = CacheableKey::create(buf);
       regionPtr->put(key, value);
     }
     LOG("Put for execKey's on region complete.");

     LOG("Adding filter");
     auto arrList = CacheableArrayList::create();
     for (int i = 100; i < 120; i++) {
       sprintf(buf, "execKey-%d", i);
       auto key = CacheableKey::create(buf);
       arrList->push_back(key);
      }

     auto filter = CacheableVector::create();
      for (int i = 100; i < 120; i++) {
        sprintf(buf, "execKey-%d", i);
        auto key = CacheableKey::create(buf);
        filter->push_back(key);
      }
      LOG("Adding filter done.");

     auto args = CacheableBoolean::create(1);
      // UNUSED bool getResult = true;

      LOG("OnServers with sendException");

     auto funcExec = FunctionService::onRegion(regionPtr);

     auto collector = funcExec->withArgs(args).withFilter(filter).execute(
         exFuncNameSendException, std::chrono::seconds(15));

     //----------------------------------------------------------------------------------------------//

     // FunctionService::onServer(pool)->execute("securityTest",
     // true)->getResult();
     // FAIL("Function execution should not have completed successfully");
     // FunctionService::onRegion(regionPtr)->execute("securityTest",
     // true)->getResult();
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    std::shared_ptr<RegionService> virtualCache;
    std::shared_ptr<Region> regionPtr;
    auto pool = getPool(regionNamesAuth[0]);
    LOG(" 6");
    if (pool != nullptr) {
      LOG(" 7");
      virtualCache = getVirtualCache(userCreds, pool);
      LOG(" 8");
      regionPtr = virtualCache->getRegion(regionNamesAuth[0]);
      LOG("Operation allowed, something is wrong.");
    } else {
      LOG("Pool is nullptr");
    }

    // createEntry( regionNamesAuth[0], keys[2], vals[2] );
    regionPtr->create(keys[2], vals[2]);
    LOG("Entry created successfully");

    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER_CLIENT, StepThree)
  {
    initCredentialGenerator();
    initClientAuth('R');
    char buf[100];
    int i = 102;

    createRegionForSecurity(regionNamesAuth[0], USE_ACK, false, nullptr, false,
                            -1, true, 0);
    std::shared_ptr<RegionService> virtualCache;
    std::shared_ptr<Region> rptr;
    auto pool = getPool(regionNamesAuth[0]);
    LOG(" 6");
    if (pool != nullptr) {
      LOG(" 7");
      virtualCache = getVirtualCache(userCreds, pool);
      LOG(" 8");
      rptr = virtualCache->getRegion(regionNamesAuth[0]);
      LOG("Operation allowed, something is wrong.");
    } else {
      LOG("Pool is nullptr");
    }

    // rptr = getHelper()->getRegion(regionNamesAuth[0]);
    sprintf(buf, "%s: %d", rptr->getName().c_str(), i);
    auto key = CacheableKey::create(buf);
    sprintf(buf, "testUpdate::%s: value of %d", rptr->getName().c_str(), i);
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
      rptr->create(keys[2], vals[2]);
      FAIL("Should have got NotAuthorizedException during createEntry");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("Trying region clear..");
      rptr->clear();
      FAIL("Should have got NotAuthorizedException for region.clear ops");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    // ASSERT(!rptr->containsKey(keys[2]),   "Key should not have been found in
    // the region");

    try {
      LOG("Trying updateEntry");
      // updateEntry(regionNamesAuth[0], keys[2], nvals[2], false, false);
      rptr->put(keys[2], nvals[2]);
      FAIL("Should have got NotAuthorizedException during updateEntry");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    // ASSERT(!rptr->containsKey(keys[2]),  "Key should not have been found in
    // the
    // region");

    try {
      //auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
     auto keyPtr = CacheableKey::create(keys[2]);
     auto checkPtr =
         std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr));
     if (checkPtr != nullptr) {
       char buf[1024];
       sprintf(buf, "In net search, get returned %s for key %s",
               checkPtr->value().c_str(), keys[2]);
       LOG(buf);
     } else {
       LOG("checkPtr is nullptr");
     }
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    //auto regPtr0 = getHelper()->getRegion( regionNamesAuth[0] );
    if (rptr != nullptr) {
      try {
        LOG("Going to do registerAllKeys");
        //  rptr->registerAllKeys();
        LOG("Going to do unregisterAllKeys");
        //  rptr->unregisterAllKeys();
      }
      HANDLE_NO_NOT_AUTHORIZED_EXCEPTION
    }

    try {
      HashMapOfCacheable entrymap;
      entrymap.clear();
      for (int i = 0; i < 5; i++) {
        entrymap.emplace(CacheableKey::create(i), CacheableInt32::create(i));
      }
      rptr->putAll(entrymap);
      FAIL("PutAll should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      LOG("GetServerKeys check started for READER");
      auto keysvec = rptr->serverKeys();
      LOG("GetServerKeys check passed for READER");
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      std::vector<std::shared_ptr<CacheableKey>>  entrykeys;
      for (int i = 0; i < 5; i++) {
        entrykeys.push_back(CacheableKey::create(i));
      }
      const auto valuesMap = rptr->getAll(entrykeys);
      if (valuesMap.size() > 0) {
        LOG("GetAll completed successfully");
      } else {
        FAIL("GetAll did not complete successfully");
      }
    }
    HANDLE_NO_NOT_AUTHORIZED_EXCEPTION

    try {
      rptr->query("1=1");

      FAIL("Query should not have completed successfully");
    }
    HANDLE_NOT_AUTHORIZED_EXCEPTION

    try {
      // FunctionService::onServer(pool)->execute("securityTest",
      // true)->getResult();
      // FAIL("Function execution should not have completed successfully");
      //auto funcServ = virtualCache->getFunctionService();
      // funcServ->onServer()->execute("securityTest", true)->getResult();
      FunctionService::onServer(virtualCache)
          ->execute("securityTest")
          ->getResult();
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
