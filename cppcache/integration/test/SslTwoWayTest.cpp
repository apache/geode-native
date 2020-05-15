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

#include "framework/Cluster.h"

namespace ssltest {

using apache::geode::client::AuthenticationRequiredException;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::Exception;
using apache::geode::client::RegionShortcut;

class SslTwoWayTest : public ::testing::Test {
 protected:
  SslTwoWayTest() {
    certificatePassword = std::string("apachegeode");
    serverSslKeysDir = boost::filesystem::path(
        getFrameworkString(FrameworkVariable::TestServerSslKeysDir));
    clientSslKeysDir = boost::filesystem::path(
        getFrameworkString(FrameworkVariable::TestClientSslKeysDir));
  }

  ~SslTwoWayTest() override = default;

  void SetUp() override {
    const auto clusterKeystore =
        (serverSslKeysDir /
         boost::filesystem::path("server_keystore_chained.p12"));
    const auto clusterTruststore =
        (serverSslKeysDir /
         boost::filesystem::path("server_truststore_chained_root.jks"));

    cluster.useSsl(true, clusterKeystore.string(), clusterTruststore.string(),
                   certificatePassword, certificatePassword);

    cluster.start();

    cluster.getGfsh()
        .create()
        .region()
        .withName("region")
        .withType("PARTITION")
        .execute();
  }

  void TearDown() override {}

  Cluster cluster = Cluster{LocatorCount{1}, ServerCount{1}};
  std::string certificatePassword;
  boost::filesystem::path serverSslKeysDir;
  boost::filesystem::path clientSslKeysDir;
};

TEST_F(SslTwoWayTest, PutGetWithValidSslConfiguration) {
  const auto clientKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_chained.pem"));
  const auto clientTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_chained_root.pem"));
  auto cache = CacheFactory()
                   .set("log-level", "debug")
                   .set("log-file", "./gemfire.log")
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

  try {
    region->put("1", "one");
  } catch (Exception& ex) {
    std::cout << ex.getStackTrace();
  }
  std::shared_ptr<apache::geode::client::Cacheable> value;

  try {
    value = region->get("1");
  } catch (Exception& ex) {
    std::cout << ex.getStackTrace();
  }

  EXPECT_TRUE(value);

  auto string_value =
      std::dynamic_pointer_cast<apache::geode::client::CacheableString>(value);

  EXPECT_TRUE(string_value);

  EXPECT_EQ(string_value->value(), "one");

  std::cout << "Read " << string_value->value() << " from the server.";
  cache.close();
}

TEST_F(SslTwoWayTest, PutWithInvalidKeystorePassword) {
  const auto clientKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_chained.pem"));
  const auto clientTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_chained_root.pem"));
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

TEST_F(SslTwoWayTest, PutWithUntrustedKeystore) {
  const auto clientUntrustedKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_untrusted.pem"));
  const auto clientTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_chained_root.pem"));

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

TEST_F(SslTwoWayTest, PutWithCorruptKeystore) {
  const auto clientCorruptKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_corrupt.pem"));

  const auto clientTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_chained_root.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
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

TEST_F(SslTwoWayTest, PutWithUntrustedTruststore) {
  const auto clientKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_chained.pem"));
  const auto clientUntrustedTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_untrusting.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientKeystore.string())
                   .set("ssl-keystore-password", certificatePassword)
                   .set("ssl-truststore", clientUntrustedTruststore.string())
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

TEST_F(SslTwoWayTest, PutWithCorruptTruststore) {
  const auto clientKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_chained.pem"));
  const auto clientUntrustedTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_corrupt.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientKeystore.string())
                   .set("ssl-keystore-password", certificatePassword)
                   .set("ssl-truststore", clientUntrustedTruststore.string())
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

TEST_F(SslTwoWayTest, PutWithMissingTruststore) {
  const auto clientKeystore =
      (clientSslKeysDir /
       boost::filesystem::path("client_keystore_chained.pem"));
  const auto clientMissingTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_missing.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientKeystore.string())
                   .set("ssl-keystore-password", certificatePassword)
                   .set("ssl-truststore", clientMissingTruststore.string())
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
