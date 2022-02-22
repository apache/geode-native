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

#include <geode/AuthInitialize.hpp>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "ThinClientSecurity.hpp"

using apache::geode::client::AuthenticationFailedException;
using apache::geode::client::AuthInitialize;
using apache::geode::client::Cacheable;
using apache::geode::client::testframework::security::CredentialGenerator;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char *regionNamesAuth[] = {"DistRegionAck", "DistRegionNoAck"};
std::shared_ptr<CredentialGenerator> credentialGeneratorHandler;

#define SECURITY_USERNAME "security-username"
#define SECURITY_PASSWORD "security-password"
class UserPasswordAuthInit : public AuthInitialize {
 public:
  UserPasswordAuthInit() = default;

  ~UserPasswordAuthInit() noexcept override = default;

  std::shared_ptr<Properties> getCredentials(
      const std::shared_ptr<Properties> &securityprops,
      const std::string &) override {
    // LOGDEBUG("UserPasswordAuthInit: inside userPassword::getCredentials");
    std::shared_ptr<Cacheable> userName;
    if (securityprops == nullptr ||
        (userName = securityprops->find(SECURITY_USERNAME)) == nullptr) {
      throw AuthenticationFailedException(
          "UserPasswordAuthInit: user name "
          "property [" SECURITY_USERNAME "] not set.");
    }

    auto credentials = Properties::create();
    credentials->insert(SECURITY_USERNAME, userName->toString().c_str());
    auto passwd = securityprops->find(SECURITY_PASSWORD);
    // If password is not provided then use empty string as the password.
    if (passwd == nullptr) {
      passwd = CacheableString::create("");
    }
    credentials->insert(SECURITY_PASSWORD, passwd->value().c_str());
    // LOGDEBUG("UserPasswordAuthInit: inserted username:password - %s:%s",
    //    userName->toString().c_str(), passwd->toString().c_str());
    return credentials;
  }

  void close() override { return; }

 private:
};

void initClientAuth() {
  auto config = Properties::create();
  config->insert("security-password", "root");
  config->insert("security-username", "root");

  auto authInitialize = std::make_shared<UserPasswordAuthInit>();

  try {
    cacheHelper = new CacheHelper(true, authInitialize, config);
  } catch (...) {
    throw;
  }
}

#define CLIENT1 s1p1
#define LOCATORSERVER s2p2

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateLocator)
  {
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(LOCATORSERVER, CreateServer1)
  {
    std::string cmdServerAuthenticator;

    try {
      if (isLocalServer) {
        CacheHelper::initServer(1, {}, locHostPort);
        LOG("Server1 started");
      }
    } catch (...) {
      std::cout << "this is some exception";
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, TestAuthentication)
  {
    initClientAuth();
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
    LOG("TestAuthentication Completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
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
  CALL_TASK(TestAuthentication);
  CALL_TASK(CloseCache1);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseLocator);
}

DUNIT_MAIN
  { doThinClientSecurityAuthentication(); }
END_MAIN
