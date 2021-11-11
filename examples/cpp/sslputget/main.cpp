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

void print_usage() {
  std::cout << "Usage: cpp-sslputget <<path>>\n";
  std::cout << "Where <<path>> is the absolute path to the location of your "
               "SSL client keystore and truststore\n";
}

int main(int argc, char** argv) {
  if (argc < 2) {
    print_usage();
    exit(-1);
  }

  auto sslKeyPath = std::string(argv[1]);

  auto cache = CacheFactory()
                   .set("log-level", "none")
                   .set("ssl-enabled", "true")
                   .set("ssl-keystore",
#ifdef WIN32
                        (sslKeyPath + "\\client_keystore.pem").c_str())
#else
                        (sslKeyPath + "/client_keystore.pem").c_str())
#endif
                   .set("ssl-keystore-password", "apachegeode")
                   .set("ssl-truststore",
#ifdef WIN32
                        (sslKeyPath + "\\client_truststore.pem").c_str())
#else
                        (sslKeyPath + "/client_truststore.pem").c_str())
#endif
                   .create();

  const auto pool = cache.getPoolManager()
                        .createFactory()
                        .addLocator("localhost", 10334)
                        .create("pool");

  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
                    .setPoolName("pool")
                    .create("testSSLRegion");

  std::string rtimmonsKey = "rtimmons";
  std::string rtimmonsValue = "Robert Timmons";
  std::string scharlesKey = "scharles";
  std::string scharlesValue = "Sylvia Charles";

  std::cout << "Storing id and username in the region" << std::endl;

  region->put(rtimmonsKey, rtimmonsValue);
  region->put(scharlesKey, scharlesValue);

  std::cout << "Getting the user info from the region" << std::endl;

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
