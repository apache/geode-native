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

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTVERSIONEDOPS_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTVERSIONEDOPS_H_

#define ROOT_NAME "testThinClientVersionedOps"

#include "ThinClientSecurityHelper.hpp"
#include <geode/CacheTransactionManager.hpp>

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

// This is the test for tracking work. bug#304

putThread *thread1 = nullptr;
putThread *thread2 = nullptr;
putThread *thread3 = nullptr;
putThread *thread4 = nullptr;

const char *regNames[] = {"DistRegionAck", "DistRegionNoAck"};
const char *group1 = "A";
const char *group2 = "B";
// const char * endPoints1 = CacheHelper::getTcrEndpoints( isLocalServer1, 3 );
std::string gfendpoints1 = "localhost:";
std::string gfendpoints2 = "localhost:";

bool isLocalServer1 = false;

static bool isLocator1 = false;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator1, isLocalServer1, 1);
std::shared_ptr<CacheableString> c1v11;
std::shared_ptr<CacheableString> c1v12;
std::shared_ptr<CacheableString> c1v13;
std::shared_ptr<CacheableString> c1v14;
std::shared_ptr<CacheableString> c1v15;
std::shared_ptr<CacheableString> s1v11;
std::shared_ptr<CacheableString> s1v12;
std::shared_ptr<CacheableString> s1v13;
std::shared_ptr<CacheableString> s1v14;
std::shared_ptr<CacheableString> s1v15;
std::shared_ptr<CacheableString> c2v11;
std::shared_ptr<CacheableString> c2v12;
std::shared_ptr<CacheableString> c2v13;
std::shared_ptr<CacheableString> c2v14;
std::shared_ptr<CacheableString> c2v15;
std::shared_ptr<CacheableString> s2v11;
std::shared_ptr<CacheableString> s2v12;
std::shared_ptr<CacheableString> s2v13;
std::shared_ptr<CacheableString> s2v14;
std::shared_ptr<CacheableString> s2v15;

void verifyAllValues() {
  LOG_INFO("verifyAllValues TEST-1");
  /*LOG_INFO("c1v11 = %s", c1v11->value().c_str());
  LOG_INFO("verifyAllValues TEST-2");
  LOG_INFO("c1v12 = %s", c1v12->value().c_str());
  LOG_INFO("c1v13 = %s", c1v13->value().c_str());
  LOG_INFO("c1v14 = %s", c1v14->value().c_str());
  LOG_INFO("c1v15 = %s", c1v15->value().c_str());

  LOG_INFO("s1v11 = %s", s1v11->value().c_str());
  LOG_INFO("s1v12 = %s", s1v12->value().c_str());
  LOG_INFO("s1v13 = %s", s1v13->value().c_str());
  LOG_INFO("s1v14 = %s", s1v14->value().c_str());
  LOG_INFO("s1v15 = %s", s1v15->value().c_str());

  LOG_INFO("c2v11 = %s", c2v11->value().c_str());
  LOG_INFO("c2v12 = %s", c2v12->value().c_str());
  LOG_INFO("c2v13 = %s", c2v13->value().c_str());
  LOG_INFO("c2v14 = %s", c2v14->value().c_str());
  LOG_INFO("c2v15 = %s", c2v15->value().c_str());

  LOG_INFO("s2v11 = %s", s2v11->value().c_str());
  LOG_INFO("s2v12 = %s", s2v12->value().c_str());
  LOG_INFO("s2v13 = %s", s2v13->value().c_str());
  LOG_INFO("s2v14 = %s", s2v14->value().c_str());
  LOG_INFO("s2v15 = %s", s2v15->value().c_str());*/
}

// added
DUNIT_TASK_DEFINITION(SERVER1, StartServer1)
  {
    LOG("starting SERVER1...");
    if (isLocalServer1) {
      CacheHelper::initServer(1, "cacheserver_concurrency_enabled1.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

// added
DUNIT_TASK_DEFINITION(SERVER2, StartServer2)
  {
    LOG("starting SERVER2...");
    if (isLocalServer1) {
      CacheHelper::initServer(2, "cacheserver_concurrency_enabled2.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StartClient1)
  {
    // TOOD::check for poolwithendpoints or locators

    // By default poolwithendpoints
    // char tmp1[100];
    // sprintf( tmp1, "%d", CacheHelper::staticHostPort1 );
    // gfendpoints1 += tmp1;

    // initClientWithPool(true/*isthinClient*/, nullptr/*poolName*/,
    // nullptr/*locators*/,serverGroup1, nullptr/*servers*/,
    // nullptr/*PropertiesPtr&*/,
    // 0/*redundancy*/, true/*clientNotification*/,
    // -1/*subscriptionAckInterval*/,
    // 5/*connections*/, 60000/*loadConditioningInterval*/);
    // auto regPtr0 = createRegionAndAttachPool(regNames[0],USE_ACK,
    // nullptr);

    initClient(true);
    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG, true, -1,
                                        std::chrono::milliseconds::zero(), -1,
                                        false, group1);
    auto regPtr0 = getHelper()->createRegionAndAttachPool(
        regNames[0], USE_ACK, "__TEST_POOL1__", true);
    LOG("StepOne_Pooled_Locator1 complete.");

    regPtr0->registerAllKeys();

    LOG("Client-1 Init complete.");
  }
END_TASK_DEFINITION

// startClient-2
DUNIT_TASK_DEFINITION(CLIENT2, StartClient2)
  {
    // TOOD::check for poolwithendpoints or locators
    /*	LOG( "Client-2 Init -1" );
    //By default poolwithendpoints
          char tmp2[100];
          LOG( "Client-2 Init -2" );
          sprintf( tmp2, "%d", CacheHelper::staticHostPort2 );
          LOG( "Client-2 Init -3" );
          gfendpoints2 += tmp2;
          LOG( "Client-2 Init -4" );
    initClientWithPool(true, nullptr, nullptr, serverGroup2,
    gfendpoints2.c_str(), nullptr, 0, true, -1, 5, 60000); LOG( "Client-2 Init
    -5" );

   auto regPtr0 = createRegionAndAttachPool(regNames[0],USE_ACK, nullptr);
    LOG( "Client-2 Init -6" );

    */
    initClient(true);
    getHelper()->createPoolWithLocators("__TEST_POOL1__", locatorsG, true, -1,
                                        std::chrono::milliseconds::zero(), -1,
                                        false, group2);
    auto regPtr0 = getHelper()->createRegionAndAttachPool(
        regNames[0], USE_ACK, "__TEST_POOL1__", true);
    LOG("StepOne_Pooled_Locator1 complete.");

    regPtr0->registerAllKeys();
    LOG("Client-2 StartClient2 complete.");
  }
END_TASK_DEFINITION

// threadPutonClient1
DUNIT_TASK_DEFINITION(CLIENT1, threadPutonClient1)
  {
    auto rptr = getHelper()->getRegion(regNames[0]);
    thread4 = new putThread(rptr, false);
    thread4->setParams(0, 10, 1, true, false, 0);
    thread4->start();
    LOG("Task: threadPutonClient1 Done");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, transactionPutOnClient2)
  {
    auto rptr = getHelper()->getRegion(regNames[0]);
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();

    txManager->begin();
    auto keyPtr1 = CacheableKey::create("key-1");
    auto valPtr = CacheableString::create("client2-value1");
    rptr->put(keyPtr1, valPtr);

    auto keyPtr2 = CacheableKey::create("key-2");
    valPtr = CacheableString::create("client2-value2");
    rptr->put(keyPtr2, valPtr);

    auto keyPtr3 = CacheableKey::create("key-3");
    valPtr = CacheableString::create("client2-value3");
    rptr->put(keyPtr3, valPtr);
    txManager->commit();

    dunit::sleep(1000);

    // localGet

    c2v11 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr1));
    c2v12 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr2));
    c2v13 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr3));

    // localDestroy
    rptr->localDestroy(keyPtr1);
    rptr->localDestroy(keyPtr2);
    rptr->localDestroy(keyPtr3);

    // remoteGet
    c2v11 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr1));
    s2v12 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr2));
    s2v13 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr3));

    // Print remoteGet Values
    LOG_INFO(
        "CLIENT-2 :: TASK:transactionPutOnClient2: localGet Val1 = %s "
        "remoteGet "
        "Val1 = %s ",
        c2v11->value().c_str(), s2v11->value().c_str());
    LOG_INFO(
        "CLIENT-2 :: TASK:transactionPutOnClient2: localGet Val2 = %s "
        "remoteGet "
        "Val2 = %s ",
        c2v12->value().c_str(), s2v12->value().c_str());
    LOG_INFO(
        "CLIENT-2 :: TASK:transactionPutOnClient2: localGet Val3 = %s "
        "remoteGet "
        "Val3 = %s ",
        c2v13->value().c_str(), s2v13->value().c_str());

    ASSERT(*c2v11 == *c2v11,
           "transactionPutOnClient2:Values should be equal-1");
    ASSERT(*c2v12 == *c2v12,
           "transactionPutOnClient2:Values should be equal-2");
    ASSERT(*c2v13 == *c2v13,
           "transactionPutOnClient2:Values should be equal-3");

    LOG("CLIENT-2 :: TASK: transactionPutOnClient2 completed successfully");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, verifyGetonClient1)
  {
    thread4->stop();
    delete thread4;

    auto rptr = getHelper()->getRegion(regNames[0]);

    auto keyPtr1 = CacheableKey::create("key-1");
    auto keyPtr2 = CacheableKey::create("key-2");
    auto keyPtr3 = CacheableKey::create("key-3");

    // localGet
    c1v11 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr1));
    c1v12 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr2));
    c1v13 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr3));

    // localDestroy
    rptr->localDestroy(keyPtr1);
    rptr->localDestroy(keyPtr2);
    rptr->localDestroy(keyPtr3);

    // remoteGet
    s1v11 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr1));
    s1v12 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr2));
    s1v13 = std::dynamic_pointer_cast<CacheableString>(rptr->get(keyPtr3));

    // Print remoteGet Values
    LOG_INFO(
        "CLIENT-2 :: verifyGetonClient1: localGet Val1 = %s remoteGet Val1 = "
        "%s ",
        c1v11->value().c_str(), s1v11->value().c_str());
    LOG_INFO(
        "CLIENT-2 :: verifyGetonClient1: localGet Val2 = %s remoteGet Val2 = "
        "%s ",
        c1v12->value().c_str(), s1v12->value().c_str());
    LOG_INFO(
        "CLIENT-2 :: verifyGetonClient1: localGet Val3 = %s remoteGet Val3 = "
        "%s ",
        c1v13->value().c_str(), s1v13->value().c_str());

    ASSERT(*c1v11 == *s1v11, "verifyGetonClient1:Values should be equal-1");
    ASSERT(*c1v12 == *s1v12, "verifyGetonClient1:Values should be equal-2");
    ASSERT(*c1v13 == *s1v13, "verifyGetonClient1:Values should be equal-3");

    LOG("CLIENT-2 :: TASK: verifyGetonClient1 completed successfully");
  }
END_TASK_DEFINITION

//
DUNIT_TASK_DEFINITION(CLIENT1, PutOnClient1)
  {
    auto rptr = getHelper()->getRegion(regNames[0]);
    thread1 = new putThread(rptr, false);
    thread1->setParams(0, 5, 1, true, false, 1);
    thread1->start();
    LOG("PUT ops done on client 1");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, PutOnClient2)
  {
    auto rptr = getHelper()->getRegion(regNames[0]);
    thread2 = new putThread(rptr, false);
    thread2->setParams(0, 5, 1, false, false, 0);  // 0, 5, 1, false, false, 0
    thread2->start();
    LOG("PUT ops done on client 2");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testServerGC)
  {
    auto rptr = getHelper()->getRegion(regNames[0]);
    thread3 = new putThread(rptr, false);
    thread3->setParams(0, 5000, 1, true, false, 0);  // 0, 5, 1, false, false, 0
    thread3->start();
    LOG("5000 PUT ops done on client 1");

    // set thread to destroy entries.
    thread3->setParams(7, 5000, 1, true, false, 0);  // Destroy
    thread3->start();
    LOG("5000 entries destroyed successfully");

    dunit::sleep(10000);

    thread3->stop();
    delete thread3;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, GetOnClient1)
  {
    LOG("CLIENT-1 :: local GET operation -1");
    thread1->stop();
    delete thread1;
    dunit::sleep(3000);
    auto regPtr = getHelper()->getRegion(regNames[0]);
    // localGet
    auto keyPtr1 = CacheableKey::create("key-1");
    auto keyPtr2 = CacheableKey::create("key-2");
    auto keyPtr3 = CacheableKey::create("key-3");
    auto keyPtr4 = CacheableKey::create("key-4");
    auto keyPtr5 = CacheableKey::create("key-5");

    c1v11 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr1));
    c1v12 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr2));
    c1v13 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr3));
    c1v14 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr4));
    c1v15 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr5));

    // Print local Get Values
    LOG_INFO("CLIENT-1 :: local GET operation -6 c1v11 = %s",
            c1v11->value().c_str());
    LOG_INFO("CLIENT-1 :: local GET operation -7 c1v12 = %s",
            c1v12->value().c_str());
    LOG_INFO("CLIENT-1 :: local GET operation -8 c1v13 = %s",
            c1v13->value().c_str());
    LOG_INFO("CLIENT-1 :: local GET operation -9 c1v14=%s",
            c1v14->value().c_str());
    LOG_INFO("CLIENT-1 :: local GET operation....Done c1v15=%s",
            c1v15->value().c_str());

    // localDestroy
    regPtr->localDestroy(keyPtr1);
    regPtr->localDestroy(keyPtr2);
    regPtr->localDestroy(keyPtr3);
    regPtr->localDestroy(keyPtr4);
    regPtr->localDestroy(keyPtr5);

    LOG("CLIENT-1 :: localDestroy() operation....Done");

    // remoteGet
    s1v11 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr1));
    s1v12 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr2));
    s1v13 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr3));
    s1v14 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr4));
    s1v15 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr5));

    // Print remoteGet Values
    LOG_INFO("CLIENT-1 :: remoteGet operation -6 s1v11 = %s",
            s1v11->value().c_str());
    LOG_INFO("CLIENT-1 :: remoteGet operation -6 s1v12 = %s",
            s1v12->value().c_str());
    LOG_INFO("CLIENT-1 :: remoteGet operation -6 s1v13 = %s",
            s1v13->value().c_str());
    LOG_INFO("CLIENT-1 :: remoteGet operation -6 s1v14 = %s",
            s1v14->value().c_str());
    LOG_INFO("CLIENT-1 :: remoteGet operation -6 s1v15 = %s",
            s1v15->value().c_str());

    ASSERT(*c1v11 == *s1v11, "GetOnClient1:Values should be equal-1");
    ASSERT(*c1v12 == *s1v12, "GetOnClient1:Values should be equal-2");
    ASSERT(*c1v13 == *s1v13, "GetOnClient1:Values should be equal-3");
    ASSERT(*c1v14 == *s1v14, "GetOnClient1:Values should be equal-4");
    ASSERT(*c1v15 == *s1v15, "GetOnClient1:Values should be equal-5");
    LOG("CLIENT-1 ::local GET operation....Done");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, GetOnClient2)
  {
    auto regPtr = getHelper()->getRegion(regNames[0]);
    thread2->stop();
    delete thread2;

    dunit::sleep(3000);
    // localGet
    auto keyPtr1 = CacheableKey::create("key-1");
    auto keyPtr2 = CacheableKey::create("key-2");
    auto keyPtr3 = CacheableKey::create("key-3");
    auto keyPtr4 = CacheableKey::create("key-4");
    auto keyPtr5 = CacheableKey::create("key-5");

    c2v11 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr1));
    c2v12 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr2));
    c2v13 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr3));
    c2v14 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr4));
    c2v15 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr5));

    // Print localGets
    // Print local Get Values
    LOG_INFO("CLIENT-2 :: local GET operation  c2v11 = %s",
            c2v11->value().c_str());
    LOG_INFO("CLIENT-2 :: local GET operation  c2v12 = %s",
            c2v12->value().c_str());
    LOG_INFO("CLIENT-2 :: local GET operation  c2v13 = %s",
            c2v13->value().c_str());
    LOG_INFO("CLIENT-2 :: local GET operation  c2v14=%s",
            c2v14->value().c_str());
    LOG_INFO("CLIENT-2 :: local GET operation....Done c2v15=%s",
            c2v15->value().c_str());

    LOG("CLIENT-2 :: local GET operation....Done");

    // localDestroy
    regPtr->localDestroy(keyPtr1);
    regPtr->localDestroy(keyPtr2);
    regPtr->localDestroy(keyPtr3);
    regPtr->localDestroy(keyPtr4);
    regPtr->localDestroy(keyPtr5);
    LOG("CLIENT-2 :: localDestroy() operation....Done");

    // remoteGet
    s2v11 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr1));
    s2v12 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr2));
    s2v13 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr3));
    s2v14 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr4));
    s2v15 = std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr5));

    // Print remoteGet Values
    LOG_INFO("CLIENT-2 :: remoteGet operation  s2v11 = %s",
            s2v11->value().c_str());
    LOG_INFO("CLIENT-2 :: remoteGet operation  s2v12 = %s",
            s2v12->value().c_str());
    LOG_INFO("CLIENT-2 :: remoteGet operation  s2v13 = %s",
            s2v13->value().c_str());
    LOG_INFO("CLIENT-2 :: remoteGet operation  s2v14 = %s",
            s2v14->value().c_str());
    LOG_INFO("CLIENT-2 :: remoteGet operation  s2v15 = %s",
            s2v15->value().c_str());

    ASSERT(*c2v11 == *s2v11, "GetOnClient2:Values should be equal-1");
    ASSERT(*c2v12 == *s2v12, "GetOnClient2:Values should be equal-2");
    ASSERT(*c2v13 == *s2v13, "GetOnClient2:Values should be equal-3");
    ASSERT(*c2v14 == *s2v14, "GetOnClient2:Values should be equal-4");
    ASSERT(*c2v15 == *s2v15, "GetOnClient2:Values should be equal-5");

    LOG("GetOnClient2 completed");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    SLEEP(2000);
    if (isLocalServer1) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseClient1)
  { cleanProc(); }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, CloseClient2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServers_With_Locator)
  {
    // starting locator
    if (isLocator1) CacheHelper::initLocator(1);
    LOG(": Locator1 started");

    // starting servers
    if (isLocalServer1) {
      CacheHelper::initServer(1, "cacheserver_concurrency_enabled1.xml",
                              locatorsG, nullptr, false, true, false, true);
      LOG(":SERVER1 started");
    }
    // starting servers
    if (isLocalServer1) {
      CacheHelper::initServer(2, "cacheserver_concurrency_enabled2.xml",
                              locatorsG, nullptr, false, true, false, true);
      LOG(":SERVER2 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServers_With_Locator_Disk)
  {
    // starting locator
    if (isLocator1) CacheHelper::initLocator(1);
    LOG(": Locator1 started");

    // starting servers
    if (isLocalServer1) {
      CacheHelper::initServer(
          1, "cacheserver_concurrency_enabled_disk_replicate1.xml", locatorsG,
          nullptr, false, true, false, true);
      LOG(":SERVER1 started");
    }
    // starting servers
    if (isLocalServer1) {
      CacheHelper::initServer(
          2, "cacheserver_concurrency_enabled_disk_replicate2.xml", locatorsG,
          nullptr, false, true, false, true);
      LOG(":SERVER2 started");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServers_With_Locator_Partition)
  {
    // starting locator
    if (isLocator1) CacheHelper::initLocator(1);
    LOG(": Locator1 started");

    // starting servers
    if (isLocalServer1) {
      CacheHelper::initServer(1, "cacheserver_concurrency_enabled_disk1.xml",
                              locatorsG, nullptr, false, true, false, true);
      LOG(":SERVER1 started");
    }
    // starting servers
    if (isLocalServer1) {
      CacheHelper::initServer(2, "cacheserver_concurrency_enabled_disk2.xml",
                              locatorsG, nullptr, false, true, false, true);
      LOG(":SERVER2 started");
    }
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(SERVER1, CloseServers_With_Locator)
  {
    if (isLocalServer1) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }

    if (isLocalServer1) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }

    if (isLocator1) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTVERSIONEDOPS_H_
