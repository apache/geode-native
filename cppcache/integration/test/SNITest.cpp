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
#include <thread>

#include <boost/log/trivial.hpp>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"
#include "framework/TestConfig.h"

namespace snitest {

using apache::geode::client::AuthenticationRequiredException;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::Exception;
using apache::geode::client::RegionShortcut;

class SNITest : public ::testing::Test {
 protected:
  SNITest() {
    certificatePassword = std::string("apachegeode");
    clientSslKeysDir = boost::filesystem::path(
        getFrameworkString(FrameworkVariable::TestClientSslKeysDir));
    currentWorkingDirectory = boost::filesystem::current_path();
    sniConfigPath = boost::filesystem::path(
        getFrameworkString(FrameworkVariable::TestSniConfigPath));
  }

  ~SNITest() override = default;

  void SetUp() override {
    auto systemRVal = 0;
    std::string dockerComposeCmd = "docker-compose -f " +
                                   sniConfigPath.string() +
                                   "/docker-compose.yml" + " up -d";
    const char* dcc = dockerComposeCmd.c_str();

    systemRVal = std::system(dcc);
    if (systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error)
          << "std::system(\"docker-compose\") returned: " << systemRVal;
    }

    systemRVal = std::system(
        "docker exec -t geode gfsh run "
        "--file=/geode/scripts/geode-starter.gfsh");
    if (systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error)
          << "std::system(\"docker exec -t geode gfsh run\") returned: "
          << systemRVal;
    }
  }

  void TearDown() override {
    auto systemRVal = std::system("docker-compose stop");
    if (systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error) << "std::system returned: " << systemRVal;
    }

    systemRVal = std::system("docker container prune -f");
    if (systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error) << "std::system returned: " << systemRVal;
    }
  }

  std::string runDockerCommand(const char* command) {
    std::string commandOutput;
#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"),
                                                   _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
#endif
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
    // 15443/tcp -> 0.0.0.0:32787
    std::size_t colonPosition = proxyString.find(":");
    std::string portNumberString = proxyString.substr((colonPosition + 1));
    return stoi(portNumberString);
  }

  std::string certificatePassword;
  boost::filesystem::path clientSslKeysDir;
  boost::filesystem::path currentWorkingDirectory;
  boost::filesystem::path sniConfigPath;
};

#if defined(_WIN32)
TEST_F(SNITest, DISABLED_connectViaProxyTest) {
#else
TEST_F(SNITest, connectViaProxyTest) {
#endif
  const auto clientTruststore =
      (clientSslKeysDir / boost::filesystem::path("/truststore_sni.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "SNITest.log")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  auto portString = runDockerCommand("docker port haproxy");
  auto portNumber = parseProxyPort(portString);

  cache.getPoolManager()
      .createFactory()
      .setSniProxy("localhost", portNumber)
      .addLocator("locator-maeve", 10334)
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("jellyfish");

  region->put("1", "one");

  cache.close();
}

#if defined(_WIN32)
TEST_F(SNITest, DISABLE_connectWithoutProxyFails) {
#else
TEST_F(SNITest, connectWithoutProxyFails) {
#endif
  const auto clientTruststore =
      (clientSslKeysDir / boost::filesystem::path("/truststore_sni.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "DEBUG")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  cache.getPoolManager()
      .createFactory()
      .setSniProxy("badProxyName", 40000)
      .addLocator("locator-maeve", 10334)
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");
  EXPECT_THROW(region->put("1", "one"),
               apache::geode::client::NotConnectedException);

  cache.close();
}

}  // namespace snitest
