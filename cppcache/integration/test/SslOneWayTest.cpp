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

class SslOneWayTest : public ::testing::Test {
 protected:
  SslOneWayTest() {
    certificatePassword = std::string("apachegeode");
    serverSslKeysDir = boost::filesystem::path(
        getFrameworkString(FrameworkVariable::TestServerSslKeysDir));
    clientSslKeysDir = boost::filesystem::path(
        getFrameworkString(FrameworkVariable::TestClientSslKeysDir));
  }

  ~SslOneWayTest() override = default;

  void SetUp() override {
    const auto clusterKeystore =
        (serverSslKeysDir /
         boost::filesystem::path("server_keystore_chained.p12"));
    const auto clusterTruststore =
        (serverSslKeysDir /
         boost::filesystem::path("server_truststore_chained_root.jks"));

    cluster.useSsl(false, clusterKeystore.string(), clusterTruststore.string(),
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

TEST_F(SslOneWayTest, PutGetWithValidSslConfiguration) {
  const auto clientTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_chained_root.pem"));
  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
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

TEST_F(SslOneWayTest, PutWithUntrustedTruststore) {
  const auto clientUntrustedTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_untrusting.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
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

TEST_F(SslOneWayTest, PutWithCorruptTruststore) {
  const auto clientUntrustedTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_corrupt.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
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

TEST_F(SslOneWayTest, PutWithMissingTruststore) {
  const auto clientMissingTruststore =
      (clientSslKeysDir /
       boost::filesystem::path("client_truststore_missing.pem"));

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
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
