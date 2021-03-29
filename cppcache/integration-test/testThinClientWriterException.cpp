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
#include <ace/Process.h>
#include "TallyListener.hpp"
#include "TallyWriter.hpp"

#include "ThinClientSecurity.hpp"

/*
   1. Using client with security enable.
   2. Check that writer is invoked and listener is not invoked on the client and
   no data in local region after the failure.
   3. Check if writer fails for any key the cache writer exception is beimg
   thrown and no data in local region.
   4. test also check the localput operation.
*/

using apache::geode::client::testframework::security::CredentialGenerator;
using apache::geode::client::testframework::security::OP_CONTAINS_KEY;
using apache::geode::client::testframework::security::OP_GET;
using apache::geode::client::testframework::security::OP_KEY_SET;
using apache::geode::client::testframework::security::OP_REGISTER_INTEREST;
using apache::geode::client::testframework::security::OP_UNREGISTER_INTEREST;
using apache::geode::client::testframework::security::opCodeList;

using apache::geode::client::testing::TallyListener;
using apache::geode::client::testing::TallyWriter;

std::shared_ptr<TallyListener> regListener;
std::shared_ptr<TallyWriter> regWriter;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char *regionNamesAuth[] = {"DistRegionAck"};
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
  if (loopNum > 2) loopNum = 1;
}

opCodeList::value_type tmpRArr[] = {OP_GET, OP_REGISTER_INTEREST,
                                    OP_UNREGISTER_INTEREST, OP_KEY_SET,
                                    OP_CONTAINS_KEY};

#define HANDLE_NOT_AUTHORIZED_EXCEPTION                           \
  catch (const apache::geode::client::NotAuthorizedException &) { \
    LOG("NotAuthorizedException Caught");                         \
    LOG("Success");                                               \
  }                                                               \
  catch (const apache::geode::client::Exception &other) {         \
    LOG(other.getStackTrace());                                   \
    FAIL(other.what());                                           \
  }

#define HANDLE_CACHEWRITER_EXCEPTION                            \
  catch (const apache::geode::client::CacheWriterException &) { \
    LOG("CacheWriterException  Caught");                        \
    LOG("Success");                                             \
  }

#define ADMIN_CLIENT s1p1
#define READER_CLIENT s2p1

void initClientAuth() {
  auto config = Properties::create();
  opCodeList rt(tmpRArr, tmpRArr + sizeof tmpRArr / sizeof *tmpRArr);
  credentialGeneratorHandler->getAuthInit(config);
  credentialGeneratorHandler->getAllowedCredentialsForOps(rt, config, nullptr);
  printf("User is %s Pass is %s ",
         config->find("security-username")->value().c_str(),
         (config->find("security-password") != nullptr
              ? config->find("security-password")->value().c_str()
              : " not set"));
  try {
    initClient(true, config);
  } catch (...) {
    throw;
  }
}

void setCacheWriter(const char *regName,
                    std::shared_ptr<TallyWriter> tallyWriter) {
  auto reg = getHelper()->getRegion(regName);
  auto attrMutator = reg->getAttributesMutator();
  attrMutator->setCacheWriter(tallyWriter);
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

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, StartLocator)
  {
    if (isLocator) {
      CacheHelper::initLocator(1);
      LOG("Locator1 started");
    }
  }
END_TASK_DEFINITION

void startClient() {
  initCredentialGenerator();
  initClientAuth();
  std::shared_ptr<Region> rptr;
  char buf[100];
  int i = 102;
  LOG("Creating region in READER_CLIENT , no-ack, no-cache, with-listener and "
      "writer");
  regListener = std::make_shared<TallyListener>();
  createRegionForSecurity(regionNamesAuth[0], false, true, regListener);
  regWriter = std::make_shared<TallyWriter>();
  setCacheWriter(regionNamesAuth[0], regWriter);
  rptr = getHelper()->getRegion(regionNamesAuth[0]);
  rptr->registerAllKeys();
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
  ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
  ASSERT(regListener->isListenerInvoked() == false,
         "Listener Should not be invoked");
  ASSERT(!rptr->containsKey(key),
         "Key should not have been found in the region");
  rptr->localPut(keys[2], vals[2]);
  ASSERT(rptr->containsKey(keys[2]),
         "Key should have been found in the region");
  ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
  ASSERT(regListener->isListenerInvoked() == true,
         "Listener Should be invoked");
  try {
    LOG("Trying updateEntry");
    regListener->resetListnerInvokation();
    updateEntry(regionNamesAuth[0], keys[2], nvals[2], false, false);
    FAIL("Should have got NotAuthorizedException during updateEntry");
  }
  HANDLE_NOT_AUTHORIZED_EXCEPTION
  ASSERT(regWriter->isWriterInvoked() == true, "Writer Should be invoked");
  ASSERT(regListener->isListenerInvoked() == false,
         "Listener Should not be invoked");
  ASSERT(rptr->containsKey(keys[2]),
         "Key should have been found in the region");
  verifyEntry(regionNamesAuth[0], keys[2], vals[2]);
  regWriter->setWriterFailed();
  try {
    LOG("Testing CacheWriterException");
    updateEntry(regionNamesAuth[0], keys[2], nvals[2], false, false);
    FAIL("Should have got NotAuthorizedException during updateEntry");
  }
  HANDLE_CACHEWRITER_EXCEPTION
  LOG("StepThree complete.");
}

DUNIT_TASK_DEFINITION(READER_CLIENT, StartClientPoolLocator)
  { startClient(); }
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

DUNIT_TASK_DEFINITION(ADMIN_CLIENT, CloseLocator)
  {
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(READER_CLIENT, CloseCacheReader)
  { cleanProc(); }
END_TASK_DEFINITION

void doThinClientWriterException() {
  CALL_TASK(StartLocator);
  CALL_TASK(StartServer1);
  CALL_TASK(StartClientPoolLocator);
  CALL_TASK(CloseCacheReader);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientWriterException(); }
END_MAIN
