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
#include <geode/GeodeCppCache.hpp>
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "ThinClientSSLAuthCorrupt"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

using namespace apache::geode::client;
using namespace test;

CacheHelper* cacheHelper = NULL;
bool isLocalServer = false;

static bool isLocator = false;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

#define CLIENT1 s1p1
#define SERVER1 s2p1

void initClient(const bool isthinClient) {
  if (cacheHelper == NULL) {
    PropertiesPtr props = Properties::create();
    props->insert("ssl-enabled", "true");
    std::string keystore = std::string(ACE_OS::getenv("TESTSRC")) + "/keystore";
    std::string pubkey = keystore + "/client_truststore.pem";
    std::string privkey = keystore + "/client_keystore_corrupt.pem";
    props->insert("ssl-keystore", privkey.c_str());
    props->insert("ssl-truststore", pubkey.c_str());
    cacheHelper = new CacheHelper(isthinClient, props);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
void cleanProc() {
  if (cacheHelper != NULL) {
    delete cacheHelper;
    cacheHelper = NULL;
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != NULL, "No cacheHelper initialized.");
  return cacheHelper;
}


void createPooledRegion(const char* name, bool ackMode, const char* locators,
                        const char* poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  RegionPtr regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != NULLPTR, "Failed to create region.");
  LOG("Pooled Region created.");
}

void createEntry(const char* name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  CacheableKeyPtr keyPtr = createKey(key);
  CacheableStringPtr valPtr = CacheableString::create(value);

  RegionPtr regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != NULLPTR, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  //verifyEntry(name, key, value);
  LOG("Entry created.");
}



const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateLocator1_With_SSL_untrustedCert)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(1, true, false, -1, 0, false);
    LOG("Locator1 started with SSL");
  }
END_TASK_DEFINITION



DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_With_Locator_And_SSL_untrustedCert)
  {
    // starting servers
    if (isLocalServer) CacheHelper::initServer(1, NULL, locatorsG, NULL, true, true, false, false, false);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateClient1)
  { initClient(true); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CreateRegions1_PoolLocators)
  {
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    RegionPtr regPtr = getHelper()->getRegion(regionNames[0]);
    try {
      regPtr->registerAllKeys(false, NULLPTR, false, false);
      FAIL("Should have got NotConnectedException during registerAllKeys");
    }
    catch (NotConnectedException exp) {
      LOG("Connection Failed as expected via NotConnectedException");
    }
    LOG("CreateRegions1_PoolLocators complete.");
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

DUNIT_TASK_DEFINITION(SERVER1, CloseLocator_With_SSL)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1, true);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_MAIN
  {
    CALL_TASK(CreateLocator1_With_SSL_untrustedCert);
    CALL_TASK(CreateServer1_With_Locator_And_SSL_untrustedCert)

    CALL_TASK(CreateClient1);

    CALL_TASK(CreateRegions1_PoolLocators);

    CALL_TASK(CloseCache1);
    CALL_TASK(CloseServer1);

    CALL_TASK(CloseLocator_With_SSL);
  }
END_MAIN
