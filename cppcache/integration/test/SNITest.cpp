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

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>
#include <boost/log/trivial.hpp>

#include "framework/Cluster.h"

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
    currentWorkingDirectory = boost::filesystem::current_path();
  }

  ~SNITest() override = default;

  void SetUp() override {
    auto systemRVal = 0;
#if defined(_WIN32)
    std::string sniDir(currentWorkingDirectory.string());
    sniDir += "/../sni-test-config";
    SetCurrentDirectory(sniDir.c_str());
#else
    systemRVal = chdir("./sni-test-config");
    if(systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error) << "chdir returned: " << systemRVal;
    }
#endif

    systemRVal = std::system("docker-compose up -d");
    if(systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error) << "std::system(\"docker-compose up -d\") returned: " << systemRVal;
    }    

    systemRVal = std::system(
        "docker exec -t geode gfsh run "
        "--file=/geode/scripts/geode-starter.gfsh");
    if(systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error) << "std::system(\"docker exec -t geode gfsh run\") returned: " << systemRVal;
    }           
  }

  void TearDown() override {
    auto systemRVal = std::system("docker-compose stop");
    if(systemRVal == -1) {
      BOOST_LOG_TRIVIAL(error) << "std::system returned: " << systemRVal;
    }   
  }

  std::string makeItSo(const char* command) {
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
  boost::filesystem::path currentWorkingDirectory;
};

TEST_F(SNITest, DISABLED_connectViaProxyTest) {
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path("sni-test-config/geode-config/truststore.jks"));

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
       boost::filesystem::path("sni-test-config/geode-config/truststore.jks"));

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
  EXPECT_THROW(region->put("1", "one"),
               apache::geode::client::NotConnectedException);

  cache.close();
}

TEST_F(SNITest, doNothingTest) {
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path("sni-test-config/geode-config/truststore.jks"));

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

}  // namespace snitest
