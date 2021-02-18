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

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTSECURITYHELPER_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTSECURITYHELPER_H_

#include <ace/Process.h>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "hacks/AceThreadId.h"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableBoolean;
using apache::geode::client::Exception;
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

bool isLocator = false;
bool isLocalServer = false;

const char* locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char* regionNamesAuth[] = {"DistRegionAck"};
std::shared_ptr<CredentialGenerator> credentialGeneratorHandler;

std::string getXmlPath() {
  char xmlPath[1000] = {'\0'};
  const char* path = ACE_OS::getenv("TESTSRC");
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
}

opCodeList::value_type tmpRArr[] = {OP_GET,
                                    OP_GETALL,
                                    OP_REGISTER_INTEREST,
                                    OP_UNREGISTER_INTEREST,
                                    OP_KEY_SET,
                                    OP_CONTAINS_KEY,
                                    OP_QUERY,
                                    OP_REGISTER_CQ};

opCodeList::value_type tmpWArr[] = {OP_CREATE,  OP_UPDATE,     OP_PUTALL,
                                    OP_DESTROY, OP_INVALIDATE, OP_REGION_CLEAR};

opCodeList::value_type tmpAArr[] = {OP_CREATE,       OP_UPDATE,
                                    OP_DESTROY,      OP_INVALIDATE,
                                    OP_REGION_CLEAR, OP_REGISTER_INTEREST,
                                    OP_GET};

#define HANDLE_NOT_AUTHORIZED_EXCEPTION                          \
  catch (const apache::geode::client::NotAuthorizedException&) { \
    LOG("NotAuthorizedException Caught");                        \
    LOG("Success");                                              \
  }                                                              \
  catch (const apache::geode::client::Exception& other) {        \
    LOG(other.getStackTrace().c_str());                          \
    FAIL(other.what());                                          \
  }

#define HANDLE_CACHEWRITER_EXCEPTION                           \
  catch (const apache::geode::client::CacheWriterException&) { \
    LOG("CacheWriterException  Caught");                       \
    LOG("Success");                                            \
  }

#define TYPE_ADMIN_CLIENT 'A'
#define TYPE_WRITER_CLIENT 'W'
#define TYPE_READER_CLIENT 'R'
#define TYPE_USER_CLIENT 'U'

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
      printf("User is %s Pass is %s ",
             config->find("security-username")->value().c_str(),
             (config->find("security-password") != nullptr
                  ? config->find("security-password")->value().c_str()
                  : " not set"));
      break;
    case 'R':
      credentialGeneratorHandler->getAllowedCredentialsForOps(rt, config,
                                                              nullptr);
      printf("User is %s Pass is %s ",
             config->find("security-username")->value().c_str(),
             (config->find("security-password") != nullptr
                  ? config->find("security-password")->value().c_str()
                  : " not set"));
      break;
    case 'A':
      credentialGeneratorHandler->getAllowedCredentialsForOps(ad, config,
                                                              nullptr);
      printf("User is %s Pass is %s ",
             config->find("security-username")->value().c_str(),
             (config->find("security-password") != nullptr
                  ? config->find("security-password")->value().c_str()
                  : " not set"));
      break;
    default:
      break;
  }

  try {
    initClient(true, config);
  } catch (...) {
    throw;
  }
}

// This putThread class is used in
// testThinClientTracking,testThinClientTicket304, testThinClientTicket317

class putThread : public ACE_Task_Base {
 public:
  explicit putThread(std::shared_ptr<Region> r, bool regInt = false,
                     int waitTime = 0) {
    m_reg = r;
    m_regInt = regInt;
    m_numthreads = 1;
    m_numops = 0;
    m_isCallBack = false;
    m_sameKey = false;
    m_waitTime = waitTime;
  }

  void setParams(int opcode, int numofops, int numthreads,
                 bool isCallBack = false, bool sameKey = false,
                 int waitTime = 0) {  //
    m_opcode = opcode;
    m_numops = numofops;
    m_numthreads = numthreads;
    m_isCallBack = isCallBack;
    m_sameKey = sameKey;
    m_waitTime = waitTime;
  }

  void start() {
    m_run = true;
    activate(THR_NEW_LWP | THR_JOINABLE, m_numthreads);
  }

  void stop() {
    if (m_run) {
      m_run = false;
      wait();
    }
  }

  int svc(void) override {
    int ops = 0;
    auto pid = ACE_OS::getpid();
    std::shared_ptr<CacheableKey> key;
    std::shared_ptr<CacheableString> value;
    std::vector<std::shared_ptr<CacheableKey>> keys0;
    char buf[32];
    char valbuf[32];
    if (m_regInt) {
      m_reg->registerAllKeys(false, true);
    }
    if (m_waitTime != 0) {
      ACE_OS::sleep(m_waitTime);
    }
    while (ops++ < m_numops) {
      if (m_sameKey) {
        sprintf(buf, "key-%d", 1);
      } else {
        sprintf(buf, "key-%d", ops);
      }
      key = CacheableKey::create(buf);
      if (m_opcode == 0) {
        if (m_isCallBack) {
          auto boolptr = CacheableBoolean::create("true");
          snprintf(valbuf, sizeof(valbuf), "client1-value%d", ops);
          value = CacheableString::create(valbuf);
          m_reg->put(key, value, boolptr);
        } else {
          snprintf(valbuf, sizeof(valbuf), "client2-value%d", ops);
          value = CacheableString::create(valbuf);
          m_reg->put(key, value);
        }
      } else if (m_opcode == 1) {
        m_reg->get(key);
      } else if (m_opcode == 5) {
        keys0.push_back(key);
        if (ops == m_numops) {
          m_reg->registerKeys(keys0, false, true);
        }
      } else if (m_opcode == 6) {
        m_reg->registerRegex("key-[1-3]", false, true);
      } else {
        try {
          if (m_isCallBack) {
            auto boolptr = CacheableBoolean::create("true");
            m_reg->destroy(key, boolptr);
          } else {
            m_reg->destroy(key);
          }
        } catch (Exception& ex) {
          printf("%d: %" PRIu64 " exception got and exception message = %s\n",
                 pid, hacks::aceThreadId(ACE_OS::thr_self()), ex.what());
        }
      }
    }
    return 0;
  }

  std::shared_ptr<Region> m_reg;
  bool m_run;
  int m_opcode;
  int m_numops;
  int m_numthreads;
  bool m_isCallBack;
  bool m_sameKey;
  bool m_regInt;
  int m_waitTime;
};

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTSECURITYHELPER_H_
