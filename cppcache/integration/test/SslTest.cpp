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
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

#include "framework/Cluster.h"

namespace {

using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::RegionShortcut;

TEST(SslTest, PutGet) {
  const auto currentWorkingDirectory = boost::filesystem::current_path();
  const auto clusterKeystore =
      (currentWorkingDirectory /
       boost::filesystem::path("ServerSslKeys/server_keystore_chained.p12"));
  const auto clusterTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "ServerSslKeys/server_truststore_chained_root.jks"));
  const auto certificatePassword = std::string("apachegeode");
  const auto clientKeystore =
      (currentWorkingDirectory /
       boost::filesystem::path("ClientSslKeys/client_keystore_chained.pem"));
  const auto clientTruststore =
      (currentWorkingDirectory /
       boost::filesystem::path(
           "ClientSslKeys/client_truststore_chained_root.pem"));

  Cluster cluster{LocatorCount{1}, ServerCount{1}};
  cluster.useSsl(clusterKeystore.string(), clusterTruststore.string(),
                 certificatePassword, certificatePassword);

  cluster.start();

  cluster.getGfsh()
      .create()
      .region()
      .withName("region")
      .withType("PARTITION")
      .execute();

  std::cout << "started cluster and created region" << std::endl;

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore", clientKeystore.string())
                   .set("ssl-keystore-password", certificatePassword)
                   .set("ssl-truststore", clientTruststore.string())
                   .create();

  const auto pool = cache.getPoolManager()
                        .createFactory()
                        .addLocator("localhost", cluster.getLocatorPort())
                        .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("region");

  region->put("1", "one");
  region->put("2", "two");

  cache.close();
}

}  // namespace
