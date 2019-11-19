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

const auto badClientTruststore = boost::filesystem::path(
    "/Users/pivotal/Workspace/geode-native-install/examples/build/cpp/"
    "sslputget/ClientSslKeys/client_truststore.pem");

class SslTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if their bodies would
  // be empty.

  SslTest() {
    // You can do set-up work for each test here.
    certificatePassword = std::string("apachegeode");
    currentWorkingDirectory = boost::filesystem::current_path();
  }

  ~SslTest() override = default;
  // You can do clean-up work that doesn't throw exceptions here.

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:
  void SetUp() override {
    // Code here will be called immediately after the constructor (right
    // before each test).
    const auto clusterKeystore =
        (currentWorkingDirectory /
         boost::filesystem::path("ServerSslKeys/server_keystore_chained.p12"));
    const auto clusterTruststore =
        (currentWorkingDirectory /
         boost::filesystem::path(
             "ServerSslKeys/server_truststore_chained_root.jks"));

    cluster.useSsl(clusterKeystore.string(), clusterTruststore.string(),
                   certificatePassword, certificatePassword);

    cluster.start();

    cluster.getGfsh()
        .create()
        .region()
        .withName("region")
        .withType("PARTITION")
        .execute();
  }

  void TearDown() override {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Class members declared here can be used by all tests in the test suite
  // for Ssl.
  Cluster cluster = Cluster{LocatorCount{1}, ServerCount{1}};
  std::string certificatePassword;
  boost::filesystem::path currentWorkingDirectory;
};

TEST_F(SslTest, PutGetWithValidSslConfiguration) {
  const auto clientKeystore =
      (currentWorkingDirectory /
       boost::filesystem::path("ClientSslKeys/client_keystore_chained.pem"));
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "ClientSslKeys/client_truststore_chained_root.pem"));
  auto cache = CacheFactory()
                   .set("log-level", "DEBUG")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientKeystore.string())
                   .set("ssl-keystore-password", certificatePassword)
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", cluster.getLocatorPort())
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

  region->put("1", "one");

  cache.close();
}

TEST_F(SslTest, PutWithInvalidKeystorePassword) {
  const auto clientKeystore =
      (currentWorkingDirectory /
       boost::filesystem::path("ClientSslKeys/client_keystore_chained.pem"));
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "ClientSslKeys/client_truststore_chained_root.pem"));
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientKeystore.string())
                   .set("ssl-keystore-password", "bad_password")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", cluster.getLocatorPort())
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

  try {
    region->put("1", "one");
    FAIL() << "Expected apache::geode::client::NotConnectedException";
  } catch (const Exception& exception) {
    EXPECT_EQ(exception.getName(),
              "apache::geode::client::NotConnectedException");
  }

  cache.close();
}

TEST_F(SslTest, PutWithUntrustedKeystore) {
  const auto clientUntrustedKeystore =
      (currentWorkingDirectory /
       boost::filesystem::path("ClientSslKeys/client_keystore_untrusted.pem"));
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "ClientSslKeys/client_truststore_chained_root.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientUntrustedKeystore.string())
                   .set("ssl-keystore-password", "secret")
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", cluster.getLocatorPort())
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

  try {
    region->put("1", "one");
    FAIL() << "Expected apache::geode::client::NotConnectedException";
  } catch (const Exception& exception) {
    EXPECT_EQ(exception.getName(),
              "apache::geode::client::NotConnectedException");
  }

  cache.close();
}

TEST_F(SslTest, PutWithCorruptKeystore) {
  const auto clientCorruptKeystore =
      (currentWorkingDirectory /
       boost::filesystem::path("ClientSslKeys/client_keystore_corrupt.pem"));

  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "ClientSslKeys/client_truststore_chained_root.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "DEBUG")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientCorruptKeystore.string())
                   .set("ssl-keystore-password", certificatePassword)
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", cluster.getLocatorPort())
      .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

  try {
    region->put("1", "one");
    FAIL() << "Expected apache::geode::client::NotConnectedException";
  } catch (const Exception& exception) {
    EXPECT_EQ(exception.getName(),
              "apache::geode::client::NotConnectedException");
  }

  cache.close();
}

}  // namespace ssltest
