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
#include "testUtils.hpp"

/* This is to test
1) If Connections are left idle ,they timed out to min connections.
2) To validate PoolAttributes.
3) Create two pools with same name(in parallel threads).It should fail.
*/

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define LOCATOR1 s2p1
#define SERVER s2p2

using apache::geode::client::Exception;
using apache::geode::client::IllegalStateException;

bool isLocalServer = false;
bool isLocator = false;

const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
const char *poolRegNames[] = {"PoolRegion1"};
const char *poolName = "__TEST_POOL1__";
const char *poolName1 = "clientPool";

const char *serverGroup = "ServerGroup1";
std::shared_ptr<Cache> cachePtr;

class putThread : public ACE_Task_Base {
 private:
  std::shared_ptr<Region> regPtr;

 public:
  explicit putThread(const char *name) : regPtr(getHelper()->getRegion(name)) {}

  int svc(void) override {
    // TODO: No. of connection should be = minConnection

    for (int i = 0; i < 10000; i++) {
      try {
        regPtr->put(keys[i % 5], vals[i % 6]);
      } catch (const Exception &) {
        // ignore
      } catch (...) {
        // ignore
      }
      // TODO: Check no. of connection > minConnetion
    }
    // LOG(" Incremented 100 times by thread.");
    return 0;
  }
  void start() { activate(); }
  void stop() { wait(); }
};

void doAttrTestingAndCreatePool(const char *poolNameToUse) {
  auto poolFac = getHelper()->getCache()->getPoolManager().createFactory();
  poolFac.setFreeConnectionTimeout(std::chrono::milliseconds(10000));
  poolFac.setLoadConditioningInterval(std::chrono::milliseconds(60000));
  poolFac.setSocketBufferSize(1024);
  poolFac.setReadTimeout(std::chrono::milliseconds(10000));
  poolFac.setMinConnections(4);
  poolFac.setMaxConnections(8);
  poolFac.setIdleTimeout(std::chrono::seconds(5));
  poolFac.setRetryAttempts(5);
  poolFac.setPingInterval(std::chrono::milliseconds(120000));
  poolFac.setUpdateLocatorListInterval(std::chrono::milliseconds(122000));
  poolFac.setStatisticInterval(std::chrono::milliseconds(120000));
  poolFac.setServerGroup(serverGroup);
  poolFac.setSubscriptionEnabled(true);
  poolFac.setSubscriptionRedundancy(1);
  poolFac.setSubscriptionMessageTrackingTimeout(
      std::chrono::milliseconds(500000));
  poolFac.setSubscriptionAckInterval(std::chrono::milliseconds(120000));
  poolFac.addLocator("localhost", CacheHelper::staticLocatorHostPort1);
  // poolFacPtr->setMultiuserSecurityMode(true);
  poolFac.setPRSingleHopEnabled(false);

  auto pptr = poolFac.create(poolNameToUse);

  // Validate the attributes
  ASSERT(pptr->getFreeConnectionTimeout() == std::chrono::milliseconds(10000),
         "FreeConnectionTimeout Should have been 10000");
  ASSERT(
      pptr->getLoadConditioningInterval() == std::chrono::milliseconds(60000),
      "LoadConditioningInterval Should have been 60000");
  ASSERT(pptr->getSocketBufferSize() == 1024,
         "SocketBufferSize Should have been 1024");
  ASSERT(pptr->getReadTimeout() == std::chrono::milliseconds(10000),
         "ReadTimeout Should have been 10000");
  ASSERT(pptr->getMinConnections() == 4, "MinConnections Should have been 4");
  ASSERT(pptr->getMaxConnections() == 8, "MaxConnections Should have been 8");
  ASSERT(pptr->getIdleTimeout() == std::chrono::seconds(5),
         "IdleTimeout Should have been 5s");
  ASSERT(pptr->getRetryAttempts() == 5, "RetryAttempts Should have been 5");
  ASSERT(pptr->getPingInterval() == std::chrono::milliseconds(120000),
         "PingInterval Should have been 120000");
  ASSERT(
      pptr->getUpdateLocatorListInterval() == std::chrono::milliseconds(122000),
      "UpdateLocatorListInterval Should have been 122000");
  ASSERT(pptr->getStatisticInterval() == std::chrono::milliseconds(120000),
         "StatisticInterval Should have been 120000");
  ASSERT(pptr->getServerGroup() == "ServerGroup1",
         "ServerGroup Should have been ServerGroup1");
  ASSERT(pptr->getSubscriptionEnabled() == true,
         "SubscriptionEnabled Should have been true");
  ASSERT(pptr->getSubscriptionRedundancy() == 1,
         "SubscriptionRedundancy Should have been 1");
  ASSERT(pptr->getSubscriptionMessageTrackingTimeout() ==
             std::chrono::milliseconds(500000),
         "SubscriptionMessageTrackingTimeout Should have been 500000");
  ASSERT(
      pptr->getSubscriptionAckInterval() == std::chrono::milliseconds(120000),
      "SubscriptionAckInterval Should have been 120000");
  // ASSERT(pptr->getMultiuserSecurityMode()==true,"SetMultiuserSecurityMode
  // Should have been true");
  ASSERT(pptr->getPRSingleHopEnabled() == false,
         "PRSingleHopEnabled should have been false");
}

void doAttrTesting(const char *poolNameToUse) {
  // auto poolFacPtr = cachePtr->getPoolFactory();
  auto pptr = getHelper()->getCache()->getPoolManager().find(poolNameToUse);
  // auto pptr = poolFacPtr->find(poolNameToUse);

  ASSERT(pptr->getName() == "clientPool",
         "Pool name should have been clientPool");
  ASSERT(pptr->getFreeConnectionTimeout() == std::chrono::milliseconds(10000),
         "FreeConnectionTimeout Should have been 10000");
  ASSERT(pptr->getLoadConditioningInterval() == std::chrono::seconds(1),
         "LoadConditioningInterval Should have been 1");
  ASSERT(pptr->getSocketBufferSize() == 1024,
         "SocketBufferSize Should have been 1024");
  ASSERT(pptr->getReadTimeout() == std::chrono::seconds(10),
         "ReadTimeout Should have been 10");
  ASSERT(pptr->getMinConnections() == 2, "MinConnections Should have been 2");
  ASSERT(pptr->getMaxConnections() == 5, "MaxConnections Should have been 5");
  ASSERT(pptr->getIdleTimeout() == std::chrono::milliseconds(5),
         "IdleTimeout Should have been 5ms");
  ASSERT(pptr->getRetryAttempts() == 5, "RetryAttempts Should have been 5");
  ASSERT(pptr->getPingInterval() == std::chrono::seconds(1),
         "PingInterval Should have been 1");
  ASSERT(
      pptr->getUpdateLocatorListInterval() == std::chrono::milliseconds(25000),
      "UpdateLocatorListInterval Should have been 25000");
  ASSERT(pptr->getStatisticInterval() == std::chrono::seconds(1),
         "StatisticInterval Should have been 1");
  ASSERT(pptr->getServerGroup() == "ServerGroup1",
         "ServerGroup Should have been ServerGroup1");
  ASSERT(pptr->getSubscriptionEnabled() == true,
         "SubscriptionEnabled Should have been true");
  ASSERT(pptr->getSubscriptionRedundancy() == 1,
         "SubscriptionRedundancy Should have been 1");
  ASSERT(
      pptr->getSubscriptionMessageTrackingTimeout() == std::chrono::seconds(5),
      "SubscriptionMessageTrackingTimeout Should have been 5");
  ASSERT(pptr->getSubscriptionAckInterval() == std::chrono::seconds(1),
         "SubscriptionAckInterval Should have been 1");
  ASSERT(pptr->getPRSingleHopEnabled() == false,
         "PRSingleHopEnabled should have been false");
}

DUNIT_TASK(LOCATOR1, StartLocator1)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK(StartLocator1)

DUNIT_TASK(SERVER, StartS12)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver1_pool.xml", locHostPort);
    }
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver2_pool.xml", locHostPort);
    }
  }
END_TASK(StartS12)

DUNIT_TASK(CLIENT1, StartC1)
  {
    auto props = Properties::create();
    props->insert("redundancy-monitor-interval", "120s");
    props->insert("statistic-sampling-enabled", "false");
    props->insert("statistic-sample-rate", "120s");

    initClient(true, props);

    doAttrTestingAndCreatePool(poolName);

    // Do PoolCreation testing , create another pool with same name
    auto poolFac = getHelper()->getCache()->getPoolManager().createFactory();
    try {
      auto pptr = poolFac.create(poolName);
      FAIL("Pool creation with same name should fail");
    } catch (IllegalStateException &) {
      LOG("OK:Pool creation with same name should fail");
    } catch (...) {
      FAIL("Pool creation with same name should fail");
    }

    createRegionAndAttachPool(poolRegNames[0], USE_ACK, poolName);
    LOG("Clnt1Init complete.");
  }
END_TASK(StartC1)

DUNIT_TASK(CLIENT2, StartC2)
  {
    auto props = Properties::create();
    auto duplicateFile =
        CacheHelper::createDuplicateXMLFile("cacheserver_pool_client.xml");

    props->insert("cache-xml-file", duplicateFile.c_str());

    try {
      LOG(" starts client");
      initClient(true, props);
      LOG(" started client");
      ASSERT(getHelper()
                 ->getCache()
                 ->getPoolManager()
                 .find("clientPoolMultiUser")
                 ->getMultiuserAuthentication(),
             "MultiUser secure mode should be true for Pool");
    } catch (const Exception &excp) {
      LOG("Exception during client 2 XML creation");
      LOG(excp.what());
    }
    doAttrTesting(poolName1);
  }
END_TASK(StartC2)

// Test min-max connection.
DUNIT_TASK(CLIENT1, ClientOp)
  {
    // Wait for load conditioning thread to reduce pool connections to
    // min.
    SLEEP(5000);
    // Check current # connections they should be == min
    std::string poolNameString =
        getHelper()->getRegion(poolRegNames[0])->getAttributes().getPoolName();
    int level = TestUtils::getCacheImpl(getHelper()->cachePtr)
                    ->getPoolSize(poolNameString);
    int min = getHelper()
                  ->getCache()
                  ->getPoolManager()
                  .find(poolNameString.c_str())
                  ->getMinConnections();
    ASSERT(level == min,
           std::string("Pool level not equal to min level. Expected ") +
               std::to_string(min) + ", actual " + std::to_string(level));

    putThread *threads[25];
    for (int thdIdx = 0; thdIdx < 10; thdIdx++) {
      threads[thdIdx] = new putThread(poolRegNames[0]);
      threads[thdIdx]->start();
    }

    SLEEP(5000);  // wait for threads to become active

    // Check current # connections they should be == max
    level = TestUtils::getCacheImpl(getHelper()->cachePtr)
                ->getPoolSize(poolNameString);
    int max = getHelper()
                  ->getCache()
                  ->getPoolManager()
                  .find(poolNameString.c_str())
                  ->getMaxConnections();
    ASSERT(level == max,
           std::string("Pool level not equal to max level. Expected ") +
               std::to_string(max) + ", actual " + std::to_string(level));

    for (int thdIdx = 0; thdIdx < 10; thdIdx++) {
      threads[thdIdx]->stop();
    }

    // Milli second sleep: IdleTimeout is 5 sec, load conditioning
    // interval is 1 min
    LOG("Waiting 25 sec for idle timeout to kick in");
    SLEEP(25000);

    level = TestUtils::getCacheImpl(getHelper()->cachePtr)
                ->getPoolSize(poolNameString);
    min = getHelper()
              ->getCache()
              ->getPoolManager()
              .find(poolNameString.c_str())
              ->getMinConnections();
    ASSERT(
        level == min,
        std::string(
            "Pool level not equal to min level after idle timeout. Expected ") +
            std::to_string(min) + ", actual " + std::to_string(level));

    LOG("Waiting 1 minute for load conditioning to kick in");
    SLEEP(60000);

    level = TestUtils::getCacheImpl(getHelper()->cachePtr)
                ->getPoolSize(poolNameString);
    ASSERT(level == min, std::string("Pool level not equal to min level after "
                                     "load conditioning. Expected ") +
                             std::to_string(min) + ", actual " +
                             std::to_string(level));
  }
END_TASK(ClientOp)

DUNIT_TASK(CLIENT1, StopC1)
  {
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK(StopC1)

DUNIT_TASK(CLIENT2, StopC2)
  {
    cleanProc();
    LOG("Clnt1Down complete: Keepalive = True");
  }
END_TASK(StopC2)

DUNIT_TASK(SERVER, CloseServers)
  {
    // stop servers
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK(CloseServers)

DUNIT_TASK(LOCATOR1, CloseLocator1)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK(CloseLocator1)
