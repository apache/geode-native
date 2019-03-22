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

#include <iostream>

#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <geode/AuthInitialize.hpp>

using namespace apache::geode::client;

constexpr auto SECURITY_USERNAME = "security-username";
constexpr auto SECURITY_PASSWORD = "security-password";

class UserPasswordAuthInit : public AuthInitialize {
public:
  UserPasswordAuthInit() = default;
  
  ~UserPasswordAuthInit() noexcept override = default;
  
  std::shared_ptr<Properties> getCredentials(
    const std::shared_ptr<Properties> &securityprops,
    const std::string &) override {
    std::shared_ptr<Cacheable> userName;
    if (securityprops == nullptr ||
      (userName = securityprops->find(SECURITY_USERNAME)) == nullptr) {
      throw AuthenticationFailedException(
      "UserPasswordAuthInit: user name "
      "property [SECURITY_USERNAME] not set.");
    }
  
    auto credentials = Properties::create();
    credentials->insert(SECURITY_USERNAME, userName->toString().c_str());
    auto passwd = securityprops->find(SECURITY_PASSWORD);
    if (passwd == nullptr) {
      passwd = CacheableString::create("");
    }
    credentials->insert(SECURITY_PASSWORD, passwd->value().c_str());
    return credentials;
  }

  void close() override { return; }
};

int main(int argc, char** argv) {
  auto config = Properties::create();
  config->insert(SECURITY_USERNAME, "root");
  config->insert(SECURITY_PASSWORD, "root");

  auto cacheFactory = CacheFactory(config);
  auto authInitialize = std::make_shared<UserPasswordAuthInit>();
  cacheFactory.set("log-level", "none");
  cacheFactory.setAuthInitialize(authInitialize);

  auto cache = cacheFactory.create();
  auto poolFactory = cache.getPoolManager().createFactory();

  poolFactory.addLocator("localhost", 10334);
  auto pool = poolFactory.create("pool");
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("example_userinfo");

  region->put("rtimmons", "Robert Timmons");
  region->put("scharles", "Sylvia Charles");

  auto user1 = region->get("rtimmons");
  auto user2 = region->get("scharles");
  std::cout << "  rtimmons = "
            << std::dynamic_pointer_cast<CacheableString>(user1)->value()
            << std::endl;
  std::cout << "  scharles = "
            << std::dynamic_pointer_cast<CacheableString>(user2)->value()
            << std::endl;

  cache.close();
}
