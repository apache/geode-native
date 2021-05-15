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
#include "ThinClientHelper.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include "ThinClientSecurity.hpp"
#include <geode/CacheTransactionManager.hpp>

#define CORRECT_CREDENTIALS 'C'
#define INCORRECT_CREDENTIALS 'I'
#define NOT_PROVIDED_CREDENTIALS 'N'

using apache::geode::client::testframework::security::CredentialGenerator;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char *regionNamesAuth[] = {"DistRegionAck", "DistRegionNoAck"};
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

void initClientAuth(char credentialsType) {
  auto config = Properties::create();
  if (credentialGeneratorHandler == nullptr) {
    FAIL("credentialGeneratorHandler is nullptr");
  }
  bool insertAuthInit = true;
  switch (credentialsType) {
    case 'C':
      credentialGeneratorHandler->getValidCredentials(config);
      config->insert("security-password",
                     config->find("security-username")->value().c_str());
      printf("Username is %s and Password is %s ",
             config->find("security-username")->value().c_str(),
             config->find("security-password")->value().c_str());
      break;
    case 'I':
      credentialGeneratorHandler->getInvalidCredentials(config);
      config->insert("security-password", "junk");
      printf("Username is %s and Password is %s ",
             config->find("security-username")->value().c_str(),
             config->find("security-password")->value().c_str());
      break;
    case 'N':
    default:
      insertAuthInit = false;
      break;
  }
  if (insertAuthInit) {
    // config->insert(
    // "security-client-auth-factory","createUserPasswordAuthInitInstance" );
    // config->insert( "security-client-auth-library","authinitImpl" );
    credentialGeneratorHandler->getAuthInit(config);
  }

  try {
    initClient(true, config);
  } catch (...) {
    throw;
  }
}

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define CLIENT3 s2p1
#define LOCATORSERVER s2p2

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateLocator)
  {
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateServer1)
  {
    initCredentialGenerator();
    std::string cmdServerAuthenticator;
    if (credentialGeneratorHandler == nullptr) {
      FAIL("credentialGeneratorHandler is nullptr");
    }

    try {
      if (isLocalServer) {
        cmdServerAuthenticator = credentialGeneratorHandler->getServerCmdParams(
            "authenticator", getXmlPath());
        printf("Input to server cmd is -->  %s",
               cmdServerAuthenticator.c_str());
        CacheHelper::initServer(
            1, nullptr, locHostPort,
            const_cast<char *>(cmdServerAuthenticator.c_str()));
        LOG("Server1 started");
      }
    } catch (...) {
      printf("this is some exception");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateServer2)
  {
    std::string cmdServerAuthenticator2;
    cmdServerAuthenticator2 = credentialGeneratorHandler->getServerCmdParams(
        "authenticator", getXmlPath());
    printf("Input to server cmd is -->  %s", cmdServerAuthenticator2.c_str());
    CacheHelper::initServer(
        2, "cacheserver_notify_subscription2.xml", locHostPort,
        const_cast<char *>(cmdServerAuthenticator2.c_str()));
    LOG("Server2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initCredentialGenerator();
    try {
      initClientAuth(INCORRECT_CREDENTIALS);
    } catch (
        const apache::geode::client::AuthenticationFailedException &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
    }

    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      FAIL("Should have thrown AuthenticationFailedException.");
    } catch (
        const apache::geode::client::AuthenticationFailedException &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
      FAIL("Only AuthenticationFailedException is expected");
    }
    LOG("StepOne Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepTwo)
  {
    initClientAuth(CORRECT_CREDENTIALS);
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      regPtr0->containsKeyOnServer(
          apache::geode::client::CacheableKey::create(keys[0]));
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      FAIL(other.what());
    }
    LOG("Handshake  and  Authentication successfully completed");
    LOG("StepTwo Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepThree)
  {
    initCredentialGenerator();
    initClientAuth(CORRECT_CREDENTIALS);
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      FAIL(other.what());
    }
    LOG("Handshake  and  Authentication successfully completed");
  }
  LOG("StepThree Completed");
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, StepFour)
  {
    initCredentialGenerator();
    try {
      initClientAuth(NOT_PROVIDED_CREDENTIALS);
    } catch (
        const apache::geode::client::AuthenticationRequiredException &other) {
      LOG(other.getStackTrace());
      FAIL(other.what());
    }

    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      FAIL("Should have thrown AuthenticationRequiredException.");
    } catch (
        const apache::geode::client::AuthenticationRequiredException &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
      FAIL("Only AuthenticationRequiredException is expected");
    }
    LOG("StepFour Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFive)
  {
    SLEEP(80);
    try {
      createRegionForSecurity(regionNamesAuth[1], USE_ACK, true);
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[0]);
      auto keyPtr = CacheableKey::create(keys[0]);
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr0->get(keyPtr));
      if (checkPtr != nullptr && !strcmp(nvals[0], checkPtr->value().c_str())) {
        LOG("checkPtr is not null");
        char buf[1024];
        sprintf(buf, "In net search, get returned %s for key %s",
                checkPtr->value().c_str(), keys[0]);
        LOG(buf);
      } else {
        LOG("checkPtr is nullptr");
      }
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      FAIL(other.what());
    }
    LOG("Handshake  and  Authentication successfully completed after FailOver");
    LOG("StepFive Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSix)
  {
    initClientAuth(CORRECT_CREDENTIALS);
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true);
      createEntry(regionNamesAuth[0], keys[0], vals[0]);
      updateEntry(regionNamesAuth[0], keys[0], nvals[0]);
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      FAIL(other.what());
    }
    LOG("Handshake  and  Authentication successfully completed");
    LOG("StepSix Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSeven)
  {
    try {
      initClientAuth(INCORRECT_CREDENTIALS);
    } catch (
        const apache::geode::client::AuthenticationFailedException &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
    }
    LOG("Setting JavaConnectionPoolSize to 0 ");
    CacheHelper::setJavaConnectionPoolSize(0);
    SLEEP(500);
    try {
      createRegionForSecurity(regionNamesAuth[0], USE_ACK, true, nullptr, true,
                              0);
      FAIL("Should have thrown AuthenticationFailedException.");
    } catch (
        const apache::geode::client::AuthenticationFailedException &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      LOG(other.what());
      FAIL("Only AuthenticationFailedException is expected");
    }
    LOG("StepSeven Completed");
  }
END_TASK_DEFINITION

void createEntryTx(const char *name, const char *key, const char *value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  // ASSERT( !regPtr->containsKey( keyPtr ), "Key should not have been found in
  // region." );
  // ASSERT( !regPtr->containsValueForKey( keyPtr ), "Value should not have been
  // found in region." );

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  // verifyEntry( name, key, value );
  LOG("Entry created.");
}

void updateEntryTx(const char *name, const char *key, const char *value) {
  LOG("updateEntry() entered.");
  fprintf(stdout, "Updating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  ASSERT(regPtr->containsValueForKey(keyPtr),
         "Value should have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

DUNIT_TASK_DEFINITION(CLIENT1, StepEight)
  {
    initClientAuth(CORRECT_CREDENTIALS);
    try {
      createRegionForSecurity(regionNamesAuth[1], USE_ACK, true);
      auto regPtr0 = getHelper()->getRegion(regionNamesAuth[1]);
      auto txManager = getHelper()->getCache()->getCacheTransactionManager();
      LOG("txManager got");
      txManager->begin();
      LOG("txManager begin done");
      createEntryTx(regionNamesAuth[1], "TxKey", "TxValue");
      LOG("createEntryTx done");
      txManager->commit();
      LOG("txManager commit done");

      auto keyPtr = CacheableKey::create("TxKey");
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr0->get(keyPtr));
      ASSERT(checkPtr != nullptr, "Value not found.");
      LOG_INFO("checkPtr->value().c_str() = %s ", checkPtr->value().c_str());
      ASSERT(strcmp("TxValue", checkPtr->value().c_str()) == 0,
             "Value not correct.");
      if (checkPtr != nullptr &&
          !strcmp("TxValue", checkPtr->value().c_str())) {
        LOG("checkPtr is not null");
        char buf[1024];
        sprintf(buf, "In net search, get returned %s for key %s",
                checkPtr->value().c_str(), "TxKey");
        LOG(buf);
      } else {
        LOG("checkPtr is nullptr");
      }

      txManager->begin();
      LOG("txManager begin done");
      createEntryTx(regionNamesAuth[1], "TxKey", "TxNewValue");
      LOG("createEntryTx done");
      txManager->rollback();
      LOG("txManager rollback done");

      checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr0->get(keyPtr));
      ASSERT(checkPtr != nullptr, "Value not found.");
      ASSERT(strcmp("TxValue", checkPtr->value().c_str()) == 0,
             "Value not correct.");
      if (checkPtr != nullptr &&
          !strcmp("TxValue", checkPtr->value().c_str())) {
        LOG("checkPtr is not null");
        char buf[1024];
        sprintf(buf, "In net search, get returned %s for key %s",
                checkPtr->value().c_str(), "TxKey");
        LOG(buf);
      } else {
        LOG("checkPtr is nullptr");
      }

    } catch (const apache::geode::client::Exception &other) {
      LOG(other.getStackTrace());
      FAIL(other.what());
    }

    LOG("StepEight Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT3, CloseCache3)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

void doThinClientSecurityAuthentication() {
  CALL_TASK(CreateLocator);
  CALL_TASK(CreateServer1);
  CALL_TASK(StepOne);
  CALL_TASK(CreateServer2);
  CALL_TASK(CloseCache1);
  CALL_TASK(StepTwo);
  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(CloseServer1);
  CALL_TASK(StepFive);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(StepSix);
  CALL_TASK(StepSeven);
  CALL_TASK(StepEight);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseCache3);
  CALL_TASK(CloseServer2);
  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientSecurityAuthentication(); }
END_MAIN
