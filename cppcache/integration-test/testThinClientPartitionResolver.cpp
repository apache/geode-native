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

#define ROOT_NAME "testThinClientPartitionResolver"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"
#include "Utils.hpp"
#include <geode/PartitionResolver.hpp>

#include <string>

#include "CacheHelper.hpp"

// Include these 2 headers for access to CacheImpl for test hooks.
#include "CacheImplHelper.hpp"
#include "testUtils.hpp"

#include "ThinClientHelper.hpp"

using apache::geode::client::EntryEvent;
using apache::geode::client::PartitionResolver;

class CustomPartitionResolver : public PartitionResolver {
 public:
  bool called;

  CustomPartitionResolver() : called(false) {}
  ~CustomPartitionResolver() override {}
  const std::string &getName() override {
    static std::string name = "CustomPartitionResolver";
    LOG("CustomPartitionResolver::getName()");
    return name;
  }

  std::shared_ptr<CacheableKey> getRoutingObject(
      const EntryEvent &opDetails) override {
    called = true;
    LOG("CustomPartitionResolver::getRoutingObject()");
    int32_t key = atoi(opDetails.getKey()->toString().c_str());
    int32_t newKey = key + 5;
    return CacheableKey::create(newKey);
  }
};
CustomPartitionResolver *cpr = new CustomPartitionResolver();
std::shared_ptr<PartitionResolver> cptr(cpr);

#define CLIENT1 s1p1
#define SERVER1 s2p1
#define SERVER2 s1p2

bool isLocalServer = false;
const std::string endPoints = CacheHelper::getTcrEndpoints(isLocalServer, 3);

std::vector<std::string> storeEndPoints(const std::string points) {
  std::vector<std::string> endpointNames;
  size_t end = 0;
  size_t start;
  std::string delim = ",";
  while ((start = points.find_first_not_of(delim, end)) != std::string::npos) {
    end = points.find(delim, start);
    if (end == std::string::npos) {
      end = points.length();
    }
    endpointNames.push_back(points.substr(start, end - start));
  }
  ASSERT(endpointNames.size() == 3, "There should be 3 end points");
  return endpointNames;
}

std::vector<std::string> endpointNames = storeEndPoints(endPoints);

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1, "cacheserver1_pr.xml");
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) CacheHelper::initServer(2, "cacheserver2_pr.xml");
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreatePoolAndRegions)
  {
    initClient(true);

    getHelper()->createPoolWithLocators("__TEST_POOL1__", nullptr);
    getHelper()->createRegionAndAttachPool2(regionNames[0], USE_ACK,
                                            "__TEST_POOL1__", cptr);
    // getHelper()->createRegionAndAttachPool2(regionNames[1], NO_ACK,
    // "__TEST_POOL1__",cptr);

    LOG("CreatePoolAndRegions complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutThroughPartitionResolver)
  {
    LOG("PutThroughPartitionResolver started.");

    for (int i = 0; i < 100; i++) {
      // auto dataReg = getHelper()->getRegion("LocalRegion");
      auto dataReg = getHelper()->getRegion(regionNames[0]);
      auto keyPtr =
          std::dynamic_pointer_cast<CacheableKey>(CacheableInt32::create(i));
      dataReg->put(keyPtr, keyPtr->hashcode());
    }
    SLEEP(5000);
    ASSERT(cpr->called, "Partition resolver not called");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CacheableHelper::registerBuiltins(true);

    // Need multiple servers to test PartitionResolver
    CALL_TASK(CreateServer1);
    CALL_TASK(CreateServer2);

    CALL_TASK(CreatePoolAndRegions);

    CALL_TASK(PutThroughPartitionResolver);

    CALL_TASK(CloseCache1);

    CALL_TASK(CloseServer1);
    CALL_TASK(CloseServer2);
  }
END_MAIN
