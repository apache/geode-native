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

/*
 * This example takes the following steps:
 *
 * 1. Create a Geode Cache, Pool, and example Region Programmatically.
 * 3. Populate some objects on the Region.
 * 4. Create Execute Object
 * 5. Execute Function
 */
#include <iostream>
#include <memory>
#include <string>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using apache::geode::client::CacheableString;
using apache::geode::client::CacheFactory;
using apache::geode::client::RegionShortcut;

int main(int argc, char** argv) {
  const auto argv_str = std::string(argv[0]);
  const auto workingDirectory = argv_str.substr(0, argv_str.find_last_of("/"));

  auto cache =
      CacheFactory()
          .set("log-level", "all")
          .set("log-file", "c:/temp/example.log")
          .set("ssl-enabled", "true")
          .set("ssl-keystore",
               workingDirectory +
                   "/../ClientSslKeys/client_keystore.password.pem")
          .set("ssl-keystore-password", "gemstone")
          .set("ssl-truststore",
               workingDirectory + "/../ClientSslKeys/client_truststore.pem")
          .create();

  const auto pool = cache.getPoolManager()
                        .createFactory()
                        .addServer("localhost", 10334)
                        .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("testSSLRegion");

  std::string rtimmonsKey = "rtimmons";
  std::string rtimmonsValue = "Robert Timmons";
  std::string scharlesKey = "scharles";
  std::string scharlesValue = "Sylvia Charles";

  std::cout << "Storing id and username in the region\n";

  region->put(rtimmonsKey, rtimmonsValue);
  region->put(scharlesKey, scharlesValue);

  std::cout << "Getting the user info from the region\n";

  const auto user1 = region->get(rtimmonsKey);
  const auto user2 = region->get(scharlesKey);

  std::cout << rtimmonsKey << " = "
            << std::dynamic_pointer_cast<CacheableString>(user1)->value()
            << '\n';
  std::cout << scharlesKey << " = "
            << std::dynamic_pointer_cast<CacheableString>(user2)->value()
            << '\n';

  cache.close();
}
