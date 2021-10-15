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

#ifndef GEODE_SECURITY_PKCSCREDENTIALGENERATOR_H_
#define GEODE_SECURITY_PKCSCREDENTIALGENERATOR_H_

#include <random>

#include "CredentialGenerator.hpp"
#include "XmlAuthzCredentialGenerator.hpp"

const char SECURITY_USERNAME[] = "security-username";
const char KEYSTORE_FILE_PATH[] = "security-keystorepath";
const char KEYSTORE_ALIAS[] = "security-alias";
const char KEYSTORE_PASSWORD[] = "security-keystorepass";

#include <ace/ACE.h>
#include <ace/OS.h>

namespace apache {
namespace geode {
namespace client {
namespace testframework {
namespace security {

template <typename T>
T randomValue(T minValue, T maxValue) {
  static thread_local std::default_random_engine generator(
      std::random_device{}());
  return std::uniform_int_distribution<T>{minValue, maxValue}(generator);
}

class PKCSCredentialGenerator : public CredentialGenerator {
 public:
  PKCSCredentialGenerator() : CredentialGenerator(ID_PKI, "PKCS") {}

  std::string getInitArgs(std::string workingDir, bool) override;

  std::string getClientAuthInitLoaderFactory() override {
    return "createPKCSAuthInitInstance";
  }
  std::string getClientAuthInitLoaderLibrary() override {
    return "securityImpl";
  }
  std::string getClientAuthenticator() override {
    return "javaobject.PKCSAuthenticator.create";
  }
  std::string getClientAuthorizer() override {
    return "javaobject.XmlAuthorization.create";
  }
  std::string getClientDummyAuthorizer() override {
    return "javaobject.DummyAuthorization.create";
  }

  void insertKeyStorePath(std::shared_ptr<Properties>& p,
                          const std::string& username) {
    auto path =
        ACE_OS::getenv("TESTSRC")
            ? std::string(ACE_OS::getenv("TESTSRC"))
            : std::string(ACE_OS::getenv("BUILDDIR")) + "/framework/data";
    p->insert(KEYSTORE_FILE_PATH, path + "/keystore/" + username + ".keystore");
  }

  void setPKCSProperties(std::shared_ptr<Properties>& p, const std::string& username) {
    p->insert(SECURITY_USERNAME, "geode");
    p->insert(KEYSTORE_ALIAS, username);
    p->insert(KEYSTORE_PASSWORD, "geode");
    insertKeyStorePath(p, username);
  }

  void getValidCredentials(std::shared_ptr<Properties>& p) override;

  void getInvalidCredentials(std::shared_ptr<Properties>& p) override;

  void getAllowedCredentialsForOps(opCodeList& opCodes,
                                   std::shared_ptr<Properties>& p,
                                   stringList* regionNames) override {
    XmlAuthzCredentialGenerator authz(id());
    authz.getAllowedCredentials(opCodes, p, regionNames);
    const char* username = p->find("security-alias")->value().c_str();
    insertKeyStorePath(p, username);
  }

  void getDisallowedCredentialsForOps(opCodeList& opCodes,
                                      std::shared_ptr<Properties>& p,
                                      stringList* regionNames) override {
    XmlAuthzCredentialGenerator authz(id());
    authz.getDisallowedCredentials(opCodes, p, regionNames);
    const char* username = p->find("security-alias")->value().c_str();
    insertKeyStorePath(p, username);
  }
};

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SECURITY_PKCSCREDENTIALGENERATOR_H_
