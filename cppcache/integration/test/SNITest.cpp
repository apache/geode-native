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

#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"

namespace ssltest {

using apache::geode::client::AuthenticationRequiredException;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::Exception;
using apache::geode::client::RegionShortcut;

class SNITest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if their bodies would
  // be empty.

  SNITest() {
    // You can do set-up work for each test here.
    certificatePassword = std::string("apachegeode");
    currentWorkingDirectory = boost::filesystem::current_path();
  }

  ~SNITest() override = default;
  // You can do clean-up work that doesn't throw exceptions here.

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:
  void SetUp() override {
    // Code here will be called immediately after the constructor (right
    // before each test).
#if defined(_WINDOWS)
    auto rVal = SetCurrentDirectory("./sni-test-config");
#else
    auto rVal = chdir("./sni-test-config");
#endif

    std::system("docker-compose up -d");
  }

  void TearDown() override {
    // Code here will be called immediately after each test (right
    // before the destructor).
    std::system("docker-compose stop");
  }

  std::string makeItSo(const char* command) {
    std::string commandOutput;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    std::array<char, 128> charBuff;
    if (!pipe) {
      throw std::runtime_error("Failed on the POPEN");
    }
    while (fgets(charBuff.data(), charBuff.size(), pipe.get()) != nullptr) {
      commandOutput += charBuff.data();
    }
    return commandOutput;
  }

  int parseProxyPort(std::string proxyString) {
    //15443/tcp -> 0.0.0.0:32787
    std::size_t colonPosition = proxyString.find(":");
    std::string portNumberString = proxyString.substr((colonPosition+1));
    return stoi(portNumberString);
  }

  // Class members declared here can be used by all tests in the test suite
  // for Ssl.
  Cluster cluster = Cluster{LocatorCount{1}, ServerCount{1}};
  std::string certificatePassword;
  boost::filesystem::path currentWorkingDirectory;
};

TEST_F(SNITest, connectViaProxyTest) {
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "sni-test-config/geode-config/truststore.jks"));

  auto cache = CacheFactory()
                   .set("log-level", "DEBUG")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  auto portString = makeItSo("docker port haproxy");
  auto portNumber = parseProxyPort(portString);
  
   cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", portNumber)
      .create("pool");

   auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

   region->put("1", "one");

  cache.close();
}

TEST_F(SNITest, connectionFailsTest) {
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "sni-test-config/geode-config/truststore.jks"));

  auto cache = CacheFactory()
                   .set("log-level", "DEBUG")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

   cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");

   auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");
EXPECT_THROW(
   region->put("1", "one"), apache::geode::client::NotConnectedException);

  cache.close();
}

TEST_F(SNITest, doNothingTest) {
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "sni-test-config/geode-config/truststore.jks"));

  auto cache = CacheFactory()
                   .set("log-level", "DEBUG")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

   cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");

  cache.close();
}

}  // namespace ssltest
