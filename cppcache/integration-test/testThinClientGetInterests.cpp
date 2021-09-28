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

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#define CLIENT1 s1p1
#define SERVER1 s2p1

#include "locator_globals.hpp"

const char *durableIds[] = {"DurableId1", "DurableId2"};

DUNIT_TASK(SERVER1, StartServer)
  {
    if (isLocalServer) {
      CacheHelper::initLocator(1);
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                              locatorsG);
    }
    LOG("SERVER started");
  }
END_TASK(StartServer)

DUNIT_TASK(CLIENT1, SetupClient1)
  {
    auto pp = Properties::create();
    pp->insert("durable-client-id", durableIds[0]);
    pp->insert("durable-timeout", std::chrono::seconds(300));
    pp->insert("notify-ack-interval", std::chrono::seconds(1));

    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, pp, 0, true);
    getHelper()->createPooledRegion(regionNames[0], false, locatorsG,
                                    "__TEST_POOL1__", true, true);
    auto keyPtr0 = CacheableString::create(keys[0]);
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    std::vector<std::shared_ptr<CacheableKey>> keys0;
    keys0.push_back(keyPtr0);
    regPtr0->registerKeys(keys0);
    auto keyPtr1 = CacheableString::create(keys[1]);
    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(keyPtr1);
    regPtr0->registerKeys(keys1);
    regPtr0->registerRegex(testregex[0]);
    regPtr0->registerRegex(testregex[1]);
    auto keyPtr2 = CacheableString::create(keys[2]);
    std::vector<std::shared_ptr<CacheableKey>> keys2;
    keys2.push_back(keyPtr2);
    keyPtr2 = CacheableString::create(keys[3]);
    keys2.push_back(keyPtr2);
    // durable
    regPtr0->registerKeys(keys2, false, true);
    regPtr0->registerRegex(testregex[2], true);

    auto vkey = regPtr0->getInterestList();
    auto vreg = regPtr0->getInterestListRegex();
    for (size_t i = 0; i < vkey.size(); i++) {
      const char *key =
          std::dynamic_pointer_cast<CacheableString>(vkey[i])->value().c_str();
      LOG(std::string("key[") + std::to_string(i) + "] = " + key);
      bool found = false;
      for (const auto &k : vkey) {
        if (!strcmp(key, k->toString().c_str())) {
          found = true;
          break;
        }
      }
      LOG(std::string("key[") + std::to_string(i) + "]=" + key + " not found!");
    }
    for (size_t i = 0; i < vreg.size(); i++) {
      auto ptr = vreg[i];
      const char *reg = ptr->value().c_str();
      LOG(std::string("regex[") + std::to_string(i) + "]=" + reg);
      bool found = false;
      for (size_t j = 0; j < vreg.size(); j++) {
        if (!strcmp(reg, testregex[j])) {
          found = true;
          break;
        }
      }
      ASSERT(found, std::string("regex[") + std::to_string(i) + "]=" + reg +
                        " not found!");
    }
    regPtr0->registerAllKeys(true);
    auto vreg1 = regPtr0->getInterestListRegex();
    for (size_t i = 0; i < vreg1.size(); i++) {
      auto ptr = vreg1[i];
      LOG(std::string("regex[") + std::to_string(i) + "]=" + ptr->value());
    }
  }
END_TASK(SetupClient1)

DUNIT_TASK(SERVER1, StopServer)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      CacheHelper::closeLocator(1);
    }
    LOG("SERVER stopped");
  }
END_TASK(StopServer)
DUNIT_TASK(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK(CloseCache1)
