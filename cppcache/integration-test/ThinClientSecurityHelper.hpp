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

#include <boost/process.hpp>
#include <boost/lexical_cast.hpp>

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

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char* regionNamesAuth[] = {"DistRegionAck"};
std::shared_ptr<CredentialGenerator> credentialGeneratorHandler;

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
      std::cout << "User is " << config->find("security-username")->value()
                << " Pass is "
                << (config->find("security-password") != nullptr
                        ? config->find("security-password")->value()
                        : " not set")
                << "\n";
      break;
    case 'R':
      credentialGeneratorHandler->getAllowedCredentialsForOps(rt, config,
                                                              nullptr);
      std::cout << "User is " << config->find("security-username")->value()
                << " Pass is "
                << (config->find("security-password") != nullptr
                        ? config->find("security-password")->value()
                        : " not set")
                << "\n";
      break;
    case 'A':
      credentialGeneratorHandler->getAllowedCredentialsForOps(ad, config,
                                                              nullptr);
      std::cout << "User is " << config->find("security-username")->value()
                << " Pass is "
                << (config->find("security-password") != nullptr
                        ? config->find("security-password")->value()
                        : " not set")
                << "\n";
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

// This PutThread class is used in
// testThinClientTracking,testThinClientTicket304, testThinClientTicket317

class PutThread {
 public:
  explicit PutThread(std::shared_ptr<Region> r, bool regInt = false,
                     int waitTime = 0)
      : region_{r},
        numOps_{0},
        numThreads_{1},
        isCallback_{false},
        sameKey_{false},
        regInt_{regInt},
        waitTime_{waitTime} {
  }

  void setParams(int opcode, int numofops, int numthreads,
                 bool isCallBack = false, bool sameKey = false,
                 int waitTime = 0) {  //
    opcode_ = opcode;
    numOps_ = numofops;
    numThreads_ = numthreads;
    isCallback_ = isCallBack;
    sameKey_ = sameKey;
    waitTime_ = waitTime;
  }

  void start() {
    for(auto i = 0; i < numThreads_; ++i) {
      threads_.emplace_back([this](){
        run();
      });
    }
  }

  void wait() {
    for(auto& thread : threads_) {
      if(thread.joinable()) {
        thread.join();
      }
    }
  }

  void stop() {
      wait();
  }

  void run() {
    int ops = 0;
    std::string key_str;
    std::shared_ptr<CacheableKey> key;
    std::shared_ptr<CacheableString> value;
    std::vector<std::shared_ptr<CacheableKey>> keys0;

    auto pid = boost::this_process::get_id();
    if (regInt_) {
      region_->registerAllKeys(false, true);
    }
    if (waitTime_ != 0) {
      std::this_thread::sleep_for(std::chrono::seconds{waitTime_});
    }
    while (ops++ < numOps_) {
      if (sameKey_) {
        key_str = "key-1";
      } else {
        key_str = "key-" + std::to_string(ops);
      }

      key = CacheableKey::create(key_str);
      if (opcode_ == 0) {
        std::string value_str;

        if (isCallback_) {
          auto boolptr = CacheableBoolean::create("true");
          value_str = "client1-value" + std::to_string(ops);
          value = CacheableString::create(value_str);
          region_->put(key, value, boolptr);
        } else {
          value_str = "client2-value" + std::to_string(ops);
          value = CacheableString::create(value_str);
          region_->put(key, value);
        }
      } else if (opcode_ == 1) {
        region_->get(key);
      } else if (opcode_ == 5) {
        keys0.push_back(key);
        if (ops == numOps_) {
          region_->registerKeys(keys0, false, true);
        }
      } else if (opcode_ == 6) {
        region_->registerRegex("key-[1-3]", false, true);
      } else {
        try {
          if (isCallback_) {
            auto boolptr = CacheableBoolean::create("true");
            region_->destroy(key, boolptr);
          } else {
            region_->destroy(key);
          }
        } catch (Exception& ex) {
          auto tid =
              boost::lexical_cast<std::string>(std::this_thread::get_id());
          std::cout << pid << ": " << tid << " exception got and exception message = " << ex.what() << "\n";
        }
      }
    }
  }

 protected:
  std::shared_ptr<Region> region_;

  int opcode_;
  int numOps_;
  int numThreads_;
  bool isCallback_;
  bool sameKey_;
  bool regInt_;
  int waitTime_;

  std::vector<std::thread> threads_;
};

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTSECURITYHELPER_H_
