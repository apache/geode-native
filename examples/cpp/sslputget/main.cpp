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
#ifdef _MSC_VER
#include <direct.h>
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#undef max
#endif
#else
#include <unistd.h>
#endif
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

std::string myGetcwd() {
  char buf[PATH_MAX];
  std::string cwd;

#ifdef _MSC_VER
  if (_getcwd(buf, PATH_MAX)) {
    cwd = buf;
  }
#else
  if (getcwd(buf, PATH_MAX)) {
    cwd = buf;
  }
#endif
  return cwd;
}

int main(int argc, char** argv) {
  auto workingDirectory = myGetcwd();

#ifdef _MSC_VER
  workingDirectory += "/..";
#endif

  auto cache =
      CacheFactory()
          .set("log-level", "none")
          .set("ssl-enabled", "true")
          .set("ssl-keystore",
               workingDirectory +
                   "/ClientSslKeys/client_keystore.password.pem")
          .set("ssl-keystore-password", "gemstone")
          .set("ssl-truststore",
               workingDirectory + "/ClientSslKeys/client_truststore.pem")
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
