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

#include <future>
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
    TearDown();

    std::string dockerComposeCmd = "docker-compose -f " +
                                   sniConfigPath.string() +
                                   "/docker-compose.yml" + " up -d";

    runProcess(dockerComposeCmd);

    runProcess(
        "docker exec -t geode gfsh run "
        "--file=/geode/scripts/geode-starter.gfsh");
  }

  void TearDown() override { cleanupDocker(); }

  void cleanupDocker() {
    runProcess("docker stop geode");
    runProcess("docker stop haproxy");
    runProcess("docker container prune -f");
  }

  std::string runProcess(std::string command) {
    const char* cstrCommand = command.c_str();
    std::string commandOutput;
#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cstrCommand, "r"),
                                                   _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cstrCommand, "r"),
                                                  pclose);
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

TEST_F(SNITest, connectViaProxy) {
  const auto clientTruststore =
      (clientSslKeysDir / boost::filesystem::path("/truststore_sni.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "SNITest.log")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  auto portString = runProcess("docker port haproxy");
  auto portNumber = parseProxyPort(portString);

  cache.getPoolManager()
      .createFactory()
      .setSniProxy("localhost", portNumber)
      .addLocator("locator-maeve", 20220)
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("jellyfish");

  region->put("1", "one");
  auto val = std::dynamic_pointer_cast<CacheableString>(region->get("1"));
  EXPECT_EQ("one", val->value());

  cache.close();
}

TEST_F(SNITest, connectWithoutProxyFails) {
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
      .addLocator("locator-maeve", 20220)
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");
  EXPECT_THROW(region->put("1", "one"),
               apache::geode::client::NotConnectedException);

  cache.close();
}

TEST_F(SNITest, dropSNIProxy) {
  const auto clientTruststore =
      (clientSslKeysDir / boost::filesystem::path("/truststore_sni.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "SNITest.log")
                   .set("ssl-enabled", "true")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  auto portString = runProcess("docker port haproxy");
  auto proxyPort = parseProxyPort(portString);

  cache.getPoolManager()
      .createFactory()
      .setSniProxy("localhost", proxyPort)
      .addLocator("locator-maeve", 20220)
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("jellyfish");

  region->put("1", "one");
  auto val = std::dynamic_pointer_cast<CacheableString>(region->get("1"));
  EXPECT_EQ("one", val->value());

  runProcess("docker stop haproxy");
  runProcess("docker container prune -f");

  EXPECT_THROW(region->put("1", "one"),
               apache::geode::client::NotConnectedException);

  std::string startProxyArgs = "-f " + sniConfigPath.string() +
                               "/docker-compose.yml "
                               "run -d --name haproxy "
                               "--publish " +
                               std::to_string(proxyPort) + ":15443 haproxy";

  runProcess("docker-compose " + startProxyArgs);

  val = std::dynamic_pointer_cast<CacheableString>(region->get("1"));
  EXPECT_EQ("one", val->value());

  cache.close();
}

}  // namespace snitest
