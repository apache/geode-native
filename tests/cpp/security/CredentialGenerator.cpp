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

#include "CredentialGenerator.hpp"

// Disable warning for "extra qualifications" here.  One of the boost log
// headers triggers this warning.  Note: use of disable pragma here is
// intentional - attempts to use push/pop as you ordinarily should just
// yielded a gripe from the MS tools that "warning number '4596' is not a
// valid compiler warning". re-enabling the warning after the include
// fails in the same way, so just leave it disabled for the rest of the
// file.  This is safe, since the warning can only trigger inside a class
// declaration, of which there are none in this file.
#ifdef WIN32
#pragma warning(disable : 4596)
#endif

#include <boost/log/trivial.hpp>

#include "DummyCredentialGenerator.hpp"
#include "DummyCredentialGenerator2.hpp"
#include "DummyCredentialGenerator3.hpp"
#include "LdapUserCredentialGenerator.hpp"
#include "NoopCredentialGenerator.hpp"
#include "PkcsCredentialGenerator.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testframework {
namespace security {

CredentialGenerator::registeredClassMap* CredentialGenerator::generatormap =
    nullptr;
std::shared_ptr<CredentialGenerator> CredentialGenerator::create(
    std::string scheme) {
  if (generators().find(scheme) != generators().end()) {
    return generators()[scheme];

    // first call to create, nothing will be registered until now.
  } else if (generators().size() == 0) {
    registerScheme(
        std::shared_ptr<CredentialGenerator>(new CredentialGenerator()));
    registerScheme(
        std::shared_ptr<CredentialGenerator>(new DummyCredentialGenerator()));
    registerScheme(
        std::shared_ptr<CredentialGenerator>(new DummyCredentialGenerator2()));
    registerScheme(
        std::shared_ptr<CredentialGenerator>(new DummyCredentialGenerator3()));
    registerScheme(std::shared_ptr<CredentialGenerator>(
        new LdapUserCredentialGenerator()));
    registerScheme(
        std::shared_ptr<CredentialGenerator>(new PKCSCredentialGenerator()));
    registerScheme(
        std::shared_ptr<CredentialGenerator>(new NoopCredentialGenerator()));
    return create(scheme);

  } else {
    return std::shared_ptr<CredentialGenerator>(new CredentialGenerator());
  }
}

void CredentialGenerator::dump() {
  BOOST_LOG_TRIVIAL(info) << "dumping all registered classes ";
  registeredClassMap::iterator it = generators().begin();
  while (it != generators().end()) {
    BOOST_LOG_TRIVIAL(info) << ((*it).second)->toString();
    it++;
  }
}

void CredentialGenerator::getAuthInit(std::shared_ptr<Properties>& prop) {
  std::string authinit = getClientAuthInitLoaderFactory();
  if (!authinit.empty()) {
    BOOST_LOG_TRIVIAL(info) << "Authentication initializer : " << authinit
                            << " library " << getClientAuthInitLoaderLibrary();

    prop->insert("security-client-auth-factory", authinit.c_str());
    prop->insert("security-client-auth-library",
                 getClientAuthInitLoaderLibrary().c_str());
  }
}

std::string CredentialGenerator::getServerCmdParams(std::string securityParams,
                                                    std::string workingDir,
                                                    bool userMode) {
  std::string securityCmdStr;
  BOOST_LOG_TRIVIAL(info) << "User mode is " << userMode;
  if (securityParams.find("authenticator") != std::string::npos &&
      !getClientAuthenticator().empty()) {
    securityCmdStr = getInitArgs(workingDir, userMode);
    securityCmdStr +=
        std::string(" --J=-Dgemfire.security-client-authenticator=") +
        getClientAuthenticator();
  }
  if ((securityParams.find("authorizer") != std::string::npos) &&
      (!getClientAuthorizer().empty())) {
    securityCmdStr += std::string(" --J=-Dgemfire.security-client-accessor=") +
                      getClientAuthorizer();
  }
  if ((securityParams.find("authorizerPP") != std::string::npos) &&
      (!getClientAuthorizer().empty())) {
    securityCmdStr +=
        std::string(" --J=-Dgemfire.security-client-accessor-pp=") +
        getClientAuthorizer();
  }
  if (m_id == ID_PKI) {
    securityCmdStr +=
        std::string(" --J=-Dgemfire.security-publickey-filepath=") +
        getPublickeyfile();
    securityCmdStr +=
        std::string(" --J=-Dgemfire.security-publickey-pass=geode");
  }
  if ((securityParams.find("dummy") != std::string::npos) &&
      (!getClientDummyAuthorizer().empty())) {
    securityCmdStr += std::string(" --J=-Dgemfire.security-client-accessor=") +
                      getClientDummyAuthorizer();
  }
#ifdef __COMPILE_DUNIT_  // lets suppress -N option in case of unit tests.
  int idx;
  while ((idx = securityCmdStr.find("--J=-Dgemfire.", 0)) >= 0) {
    securityCmdStr.replace(idx, 2, "");
  }
#endif
  return securityCmdStr;
}

void CredentialGenerator::getValidCredentials(std::shared_ptr<Properties>&) {}

void CredentialGenerator::getInvalidCredentials(std::shared_ptr<Properties>&) {}

void CredentialGenerator::getAllowedCredentialsForOps(
    opCodeList&, std::shared_ptr<Properties>&, stringList*) {}

void CredentialGenerator::getDisallowedCredentialsForOps(
    opCodeList&, std::shared_ptr<Properties>&, stringList*) {}

std::string CredentialGenerator::getInitArgs(std::string, bool) { return ""; }

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache
