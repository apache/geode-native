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

#include "UserPasswordAuthInit.hpp"

#include <geode/CacheableString.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Properties.hpp>

#include "securityimpl_export.h"

#define SECURITY_USERNAME "security-username"
#define SECURITY_PASSWORD "security-password"

namespace apache {
namespace geode {
namespace client {

extern "C" {
SECURITYIMPL_EXPORT AuthInitialize* createUserPasswordAuthInitInstance() {
  return new UserPasswordAuthInit();
}
}
std::shared_ptr<Properties> UserPasswordAuthInit::getCredentials(
    const std::shared_ptr<Properties>& securityprops, const std::string&) {
  // LOG_DEBUG("UserPasswordAuthInit: inside userPassword::getCredentials");
  std::shared_ptr<Cacheable> userName;
  if (securityprops == nullptr ||
      (userName = securityprops->find(SECURITY_USERNAME)) == nullptr) {
    throw AuthenticationFailedException(
        "UserPasswordAuthInit: user name "
        "property [" SECURITY_USERNAME "] not set.");
  }

  auto credentials = Properties::create();
  credentials->insert(SECURITY_USERNAME, userName->toString());
  auto passwd = securityprops->find(SECURITY_PASSWORD);
  // If password is not provided then use empty string as the password.
  if (passwd == nullptr) {
    passwd = CacheableString::create("");
  }
  credentials->insert(SECURITY_PASSWORD, passwd->toString());
  // LOG_DEBUG("UserPasswordAuthInit: inserted username:password - %s:%s",
  //    userName->toString().c_str(), passwd->toString().c_str());
  return credentials;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
