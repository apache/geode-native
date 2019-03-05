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

#include "exampleAuthInitialize.hpp"

using apache::geode::client::CacheFactory;
using apache::geode::client::RegionShortcut;
using apache::geode::client::CacheableString;

int main(int argc, char** argv) {
  auto cache = CacheFactory()
      .set("log-level", "none")
      .setAuthInitialize(std::unique_ptr<ExampleAuthInitialize>(new ExampleAuthInitialize()))
      .create();
  auto pool = cache.getPoolManager()
      .createFactory()
      .addLocator("localhost", 10334)
      .create("pool");
  auto region = cache.createRegionFactory(RegionShortcut::PROXY)
      .setPoolName("pool")
      .create("region");

  region->put("a", "1");
  region->put("b", "2");

  auto a = region->get("a");
  auto b = region->get("b");

  std::cout << "a = "
            << std::dynamic_pointer_cast<CacheableString>(a)->value()
            << std::endl;
  std::cout << "b = "
            << std::dynamic_pointer_cast<CacheableString>(b)->value()
            << std::endl;

  cache.close();
}
