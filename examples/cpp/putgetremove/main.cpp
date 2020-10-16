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

#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/RegionFactory.hpp>
#include <geode/RegionShortcut.hpp>

using namespace apache::geode::client;

int main(int argc, char** argv) {
  auto cache = CacheFactory()
      .set("log-level", "none")
      .create();

  cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");
  
  auto regionFactory = cache.createRegionFactory(RegionShortcut::PROXY);
  auto region = regionFactory.setPoolName("pool").create("example_userinfo");

  std::cout << "Storing id and username in the region" << std::endl;
  region->put("rtimmons", "Robert Timmons");
  region->put("scharles", "Sylvia Charles");

  std::cout << "Getting the user info from the region" << std::endl;
  auto user1 = region->get("rtimmons");
  auto user2 = region->get("scharles");
  std::cout << "  rtimmons = "
            << std::dynamic_pointer_cast<CacheableString>(user1)->value()
            << std::endl;
  std::cout << "  scharles = "
            << std::dynamic_pointer_cast<CacheableString>(user2)->value()
            << std::endl;

  std::cout << "Removing rtimmons info from the region" << std::endl;
  region->remove("rtimmons");

  if (region->existsValue("rtimmons")) {
    std::cout << "rtimmons's info not deleted" << std::endl;
  } else {
    std::cout << "rtimmons's info successfully deleted" << std::endl;
  }

  cache.close();
}
