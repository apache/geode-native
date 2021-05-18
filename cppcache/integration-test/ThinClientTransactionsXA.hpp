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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTTRANSACTIONSXA_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTTRANSACTIONSXA_H_

#include "fw_dunit.hpp"
#include <ace/Auto_Event.h>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <string>
#include <geode/TransactionId.hpp>
#include <geode/CacheTransactionManager.hpp>

#define ROOT_NAME "ThinClientTransactionsXA"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheServerException;
using apache::geode::client::CacheTransactionManager;
using apache::geode::client::EntryExistsException;
using apache::geode::client::EntryNotFoundException;
using apache::geode::client::IllegalStateException;
using apache::geode::client::Properties;
using apache::geode::client::TransactionException;
using apache::geode::client::TransactionId;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2
#define CREATE_TWICE_KEY "__create_twice_key"
#define CREATE_TWICE_VALUE "__create_twice_value"
CacheHelper* cacheHelper = nullptr;
static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 0;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    config->insert("suspended-tx-timeout", std::chrono::seconds(1));
    cacheHelper = new CacheHelper(isthinClient, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey) {
  // Verify key and value exist in this region, in this process.
  const char* value = val ? val : "";
  char* buf =
      reinterpret_cast<char*>(malloc(1024 + strlen(key) + strlen(value)));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (noKey) {
    sprintf(buf, "Verify key %s does not exist in region %s", key, name);
  } else if (!val) {
    sprintf(buf, "Verify value for key %s does not exist in region %s", key,
            name);
  } else {
    sprintf(buf, "Verify value for key %s is: %s in region %s", key, value,
            name);
  }
  LOG(buf);
  free(buf);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (noKey == false) {  // need to find the key!
    ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
  }
  if (val != nullptr) {  // need to have a value!
    ASSERT(regPtr->containsValueForKey(keyPtr), "Value not found in region.");
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  uint32_t SLEEP = 10;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (noKey) {
      if (regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key found in region.");
    }
    if (val == nullptr) {
      if (regPtr->containsValueForKey(keyPtr)) {
        containsValueCnt++;
      } else {
        break;
      }
      ASSERT(containsValueCnt < MAX, "Value found in region.");
    }

    if (val != nullptr) {
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr));

      ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
      LOG("In verify loop, get returned " + checkPtr->value() + " for key " +
          key);

      if (strcmp(checkPtr->value().c_str(), value) != 0) {
        testValueCnt++;
      } else {
        break;
      }
      ASSERT(testValueCnt < MAX, "Incorrect value found.");
    }
    dunit::sleep(SLEEP);
  }
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char* name, const char* key, const char* val,
                  int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyEntry() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

void createRegion(const char* name, bool ackMode, const char* endpoints,
                  bool clientNotificationEnabled = false,
                  bool cachingEnable = true) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createRegion(name, ackMode, cachingEnable, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
void createPooledRegion(const std::string& name, bool ackMode, const std::string& locators,
                        const std::string& poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name.c_str(), ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createPooledRegionSticky(const std::string& name, bool ackMode,
                              const std::string& locators, const std::string& poolname,
                              bool clientNotificationEnabled = false,
                              bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name.c_str(), ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createPooledRegionSticky(
      name, ackMode, locators, poolname, cachingEnable,
      clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createEntry(const char* name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  // verifyEntry( name, key, value );
  LOG("Entry created.");
}
void createEntryTwice(const char* name, const char* key, const char* value) {
  LOG("createEntryTwice() entered.");
  char message[500];
  sprintf(message, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  LOG(message);
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);
  auto regPtr = getHelper()->getRegion(name);
  regPtr->create(keyPtr, valPtr);
  try {
    regPtr->create(keyPtr, valPtr);
  } catch (const EntryExistsException& geodeExcp) {
    LOG(geodeExcp.what());
    LOG("createEntryTwice() Clean Exit.");
    return;
  }
  ASSERT(false,
         "Creating key twice is not allowed and while doing that exception was "
         "not thrown");
  return;  // This return will never reach
}

void updateEntry(const char* name, const char* key, const char* value) {
  LOG("updateEntry() entered.");
  fprintf(stdout, "Updating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  ASSERT(regPtr->containsValueForKey(keyPtr),
         "Value should have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

void doGetAgain(const char* name, const char* key, const char* value) {
  LOG("doGetAgain() entered.");
  fprintf(stdout,
          "get for entry -- key: %s  expecting value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "get  region name%s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In doGetAgain, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("GetAgain complete.");
}

void doNetsearch(const char* name, const char* key, const char* value) {
  LOG("doNetsearch() entered.");
  fprintf(
      stdout,
      "Netsearching for entry -- key: %s  expecting value: %s in region %s\n",
      key, value, name);
  fflush(stdout);
  static int count = 0;
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "netsearch  region %s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  if (count == 0) {
    ASSERT(!regPtr->containsKey(keyPtr),
           "Key should not have been found in region.");
    ASSERT(!regPtr->containsValueForKey(keyPtr),
           "Value should not have been found in region.");
    count++;
  }
  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In net search, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4",
                      "Key-5", "Key-6", "Key-7"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4",
                      "Value-5", "Value-6", "Value-7"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4", "New Value-5", "New Value-6",
                       "New Value-7"};

const char* regionNames[] = {"DistRegionAck", "not-used", "not-used"};

const bool USE_ACK = true;
const bool NO_ACK = false;
#include "LocatorHelper.hpp"
#define THREADERRORCHECK(x, y) \
  do {                         \
    if (!(x)) {                \
      m_isFailed = true;       \
      sprintf(m_error, y);     \
      return -1;               \
    }                          \
  } while (0)

class SuspendTransactionThread : public ACE_Task_Base {
 private:
  TransactionId* m_suspendedTransaction;
  bool m_sleep;
  ACE_Auto_Event* m_txEvent;

 public:
  SuspendTransactionThread(bool sleep, ACE_Auto_Event* txEvent)
      : m_suspendedTransaction(nullptr), m_sleep(sleep), m_txEvent(txEvent) {}

  int svc(void) override {
    char buf[1024];
    sprintf(buf, " In SuspendTransactionThread");
    LOG(buf);

    auto txManager = getHelper()->getCache()->getCacheTransactionManager();

    txManager->begin();

    createEntry(regionNames[0], keys[4], vals[4]);

    m_suspendedTransaction = &txManager->getTransactionId();

    if (m_sleep) {
      m_txEvent->wait();
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    m_suspendedTransaction = &txManager->suspend();
    sprintf(buf, " Out SuspendTransactionThread");
    LOG(buf);

    getHelper()
        ->getCache()
        ->getPoolManager()
        .find("__TESTPOOL1_")
        ->releaseThreadLocalConnection();

    return 0;
  }
  void start() { activate(); }
  void stop() { wait(); }
  TransactionId& getSuspendedTx() { return *m_suspendedTransaction; }
};
class ResumeTransactionThread : public ACE_Task_Base {
 private:
  TransactionId& m_suspendedTransaction;
  bool m_commit;
  bool m_tryResumeWithSleep;
  bool m_isFailed;
  char m_error[256];
  ACE_Auto_Event* m_txEvent;

 public:
  ResumeTransactionThread(TransactionId& suspendedTransaction, bool commit,
                          bool tryResumeWithSleep, ACE_Auto_Event* txEvent)
      : m_suspendedTransaction(suspendedTransaction),
        m_commit(commit),
        m_tryResumeWithSleep(tryResumeWithSleep),
        m_isFailed(false),
        m_txEvent(txEvent) {}

  int svc(void) override {
    char buf[1024];
    sprintf(buf, "In ResumeTransactionThread");
    LOG(buf);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    THREADERRORCHECK(regPtr0 != nullptr,
                     "In ResumeTransactionThread - Region not found.");

    auto keyPtr4 = CacheableKey::create(keys[4]);
    auto keyPtr5 = CacheableKey::create(keys[5]);
    auto keyPtr6 = CacheableKey::create(keys[6]);

    THREADERRORCHECK(!regPtr0->containsKeyOnServer(keyPtr4),
                     "In ResumeTransactionThread - Key should not have been "
                     "found in region.");

    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    if (m_tryResumeWithSleep) {
      THREADERRORCHECK(!txManager->isSuspended(m_suspendedTransaction),
                       "In ResumeTransactionThread - the transaction should "
                       "NOT be in suspended state");
    } else {
      THREADERRORCHECK(txManager->isSuspended(m_suspendedTransaction),
                       "In ResumeTransactionThread - the transaction should be "
                       "in suspended state");
    }

    THREADERRORCHECK(
        txManager->exists(m_suspendedTransaction),
        "In ResumeTransactionThread - the transaction should exist");

    if (m_tryResumeWithSleep) {
      m_txEvent->signal();
      txManager->tryResume(m_suspendedTransaction, std::chrono::seconds(30));
    } else {
      txManager->resume(m_suspendedTransaction);
    }

    THREADERRORCHECK(
        regPtr0->containsKeyOnServer(keyPtr4),
        "In ResumeTransactionThread - Key should have been found in region.");

    if (m_commit) {
      txManager->prepare();
      txManager->commit();
      THREADERRORCHECK(
          regPtr0->containsKeyOnServer(keyPtr4),
          "In ResumeTransactionThread - Key should have been found in region.");
    } else {
      txManager->prepare();
      txManager->rollback();
      THREADERRORCHECK(!regPtr0->containsKeyOnServer(keyPtr4),
                       "In ResumeTransactionThread - Key should not have been "
                       "found in region.");
    }

    if (m_commit) {
      regPtr0->destroy(keyPtr4);

      THREADERRORCHECK(!regPtr0->containsKeyOnServer(keyPtr4),
                       "In ResumeTransactionThread - Key should not have been "
                       "found in region.");
    } else {
      try {
        regPtr0->destroy(keyPtr4);
        FAIL("Should have got EntryNotFoundException for keyPtr4");
      } catch (EntryNotFoundException& /*ex*/) {
        LOG("Got expected EntryNotFoundException for keyPtr4");
      }
    }
    getHelper()
        ->getCache()
        ->getPoolManager()
        .find("__TESTPOOL1_")
        ->releaseThreadLocalConnection();
    sprintf(buf, " Out ResumeTransactionThread");
    LOG(buf);
    return 0;
  }
  void start() { activate(); }
  void stop() { wait(); }
  bool isFailed() { return m_isFailed; }
  char* getError() { return m_error; }
};

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SuspendResumeCommit)
  {
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    ASSERT(regPtr0 != nullptr, "In SuspendResumeCommit - Region not found.");

    auto keyPtr4 = CacheableKey::create(keys[4]);
    auto keyPtr5 = CacheableKey::create(keys[5]);
    auto keyPtr6 = CacheableKey::create(keys[6]);

    txManager->begin();
    createEntry(regionNames[0], keys[4], vals[4]);
    auto& m_suspendedTransaction = txManager->suspend();

    ASSERT(
        !regPtr0->containsKeyOnServer(keyPtr4),
        "In SuspendResumeCommit - Key should not have been found in region.");

    ASSERT(txManager->isSuspended(m_suspendedTransaction),
           "In SuspendResumeCommit - the transaction should be in suspended "
           "state");
    ASSERT(txManager->exists(m_suspendedTransaction),
           "In SuspendResumeCommit - the transaction should exist");

    txManager->resume(m_suspendedTransaction);

    ASSERT(
        !txManager->tryResume(m_suspendedTransaction),
        "SuspendResumeRollback: Transaction shouldnt have been resumed again");
    ASSERT(regPtr0->containsKeyOnServer(keyPtr4),
           "In SuspendResumeCommit - Key should have been found in region.");

    txManager->prepare();
    txManager->commit();
    ASSERT(regPtr0->containsKeyOnServer(keyPtr4),
           "In SuspendResumeCommit - Key should have been found in region.");

    regPtr0->destroy(keyPtr4);

    ASSERT(!txManager->isSuspended(m_suspendedTransaction),
           "In SuspendResumeCommit the transaction should NOT present");
    ASSERT(!txManager->exists(m_suspendedTransaction),
           "In SuspendResumeCommit - the transaction should NOT exist");
    ASSERT(!txManager->tryResume(m_suspendedTransaction),
           "In SuspendResumeCommit - the transaction should NOT have been "
           "resumed");

    bool resumeExc = false;
    try {
      txManager->resume(m_suspendedTransaction);
    } catch (const IllegalStateException&) {
      resumeExc = true;
    }

    ASSERT(resumeExc,
           "SuspendResumeCommit: Transaction shouldnt have been resumed");

    bool threwTransactionException = false;
    try {
      txManager->suspend();
    } catch (const TransactionException&) {
      threwTransactionException = true;
    }
    ASSERT(threwTransactionException,
           "SuspendResumeCommit: Transaction shouldnt have been suspended");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SuspendTimeOut)
  {
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    auto keyPtr4 = CacheableKey::create(keys[4]);
    auto keyPtr5 = CacheableKey::create(keys[5]);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    ASSERT(regPtr0 != nullptr, "In SuspendTimeOut - Region not found.");

    txManager->begin();
    createEntry(regionNames[0], keys[4], vals[4]);
    auto& tid1 = txManager->suspend();

    txManager->begin();
    createEntry(regionNames[0], keys[5], vals[5]);
    auto& tid2 = txManager->suspend();

    txManager->resume(tid1);
    createEntry(regionNames[0], keys[6], vals[6]);
    txManager->prepare();
    txManager->commit();

    ASSERT(txManager->isSuspended(tid2),
           "In SuspendTimeOut the transaction should be present");
    ASSERT(txManager->exists(tid2),
           "In SuspendTimeOut - the transaction should exist");

    ASSERT(regPtr0->containsKeyOnServer(keyPtr4),
           "In SuspendTimeOut - Key should have been found in region.");
    ASSERT(!regPtr0->containsKeyOnServer(keyPtr5),
           "In SuspendTimeOut - Key should not have been found in region.");

    std::this_thread::sleep_for(std::chrono::seconds(65));
    ASSERT(!txManager->tryResume(tid2),
           "In SuspendTimeOut - the transaction should NOT have been resumed");
    ASSERT(!txManager->isSuspended(tid2),
           "In SuspendTimeOut the transaction should NOT present");
    ASSERT(!txManager->exists(tid2),
           "In SuspendTimeOut - the transaction should NOT exist");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SuspendResumeRollback)
  {
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    auto keyPtr4 = CacheableKey::create(keys[4]);
    auto keyPtr5 = CacheableKey::create(keys[5]);
    auto keyPtr6 = CacheableKey::create(keys[6]);

    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    ASSERT(regPtr0 != nullptr, "In SuspendResumeRollback - Region not found.");

    txManager->begin();
    createEntry(regionNames[0], keys[4], vals[4]);
    auto& m_suspendedTransaction = txManager->suspend();

    ASSERT(
        !regPtr0->containsKeyOnServer(keyPtr4),
        "In SuspendResumeRollback - Key should not have been found in region.");

    ASSERT(txManager->isSuspended(m_suspendedTransaction),
           "In SuspendResumeRollback the transaction should be in suspended "
           "state");
    ASSERT(txManager->exists(m_suspendedTransaction),
           "In SuspendResumeRollback - the transaction should exist");

    txManager->resume(m_suspendedTransaction);

    ASSERT(regPtr0->containsKeyOnServer(keyPtr4),
           "In SuspendResumeRollback - Key should have been found in region.");

    txManager->prepare();
    txManager->rollback();
    ASSERT(
        !regPtr0->containsKeyOnServer(keyPtr4),
        "In SuspendResumeRollback - Key should not have been found in region.");

    try {
      regPtr0->destroy(keyPtr4);
      FAIL("Should have got EntryNotFoundException for keyPtr4");
    } catch (EntryNotFoundException& /*ex*/) {
      LOG("Got expected EntryNotFoundException for keyPtr4");
    }

    ASSERT(!txManager->isSuspended(m_suspendedTransaction),
           "In SuspendResumeRollback the transaction should NOT present");
    ASSERT(!txManager->exists(m_suspendedTransaction),
           "In SuspendResumeRollback - the transaction should NOT exist");
    ASSERT(!txManager->tryResume(m_suspendedTransaction),
           "In SuspendResumeRollback - the transaction should NOT have been "
           "resumed");
    bool resumeExc = false;
    try {
      txManager->resume(m_suspendedTransaction);
    } catch (const IllegalStateException&) {
      resumeExc = true;
    }

    ASSERT(resumeExc,
           "SuspendResumeRollback: Transaction shouldnt have been resumed");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, SuspendResumeInThread)
  {
    // start suspend thread  and resume thread and rollback immedidately
    char buf[1024];
    sprintf(
        buf,
        "start suspend thread  and resume thread and rollback immedidately");
    LOG(buf);
    ACE_Auto_Event txEvent;

    SuspendTransactionThread* suspendTh =
        new SuspendTransactionThread(false, &txEvent);
    suspendTh->activate();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ResumeTransactionThread* resumeTh = new ResumeTransactionThread(
        suspendTh->getSuspendedTx(), false, false, &txEvent);
    resumeTh->activate();

    suspendTh->wait();
    delete suspendTh;
    resumeTh->wait();
    delete resumeTh;

    // start suspend thread  and resume thread and commit immedidately
    sprintf(buf,
            "start suspend thread  and resume thread and commit immedidately");
    LOG(buf);
    suspendTh = new SuspendTransactionThread(false, &txEvent);
    suspendTh->activate();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    resumeTh = new ResumeTransactionThread(suspendTh->getSuspendedTx(), true,
                                           false, &txEvent);
    resumeTh->activate();

    suspendTh->wait();
    delete suspendTh;
    resumeTh->wait();
    delete resumeTh;

    // start suspend thread  and tryresume thread with rollback. make tryResume
    // to
    // sleep
    sprintf(buf,
            "start suspend thread  and tryresume thread with rollback. make "
            "tryResume to sleep");
    LOG(buf);
    suspendTh = new SuspendTransactionThread(true, &txEvent);
    suspendTh->activate();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    resumeTh = new ResumeTransactionThread(suspendTh->getSuspendedTx(), false,
                                           true, &txEvent);
    resumeTh->activate();

    suspendTh->wait();
    delete suspendTh;
    resumeTh->wait();
    delete resumeTh;

    // start suspend thread  and tryresume thread with commit. make tryResume to
    // sleep
    sprintf(buf,
            "start suspend thread  and tryresume thread with commit. make "
            "tryResume to sleep");
    LOG(buf);
    suspendTh = new SuspendTransactionThread(true, &txEvent);
    suspendTh->activate();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sprintf(buf, "suspendTh->activate();");
    LOG(buf);

    resumeTh = new ResumeTransactionThread(suspendTh->getSuspendedTx(), true,
                                           true, &txEvent);
    resumeTh->activate();

    suspendTh->wait();
    delete suspendTh;
    resumeTh->wait();
    delete resumeTh;
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateNonexistentServerRegion_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion("non-region", USE_ACK, locatorsG, "__TESTPOOL1_");
    try {
      createEntry("non-region", keys[0], vals[0]);
      FAIL(
          "Expected exception when doing operations on a non-existent region.");
    } catch (const CacheServerException& ex) {
      printf(
          "Got expected CacheServerException when performing operation "
          "on a non-existent region: %s\n",
          ex.what());
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1,
                      CreateNonexistentServerRegion_Pooled_Locator_Sticky)
  {
    initClient(true);
    createPooledRegionSticky("non-region", USE_ACK, locatorsG, "__TESTPOOL1_");
    try {
      createEntry("non-region", keys[0], vals[0]);
      FAIL(
          "Expected exception when doing operations on a non-existent region.");
    } catch (const CacheServerException& ex) {
      printf(
          "Got expected CacheServerException when performing operation "
          "on a non-existent region: %s\n",
          ex.what());
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_");
    LOG("StepOne_Pooled complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator_Sticky)
  {
    createPooledRegionSticky(regionNames[0], USE_ACK, locatorsG,
                             "__TESTPOOL1_");
    LOG("StepOne_Pooled_Locator_Sticky complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator_Sticky)
  {
    initClient(true);
    createPooledRegionSticky(regionNames[0], USE_ACK, locatorsG,
                             "__TESTPOOL1_");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    txManager->begin();
    createEntry(regionNames[0], keys[0], vals[0]);
    txManager->prepare();
    txManager->commit();
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    txManager->begin();
    createEntry(regionNames[0], keys[1], vals[1]);
    txManager->prepare();
    txManager->commit();
    verifyEntry(regionNames[0], keys[1], vals[1]);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    auto reg0 = getHelper()->getRegion(regionNames[0]);
    auto vec0 = reg0->serverKeys();
    ASSERT(vec0.size() == 2, "Should have 2 keys in first region.");
    std::string key0, key1;
    key0 = vec0[0]->toString().c_str();
    key1 = vec0[1]->toString().c_str();
    ASSERT(key0 != key1, "The two keys should be different in first region.");
    ASSERT(key0 == keys[0] || key0 == keys[1],
           "Unexpected key in first region.");
    ASSERT(key1 == keys[0] || key1 == keys[1],
           "Unexpected key in first region.");

    doNetsearch(regionNames[0], keys[1], vals[1]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    auto txManager = getHelper()->getCache()->getCacheTransactionManager();
    txManager->begin();
    updateEntry(regionNames[0], keys[1], nvals[1]);
    txManager->prepare();
    txManager->commit();
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  { createEntryTwice(regionNames[0], CREATE_TWICE_KEY, CREATE_TWICE_VALUE); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
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

void runTransactionOps() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(CreateNonexistentServerRegion_Pooled_Locator);
  CALL_TASK(StepOne_Pooled_Locator);
  CALL_TASK(StepTwo_Pooled_Locator);

  CALL_TASK(StepThree);
  CALL_TASK(StepFour);
  CALL_TASK(StepFive);
  CALL_TASK(StepSix);
  CALL_TASK(StepSeven);

  CALL_TASK(SuspendResumeInThread);
  CALL_TASK(SuspendResumeCommit);
  CALL_TASK(SuspendResumeRollback);
  CALL_TASK(SuspendTimeOut);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);
  CALL_TASK(CloseLocator1);
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTTRANSACTIONSXA_H_
