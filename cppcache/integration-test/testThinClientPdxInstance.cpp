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

#include <string>

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <ace/Date_Time.h>

#include <geode/PdxInstance.hpp>
#include <geode/PdxInstanceFactory.hpp>
#include <geode/WritablePdxInstance.hpp>

#include "fw_dunit.hpp"
#include "testobject/PdxType.hpp"
#include "testobject/NestedPdxObject.hpp"
#include "CachePerfStats.hpp"
#include "LocalRegion.hpp"

#define ROOT_NAME "testThinClientPdxInstance"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"
#include "CacheImpl.hpp"
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"

using PdxTests::Address;

using testobject::ChildPdx;
using testobject::ParentPdx;
using testobject::SerializePdx;

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableArrayList;
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableEnum;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableHashSet;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableLinkedHashSet;
using apache::geode::client::CacheableObjectArray;
using apache::geode::client::CacheableVector;
using apache::geode::client::CacheHelper;
using apache::geode::client::CacheRegionHelper;
using apache::geode::client::IllegalStateException;
using apache::geode::client::LocalRegion;
using apache::geode::client::PdxFieldTypes;
using apache::geode::client::PdxInstance;
using apache::geode::client::PdxSerializable;
using apache::geode::client::Properties;
using apache::geode::client::WritablePdxInstance;

bool isLocalServer = false;

CacheHelper *cacheHelper = nullptr;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
static bool isLocator = false;
const bool USE_ACK = true;
const bool NO_ACK = false;

const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);

const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

template <typename T1, typename T2>
bool genericValCompare(T1 value1, T2 value2) /*const*/
{
  if (value1 != value2) return false;
  return true;
}

template <typename T1, typename T2, typename L>
bool genericCompare(std::vector<T1> value1, std::vector<T2> value2,
                    L length) /*const*/
{
  L i = 0;
  while (i < length) {
    if (value1[i] != value2[i]) {
      return false;
    } else {
      i++;
    }
  }
  return true;
}

template <typename T1, typename T2>
bool generic2DCompare(T1 **value1, T2 **value2, int length,
                      int *arrLengths) /*const*/
{
  for (int j = 0; j < length; j++) {
    for (int k = 0; k < arrLengths[j]; k++) {
      LOG_INFO("generic2DCompare value1 = %d \t value2 = %d ", value1[j][k],
               value2[j][k]);
      if (value1[j][k] != value2[j][k]) return false;
    }
  }
  LOG_INFO("PdxObject::genericCompare Line_34");
  return true;
}

void initClient(const bool isthinClient, bool isPdxReadSerailized) {
  LOG_INFO("isPdxReadSerailized = %d ", isPdxReadSerailized);
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    config->insert("enable-time-statistics", "true");
    LOG_INFO("enabled-time-statistics ");
    cacheHelper = new CacheHelper(isthinClient, false, isPdxReadSerailized,
                                  config, false);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
static int clientWithXml = 0;
void initClient(const char *clientXmlFile) {
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (clientWithXml > 2) config->insert("grid-client", "true");
    clientWithXml += 1;
    config->insert("enable-time-statistics", "true");
    cacheHelper = new CacheHelper(nullptr, clientXmlFile, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}
CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}
void createPooledRegion(const std::string &name, bool ackMode,
                        const std::string &locators,
                        const std::string &poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name.c_str(),
          ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}
DUNIT_TASK_DEFINITION(SERVER1, CreateLocator1)
  {
    // starting locator
    if (isLocator) CacheHelper::initLocator(1);
    LOG("Locator1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_pdxinstance_hashcode.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1_With_Locator)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_pdxinstance_hashcode.xml",
                              locatorsG);
    }
    LOG("SERVER1 with locator started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator_PdxReadSerialized)
  {
    initClient(true, true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       false, false);
    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator_PdxReadSerialized)
  {
    initClient(true, true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       false, false);
    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2,
                      StepTwo_Pooled_Locator_CachingEnabled_PdxReadSerialized)
  {
    initClient(true, true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       false, true);
    LOG("StepTwo_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pooled_Locator)
  {
    initClient(true, false);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       false, false);
    LOG("StepOne_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pooled_Locator)
  {
    initClient(true, false);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__",
                       false, false);
    LOG("StepTwo_Pooled_Locator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, testReadSerializedXMLProperty_Client2)
  {
    ASSERT(cacheHelper->getCache()->getPdxReadSerialized() == true,
           "Pdx read serialized property should be true.");
    LOG("testReadSerializedXMLProperty complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, testReadSerializedXMLProperty_Client1)
  {
    ASSERT(cacheHelper->getCache()->getPdxReadSerialized() == true,
           "Pdx read serialized property should be true.");
    LOG("testReadSerializedXMLProperty complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putPdxWithIdentityField)
  {
    LOG("putPdxWithIdentityField started ");

    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          SerializePdx::createDeserializable);
      LOG("SerializePdx Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("SerializePdx IllegalStateException");
    }

    auto rptr = getHelper()->getRegion(regionNames[0]);
    // Creating object of type PdxObject
    /*SerializePdxPtr*/ auto sp = std::make_shared<SerializePdx>(true);

    // PUT Operation
    rptr->put(CacheableInt32::create(1), sp);
    LOG("pdxPut 1 completed ");

    LOG("putPdxWithIdentityField complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putCacheableObjectArrayWithPdxFields)
  {
    LOG("putCacheableObjectArrayWithPdxFields started ");

    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
      LOG("Address Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("Address IllegalStateException");
    }

    auto rptr = getHelper()->getRegion(regionNames[0]);

    // Creating CacheableobjectArray of type PdxObject
    std::shared_ptr<CacheableObjectArray> objectArray =
        CacheableObjectArray::create();
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(1, "street0", "city0")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(2, "street1", "city1")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(3, "street2", "city2")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(4, "street3", "city3")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(5, "street4", "city4")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(6, "street5", "city5")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(7, "street6", "city6")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(8, "street7", "city7")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(9, "street8", "city8")));
    objectArray->push_back(
        std::shared_ptr<Address>(new Address(10, "street9", "city9")));

    // PUT Operation
    rptr->put(CacheableInt32::create(100), objectArray);
    LOG("putCacheableObjectArrayWithPdxFields on key 100 completed ");

    LOG("putCacheableObjectArrayWithPdxFields complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verifyPdxIdentityField)
  {
    LOG("verifyPdxIdentityField started ");

    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          SerializePdx::createDeserializable);
      LOG("SerializePdx Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("SerializePdx IllegalStateException");
    }

    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto pi = std::dynamic_pointer_cast<PdxInstance>(
        rptr->get(CacheableInt32::create(1)));
    LOG("PdxInstancePtr get complete");

    LOG("Statistics for for (PdxTests.PdxType) PdxInstance");
    LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(rptr.get()));

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 0,
           "pdxInstanceDeserialization should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 1,
           "pdxInstanceCreations should be equal to 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than equal to 0.");

    ASSERT(pi->getFieldNames()->length() == 4,
           "number of fields should be four in SerializePdx");

    ASSERT(pi->isIdentityField("i1") == true,
           "SerializePdx1.i1 should be identity field");

    ASSERT(pi->isIdentityField("i2") == false,
           "SerializePdx1.i2 should NOT be identity field");

    ASSERT(pi->hasField("i1") == true,
           "SerializePdx1.i1 should be in PdxInstance stream");

    ASSERT(pi->hasField("i3") == false,
           "There is no field i3 in SerializePdx1's PdxInstance stream");

    std::shared_ptr<CacheableKey> javaPdxHCKey =
        CacheableKey::create("javaPdxHC");
    auto pIPtr2 = std::dynamic_pointer_cast<Cacheable>(rptr->get(javaPdxHCKey));
    LOG("javaPdxHCKey get done");
    CacheableInt32 *val = dynamic_cast<CacheableInt32 *>(pIPtr2.get());
    LOG("javaPdxHCKey cast done");
    int javaPdxHC = val->value();
    LOG_INFO("javaPdxHC hash code = %d ", javaPdxHC);

    int pdxInstHashcode = pi->hashcode();

    ASSERT(
        javaPdxHC == pdxInstHashcode,
        "Pdxhashcode for identity field object SerializePdx1 not matched with "
        "java pdx hash code.");

    auto pi2 = std::dynamic_pointer_cast<PdxInstance>(
        rptr->get(CacheableInt32::create(1)));

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 0,
           "pdxInstanceDeserialization should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 2,
           "pdxInstanceCreations should be equal to 2.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    bool ret = false;
    ret = (*pi == *pi2);
    LOG_INFO("PdxObject ret = %d ", ret);
    ASSERT(ret == true, "Both PdxInstances should be equal.");

    LOG("verifyPdxIdentityField complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verifyCacheableObjectArrayWithPdxField)
  {
    LOG("verifyCacheableObjectArrayWithPdxField started ");

    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
      LOG("Address Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("Address IllegalStateException");
    }

    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto objectArrayPtr = std::dynamic_pointer_cast<CacheableObjectArray>(
        rptr->get(CacheableInt32::create(100)));
    LOG("PdxInstancePtr get on key 100 complete");

    for (size_t i = 0; i < objectArrayPtr->size(); i++) {
      auto pi = std::dynamic_pointer_cast<PdxInstance>(objectArrayPtr->at(i));
      LOG("PdxInstancePtr obtained from CacheableObjectArray");

      auto pdxFactory =
          cacheHelper->getCache()->createPdxInstanceFactory("PdxTests.Address");
      LOG("PdxInstanceFactoryPtr created for PdxTests.Address....");

      pdxFactory.writeInt("_aptNumber", static_cast<int32_t>(i + 1));
      char streetStr[256] = {0};
      sprintf(streetStr, "street%zd", i);
      pdxFactory.writeString("_street", streetStr);
      char cityStr[256] = {0};
      sprintf(cityStr, "city%zd", i);
      pdxFactory.writeString("_city", cityStr);

      auto addrPtr = std::dynamic_pointer_cast<Address>(pi->getObject());
      LOG("AddressPtr created using PdxInstance getObject()....");
      auto newAddrPtr = std::make_shared<Address>(static_cast<int32_t>(i + 1),
                                                  streetStr, cityStr);
      LOG("AddressPtr created using new....");
      ASSERT(addrPtr->equals(*newAddrPtr) == true,
             "Both PdxInstances should be equal.");

      auto retPtr = pdxFactory.create();
      LOG("PdxInstancePtr created....");

      bool ret = false;
      ret = (*pi == *retPtr);
      LOG_INFO("PdxObject ret = %d ", ret);
      ASSERT(ret == true, "Both PdxInstances should be equal.");
    }

    LOG("verifyCacheableObjectArrayWithPdxField complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, putPdxWithNullIdentityFields)
  {
    LOG("putPdxWithNullIdentityFields started ");

    auto rptr = getHelper()->getRegion(regionNames[0]);
    // Creating object of type PdxObject
    auto sp = std::make_shared<SerializePdx>(false);

    // PUT Operation
    rptr->put(CacheableInt32::create(2), sp);
    LOG("pdxPut 1 completed ");

    LOG("putPdxWithNullIdentityFields complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verifyPdxNullIdentityFieldHC)
  {
    LOG("verifyPdxNullIdentityFieldHC started ");

    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto pi = std::dynamic_pointer_cast<PdxInstance>(
        rptr->get(CacheableInt32::create(2)));
    LOG("PdxInstancePtr get complete");

    std::shared_ptr<CacheableKey> javaPdxHCKey =
        CacheableKey::create("javaPdxHC");
    auto pIPtr2 = std::dynamic_pointer_cast<Cacheable>(rptr->get(javaPdxHCKey));
    LOG("javaPdxHCKey get done");
    CacheableInt32 *val = dynamic_cast<CacheableInt32 *>(pIPtr2.get());
    LOG("javaPdxHCKey cast done");
    int javaPdxHC = val->value();
    LOG_INFO("javaPdxHC hash code = %d ", javaPdxHC);

    int pdxInstHashcode = pi->hashcode();

    ASSERT(
        javaPdxHC == pdxInstHashcode,
        "Pdxhashcode for identity field object SerializePdx1 not matched with "
        "java pdx hash code.");

    auto pi2 = std::dynamic_pointer_cast<PdxInstance>(
        rptr->get(CacheableInt32::create(2)));

    bool ret = false;
    ret = (*pi == *pi2);
    LOG_INFO("PdxObject ret = %d ", ret);
    ASSERT(ret == true, "Both PdxInstances should be equal.");

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(CacheableInt32::create(1));
    keys1.push_back(CacheableInt32::create(2));

    const auto valuesMap = rptr->getAll(keys1);
    LOG("getAll on Pdx objects completed.");

    ASSERT(valuesMap.size() == keys1.size(), "getAll size did not match");

    LOG("verifyPdxNullIdentityFieldHC complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, pdxPut)
  {
    LOG("pdxPut started ");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      LOG("PdxObject Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("PdxObject IllegalStateException");
    }

    // Creating object of type PdxObject
    auto pdxobj = std::make_shared<PdxTests::PdxType>();

    auto keyport = CacheableKey::create("pdxput");
    auto keyport1 = CacheableKey::create("pdxput2");
    auto rptr = getHelper()->getRegion(regionNames[0]);

    // PUT Operation
    rptr->put(keyport, pdxobj);
    LOG("pdxPut 1 completed ");

    auto pIPtr1 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));

    LOG("Statistics for for (PdxTests.PdxType) PdxInstance");
    LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(rptr.get()));

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 0,
           "pdxInstanceDeserialization should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 1,
           "pdxInstanceCreations should be equal to 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    auto toString = pIPtr1->toString();
    LOG_INFO("pdxinstance toString = %s ", toString.c_str());
    int pdxInstHashcode = pIPtr1->hashcode();
    LOG_INFO("pdxinstance hash code = %d ", pdxInstHashcode);

    auto javaPdxHCKey = CacheableKey::create("javaPdxHC");
    auto pIPtr2 = std::dynamic_pointer_cast<Cacheable>(rptr->get(javaPdxHCKey));
    LOG("In verifyPdxInstanceHashcode get done");
    CacheableInt32 *val = dynamic_cast<CacheableInt32 *>(pIPtr2.get());
    LOG("In verifyPdxInstanceHashcode cast done");
    int javaPdxHC = val->value();
    LOG_INFO("javaPdxHC hash code = %d ", javaPdxHC);

    ASSERT(javaPdxHC == pdxInstHashcode,
           "Pdxhashcode hashcode not matched with java pdx hash code.");

    // Creating object of type ParentPdx
    auto parentPdxObj = std::make_shared<ParentPdx>(1);

    LOG("pdxPut parentPdxObj created ");
    rptr->put(keyport1, parentPdxObj);

    LOG("pdxPut 2 completed ");

    LOG("pdxPut complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, getObject)
  {
    LOG("getObject started ");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      LOG("PdxObject Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("PdxObject IllegalStateException");
    }

    try {
      serializationRegistry->addPdxSerializableType(
          ChildPdx::createDeserializable);
      LOG("ChildPdx Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("ChildPdx IllegalStateException");
    }

    try {
      serializationRegistry->addPdxSerializableType(
          ParentPdx::createDeserializable);
      LOG("ParentPdx Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("ParentPdx IllegalStateException");
    }

    auto keyport = CacheableKey::create("pdxput");
    auto keyport1 = CacheableKey::create("pdxput2");
    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto pIPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG("PdxObject get Successful....");

    LOG_INFO("pdxinstance classname = " + pIPtr->getClassName());
    ASSERT(pIPtr->getClassName() == "PdxTests.PdxType",
           "pdxInstance.getClassName should return PdxTests.PdxType.");

    auto pt = pIPtr->getObject();

    LOG("PdxObject getObject Successful....");

    LOG("Statistics for for (PdxTests.PdxType) PdxInstance ");
    auto &&lregPtr = std::dynamic_pointer_cast<LocalRegion>(rptr);

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 1,
           "pdxInstanceDeserialization should be equal to 1.");
    ASSERT(
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations() >
            1,
        "pdxInstanceCreations should be greater than 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    auto ptorig = std::make_shared<PdxTests::PdxType>();
    LOG("PdxObject ptorig Successful....");
    PdxTests::PdxType *obj1 = ptorig.get();
    LOG("obj1 Successful....");
    PdxTests::PdxType *obj2 = dynamic_cast<PdxTests::PdxType *>(pt.get());
    LOG("obj2 Successful....");

    ASSERT(obj1->equals(*obj2, true) == true,
           "PdxInstance.getObject not equals original object.");

    auto pIPtr1 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    LOG("ParentPdxObject get Successful....");
    auto toString = pIPtr1->toString();
    LOG_INFO("ParentPdxObject toString = %s ", toString.c_str());
    auto pt1 = std::dynamic_pointer_cast<ParentPdx>(pIPtr1->getObject());
    LOG("ParentPdxObject getObject Successful....");

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 2,
           "pdxInstanceDeserialization should be equal to 2.");
    ASSERT(
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations() >
            1,
        "pdxInstanceCreations should be greater than 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    auto parentPdxObj = std::make_shared<ParentPdx>(1);
    LOG("ParentPdxObject parentPdxObj Successful....");
    ParentPdx *parentObj1 = parentPdxObj.get();
    LOG("parentObj1 Successful....");
    ParentPdx *parentObj2 = dynamic_cast<ParentPdx *>(pt1.get());
    LOG("parentObj2 Successful....");

    ASSERT(parentObj1->equals(*parentObj2, true) == true,
           "PdxInstance.getObject not equals original object.");

    LOG("getObject complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verifyPdxInstanceEquals)
  {
    LOG("Task verifyPdxInstanceEquals started.");

    try {
      auto serializationRegistry =
          CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
              ->getSerializationRegistry();
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      LOG("PdxObject Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("PdxObject IllegalStateException");
    }

    auto keyport = CacheableKey::create("pdxput");
    auto keyport1 = CacheableKey::create("pdxput2");
    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto pIPtr1 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG("get1 Successfully....");
    auto pIPtr2 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG("get2 Successfully....");

    bool ret = false;
    ret = (*pIPtr1 == *pIPtr2);
    LOG_INFO("PdxObject ret = %d ", ret);
    ASSERT(ret == true, "Both PdxInstances should be equal.");

    pIPtr1 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    LOG("parentPdx get1 Successfully....");
    pIPtr2 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    LOG("parentPdx get2 Successfully....");

    ret = (*pIPtr1 == *pIPtr2);
    LOG_INFO("parentPdx ret = %d ", ret);
    ASSERT(ret == true, "Both PdxInstances should be equal.");

    LOG("Task verifyPdxInstanceEquals complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, verifyPdxInstanceHashcode)
  {
    LOG("verifyPdxInstanceHashcode started ");

    auto rptr = getHelper()->getRegion(regionNames[0]);
    auto keyport1 = CacheableKey::create("javaPdxHC");
    auto keyport2 = CacheableKey::create("pdxput2");
    auto pIPtr1 = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport2));
    int pdxInstHashcode = pIPtr1->hashcode();
    LOG_INFO("pdxinstance hash code = %d ", pdxInstHashcode);

    auto pIPtr2 = std::dynamic_pointer_cast<Cacheable>(rptr->get(keyport1));
    LOG("In verifyPdxInstanceHashcode get done");
    CacheableInt32 *val1 = dynamic_cast<CacheableInt32 *>(pIPtr2.get());
    LOG("In verifyPdxInstanceHashcode cast done");
    int javaPdxHC = val1->value();
    LOG_INFO("javaPdxHC hash code again = %d ", javaPdxHC);

    ASSERT(javaPdxHC == pdxInstHashcode,
           "Pdxhashcode hashcode not matched with java padx hash code.");

    LOG("verifyPdxInstanceHashcode complete");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, accessPdxInstance)
  {
    LOG("accessPdxInstance started ");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      LOG("PdxObject Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("PdxObject IllegalStateException");
    }

    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
      LOG("Address Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("Address IllegalStateException");
    }

    auto keyport = CacheableKey::create("pdxput");
    auto keyport1 = CacheableKey::create("pdxput2");
    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto pIPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG_INFO("PdxInstancePtr created ");

    auto pdxobjPtr = std::make_shared<PdxTests::PdxType>();

    bool bval = pIPtr->getBooleanField("m_bool");
    ASSERT(pdxobjPtr->getBool() == bval, "bool values should be equal");
    ASSERT(pIPtr->getFieldType("m_bool") == PdxFieldTypes::BOOLEAN,
           "Type Value BOOLEAN Mismatch");

    signed char byteVal = pIPtr->getByteField("m_byte");
    ASSERT(pdxobjPtr->getByte() == byteVal, "byte values should be equal");
    ASSERT(pIPtr->getFieldType("m_byte") == PdxFieldTypes::BYTE,
           "Type Value BYTE Mismatch");

    byteVal = pIPtr->getByteField("m_sbyte");
    ASSERT(pdxobjPtr->getSByte() == byteVal, "Sbyte values should be equal");
    ASSERT(pIPtr->getFieldType("m_sbyte") == PdxFieldTypes::BYTE,
           "Type Value BYTE Mismatch");

    int16_t shortVal = pIPtr->getShortField("m_int16");
    ASSERT(pdxobjPtr->getShort() == shortVal, "shortVal should be equal");
    ASSERT(pIPtr->getFieldType("m_int16") == PdxFieldTypes::SHORT,
           "Type Value SHORT Mismatch");

    shortVal = pIPtr->getShortField("m_uint16");
    ASSERT(pdxobjPtr->getUint16() == shortVal, "m_uint16 should be equal");
    ASSERT(pIPtr->getFieldType("m_uint16") == PdxFieldTypes::SHORT,
           "Type Value SHORT Mismatch");

    int val = pIPtr->getIntField("m_int32");
    ASSERT(pdxobjPtr->getInt() == val, "int32 values should be equal");
    ASSERT(pIPtr->getFieldType("m_int32") == PdxFieldTypes::INT,
           "Type Value INT Mismatch");

    val = pIPtr->getIntField("m_uint32");
    ASSERT(pdxobjPtr->getUInt() == val, "m_uint32 values should be equal");
    ASSERT(pIPtr->getFieldType("m_uint32") == PdxFieldTypes::INT,
           "Type Value INT Mismatch");

    int64_t longVal = pIPtr->getLongField("m_long");
    ASSERT(pdxobjPtr->getLong() == longVal, "int64 values should be equal");
    ASSERT(pIPtr->getFieldType("m_long") == PdxFieldTypes::LONG,
           "Type Value LONG Mismatch");

    longVal = pIPtr->getLongField("m_ulong");
    ASSERT(pdxobjPtr->getULong() == longVal, "m_ulong values should be equal");
    ASSERT(pIPtr->getFieldType("m_ulong") == PdxFieldTypes::LONG,
           "Type Value LONG Mismatch");

    float floatVal = pIPtr->getFloatField("m_float");
    ASSERT(pdxobjPtr->getFloat() == floatVal, "floatVal should be equal");
    ASSERT(pIPtr->getFieldType("m_float") == PdxFieldTypes::FLOAT,
           "Type Value FLOAT Mismatch");

    double doubleVal = pIPtr->getDoubleField("m_double");
    ASSERT(pdxobjPtr->getDouble() == doubleVal, "doubleVal should be equal");
    ASSERT(pIPtr->getFieldType("m_double") == PdxFieldTypes::DOUBLE,
           "Type Value DOUBLE Mismatch");

    auto stringVal = pIPtr->getStringField("m_string");
    ASSERT(stringVal == pdxobjPtr->getString(), "stringVal should be equal");
    ASSERT(pIPtr->getFieldType("m_string") == PdxFieldTypes::STRING,
           "Type Value STRING Mismatch");

    auto stringArrayVal = pIPtr->getStringArrayField("m_stringArray");
    ASSERT(pdxobjPtr->getStringArrayLength() ==
               static_cast<int32_t>(stringArrayVal.size()),
           "stringArrayLen should be equal");
    ASSERT(pIPtr->getFieldType("m_stringArray") == PdxFieldTypes::STRING_ARRAY,
           "Type Value STRING_ARRAY Mismatch");
    auto strArray = pdxobjPtr->getStringArray();
    for (size_t i = 0; i < stringArrayVal.size(); i++) {
      ASSERT(strArray[i] == stringArrayVal[i],
             "All stringVals should be equal");
    }

    auto byteArray = pIPtr->getByteArrayField("m_byteArray");
    ASSERT(genericValCompare(pdxobjPtr->getByteArrayLength(),
                             static_cast<int32_t>(byteArray.size())) == true,
           "byteArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getByteArray(), byteArray,
                          byteArray.size()) == true,
           "byteArray should be equal");
    ASSERT(pIPtr->getFieldType("m_byteArray") == PdxFieldTypes::BYTE_ARRAY,
           "Type Value BYTE_ARRAY Mismatch");

    auto charArray = pIPtr->getCharArrayField("m_charArray");
    ASSERT(genericValCompare(pdxobjPtr->getCharArrayLength(),
                             static_cast<int32_t>(charArray.size())) == true,
           "charArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getCharArray(), charArray,
                          charArray.size()) == true,
           "charArray should be equal");
    ASSERT(pIPtr->getFieldType("m_charArray") == PdxFieldTypes::CHAR_ARRAY,
           "Type Value CHAR_ARRAY Mismatch");

    byteArray = pIPtr->getByteArrayField("m_sbyteArray");
    ASSERT(genericValCompare(pdxobjPtr->getByteArrayLength(),
                             static_cast<int32_t>(byteArray.size())) == true,
           "sbyteArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getSByteArray(), byteArray,
                          byteArray.size()) == true,
           "m_sbyteArray should be equal");
    ASSERT(pIPtr->getFieldType("m_sbyteArray") == PdxFieldTypes::BYTE_ARRAY,
           "Type Value BYTE_ARRAY Mismatch");

    auto boolArray = pIPtr->getBooleanArrayField("m_boolArray");
    auto boolArrayLength = boolArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getBoolArrayLength(),
                             static_cast<int32_t>(boolArrayLength)) == true,
           "boolArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getBoolArray(), boolArray,
                          boolArrayLength) == true,
           "boolArray should be equal");
    ASSERT(pIPtr->getFieldType("m_boolArray") == PdxFieldTypes::BOOLEAN_ARRAY,
           "Type Value BOOLEAN_ARRAY Mismatch");

    auto shortArray = pIPtr->getShortArrayField("m_int16Array");
    auto shortArrayLength = shortArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getShortArrayLength(),
                             static_cast<int32_t>(shortArrayLength)) == true,
           "shortArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getShortArray(), shortArray,
                          shortArrayLength) == true,
           "shortArray should be equal");
    ASSERT(pIPtr->getFieldType("m_int16Array") == PdxFieldTypes::SHORT_ARRAY,
           "Type Value SHORT_ARRAY Mismatch");

    shortArray = pIPtr->getShortArrayField("m_uint16Array");
    shortArrayLength = shortArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getShortArrayLength(),
                             static_cast<int32_t>(shortArrayLength)) == true,
           "shortArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getUInt16Array(), shortArray,
                          shortArrayLength) == true,
           "m_uint16Array should be equal");
    ASSERT(pIPtr->getFieldType("m_uint16Array") == PdxFieldTypes::SHORT_ARRAY,
           "Type Value SHORT_ARRAY Mismatch");

    auto intArray = pIPtr->getIntArrayField("m_int32Array");
    auto intArrayLength = intArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getIntArrayLength(),
                             static_cast<int32_t>(intArrayLength)) == true,
           "intArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getIntArray(), intArray, intArrayLength) ==
               true,
           "intArray should be equal");
    ASSERT(pIPtr->getFieldType("m_int32Array") == PdxFieldTypes::INT_ARRAY,
           "Type Value INT_ARRAY Mismatch");

    intArray = pIPtr->getIntArrayField("m_uint32Array");
    ASSERT(genericValCompare(pdxobjPtr->getIntArrayLength(),
                             static_cast<int32_t>(intArrayLength)) == true,
           "intArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getUIntArray(), intArray,
                          intArrayLength) == true,
           "m_uint32Array should be equal");
    ASSERT(pIPtr->getFieldType("m_uint32Array") == PdxFieldTypes::INT_ARRAY,
           "Type Value INT_ARRAY Mismatch");

    auto longArray = pIPtr->getLongArrayField("m_longArray");
    auto longArrayLength = longArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getLongArrayLength(),
                             static_cast<int32_t>(longArrayLength)) == true,
           "longArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getLongArray(), longArray,
                          longArrayLength) == true,
           "longArray should be equal");
    ASSERT(pIPtr->getFieldType("m_longArray") == PdxFieldTypes::LONG_ARRAY,
           "Type Value LONG_ARRAY Mismatch");

    longArray = pIPtr->getLongArrayField("m_ulongArray");
    longArrayLength = longArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getLongArrayLength(),
                             static_cast<int32_t>(longArrayLength)) == true,
           "longArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getULongArray(), longArray,
                          longArrayLength) == true,
           "m_ulongArray should be equal");
    ASSERT(pIPtr->getFieldType("m_ulongArray") == PdxFieldTypes::LONG_ARRAY,
           "Type Value LONG_ARRAY Mismatch");

    auto doubleArray = pIPtr->getDoubleArrayField("m_doubleArray");
    auto doubleArrayLength = doubleArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getDoubleArrayLength(),
                             static_cast<int32_t>(doubleArrayLength)) == true,
           "doubleArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getDoubleArray(), doubleArray,
                          doubleArrayLength) == true,
           "doubleArray should be equal");
    ASSERT(pIPtr->getFieldType("m_doubleArray") == PdxFieldTypes::DOUBLE_ARRAY,
           "Type Value DOUBLE_ARRAY Mismatch");

    auto floatArray = pIPtr->getFloatArrayField("m_floatArray");
    auto floatArrayLength = floatArray.size();
    ASSERT(genericValCompare(pdxobjPtr->getFloatArrayLength(),
                             static_cast<int32_t>(floatArrayLength)) == true,
           "floatArrayLength should be equal");
    ASSERT(genericCompare(pdxobjPtr->getFloatArray(), floatArray,
                          floatArrayLength) == true,
           "floatArray should be equal");
    ASSERT(pIPtr->getFieldType("m_floatArray") == PdxFieldTypes::FLOAT_ARRAY,
           "Type Value FLOAT_ARRAY Mismatch");

    auto object = pIPtr->getCacheableField("m_pdxEnum");
    ASSERT(object != nullptr, "enumObject should not be nullptr");
    auto enumObject = std::dynamic_pointer_cast<CacheableEnum>(object);
    ASSERT(
        enumObject->getEnumOrdinal() == pdxobjPtr->getEnum()->getEnumOrdinal(),
        "enumObject ordinal should be equal");
    ASSERT(enumObject->getEnumClassName() ==
               pdxobjPtr->getEnum()->getEnumClassName(),
           "enumObject classname should be equal");
    ASSERT(enumObject->getEnumName() == pdxobjPtr->getEnum()->getEnumName(),
           "enumObject enumname should be equal");
    ASSERT(pIPtr->getFieldType("m_pdxEnum") == PdxFieldTypes::OBJECT,
           "Type Value OBJECT Mismatch");

    auto dateObject = pIPtr->getCacheableDateField("m_dateTime");
    ASSERT(dateObject != nullptr, "date should not be nullptr");
    ASSERT((*(dateObject.get()) == *(pdxobjPtr->getDate().get())) == true,
           "dateObject should be equal");
    ASSERT(pIPtr->getFieldType("m_dateTime") == PdxFieldTypes::DATE,
           "Type Value DATE Mismatch");

    auto object2 = pIPtr->getCacheableField("m_map");
    ASSERT(object2 != nullptr, "object2 should not be nullptr");
    auto mapObject = std::dynamic_pointer_cast<CacheableHashMap>(object2);
    ASSERT(genericValCompare(pdxobjPtr->getHashMap()->size(),
                             mapObject->size()) == true,
           "mapobject size should be equal");
    ASSERT(pIPtr->getFieldType("m_map") == PdxFieldTypes::OBJECT,
           "Type Value OBJECT Mismatch");

    object2 = pIPtr->getCacheableField("m_vector");
    ASSERT(object2 != nullptr, "object2 should not be nullptr");
    auto vec = std::dynamic_pointer_cast<CacheableVector>(object2);
    ASSERT(
        genericValCompare(pdxobjPtr->getVector()->size(), vec->size()) == true,
        "vec size should be equal");
    ASSERT(pIPtr->getFieldType("m_vector") == PdxFieldTypes::OBJECT,
           "Type Value OBJECT Mismatch");

    object2 = pIPtr->getCacheableField("m_arraylist");
    ASSERT(object2 != nullptr, "object2 should not be nullptr");
    auto arrList = std::dynamic_pointer_cast<CacheableArrayList>(object2);
    ASSERT(genericValCompare(pdxobjPtr->getArrayList()->size(),
                             arrList->size()) == true,
           "arrList size should be equal");
    ASSERT(pIPtr->getFieldType("m_arraylist") == PdxFieldTypes::OBJECT,
           "Type Value OBJECT Mismatch");

    object2 = pIPtr->getCacheableField("m_chs");
    ASSERT(object2 != nullptr, "object2 should not be nullptr");
    auto hashSet = std::dynamic_pointer_cast<CacheableHashSet>(object2);
    ASSERT(genericValCompare(pdxobjPtr->getHashSet()->size(),
                             hashSet->size()) == true,
           "hashSet size should be equal");
    ASSERT(pIPtr->getFieldType("m_chs") == PdxFieldTypes::OBJECT,
           "Type Value OBJECT Mismatch");

    object2 = pIPtr->getCacheableField("m_clhs");
    ASSERT(object2 != nullptr, "object2 should not be nullptr");
    auto linkedHashSet =
        std::dynamic_pointer_cast<CacheableLinkedHashSet>(object2);
    ASSERT(genericValCompare(pdxobjPtr->getLinkedHashSet()->size(),
                             linkedHashSet->size()) == true,
           "linkedHashSet size should be equal");
    ASSERT(pIPtr->getFieldType("m_clhs") == PdxFieldTypes::OBJECT,
           "Type Value OBJECT Mismatch");

    int8_t **byteByteArrayVal = nullptr;
    int32_t byteArrayLen = 0;
    int32_t *elementLength = nullptr;
    pIPtr->getField("m_byteByteArray", &byteByteArrayVal, byteArrayLen,
                    elementLength);
    ASSERT(genericValCompare(pdxobjPtr->getbyteByteArrayLength(),
                             byteArrayLen) == true,
           "byteByteArrayLength should be equal");
    int8_t **bytArray = pdxobjPtr->getArrayOfByteArrays();
    ASSERT(generic2DCompare(byteByteArrayVal, bytArray, byteArrayLen,
                            elementLength) == true,
           "byteByteArray should be equal");
    for (auto i = 0; i < byteArrayLen; i++) {
      delete[] byteByteArrayVal[i];
    }
    delete[] byteByteArrayVal;
    delete[] elementLength;
    ASSERT(pIPtr->getFieldType("m_byteByteArray") ==
               PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS,
           "Type Value ARRAY_OF_BYTE_ARRAYS Mismatch");

    auto objectArray = pIPtr->getCacheableObjectArrayField("m_objectArray");
    ASSERT(objectArray != nullptr, "objectArray should not be nullptr");
    ASSERT(genericValCompare(pdxobjPtr->getCacheableObjectArray()->size(),
                             objectArray->size()) == true,
           "objectArray size should be equal");
    ASSERT(pIPtr->getFieldType("m_objectArray") == PdxFieldTypes::OBJECT_ARRAY,
           "Type Value OBJECT_ARRAY Mismatch");

    auto objectArrayEmptyFieldName = pIPtr->getCacheableObjectArrayField("");
    ASSERT(objectArrayEmptyFieldName != nullptr,
           "objectArrayEmptyFieldName should not be nullptr");
    ASSERT(genericValCompare(
               pdxobjPtr->getCacheableObjectArrayEmptyPdxFieldName()->size(),
               objectArrayEmptyFieldName->size()) == true,
           "objectArrayEmptyFieldName size should be equal");

    for (size_t i = 0; i < objectArray->size(); i++) {
      auto pi = std::dynamic_pointer_cast<PdxInstance>(objectArray->at(i));
      LOG("PdxInstancePtr obtained from CacheableObjectArray");

      auto pdxFactory =
          cacheHelper->getCache()->createPdxInstanceFactory("PdxTests.Address");
      LOG("PdxInstanceFactoryPtr created for PdxTests.Address....");

      pdxFactory.writeInt("_aptNumber", static_cast<int32_t>(i + 1));
      char streetStr[256] = {0};
      sprintf(streetStr, "street%zd", i);
      pdxFactory.writeString("_street", streetStr);
      char cityStr[256] = {0};
      sprintf(cityStr, "city%zd", i);
      pdxFactory.writeString("_city", cityStr);

      auto addrPtr = std::dynamic_pointer_cast<Address>(pi->getObject());
      LOG("AddressPtr created using PdxInstance getObject()....");
      auto newAddrPtr = std::make_shared<Address>(static_cast<int32_t>(i + 1),
                                                  streetStr, cityStr);
      LOG("AddressPtr created using new....");
      ASSERT(addrPtr->equals(*newAddrPtr) == true,
             "Both PdxInstances should be equal.");

      auto retPtr = pdxFactory.create();
      LOG("PdxInstancePtr created....");

      bool ret = false;
      ret = (*pi == *retPtr);
      LOG_INFO("PdxObject ret = %d ", ret);
      ASSERT(ret == true, "Both PdxInstances should be equal.");
    }
    LOG_INFO("PdxInstancePtr for ParentPdx accessPdxInstance ");
    pIPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    LOG_INFO("PdxInstancePtr for ParentPdx object got ");

    auto childObjPtr = pIPtr->getCacheableField("m_childPdx");
    ASSERT(childObjPtr != nullptr, "childObjPtr should not be nullptr");
    LOG_INFO("got childPdx field ");
    auto cpi = std::dynamic_pointer_cast<PdxInstance>(childObjPtr);
    LOG_INFO("cast to pdxinstance done ");
    auto cpo = std::dynamic_pointer_cast<ChildPdx>(cpi->getObject());
    LOG_INFO("got childPdx getObject ");
    auto childpdxobjPtr = std::make_shared<ChildPdx>(1);
    LOG_INFO("created new childPdx");
    ASSERT(cpo->equals(*childpdxobjPtr) == true, "child pdx should be equal");
    LOG("accessPdxInstance complete");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, modifyPdxInstance)
  {
    LOG("modifyPdxInstance started ");

    auto rptr = getHelper()->getRegion(regionNames[0]);
    auto keyport = CacheableKey::create("pdxput");
    auto keyport1 = CacheableKey::create("pdxput2");

    auto pIPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG("modifyPdxInstance get complete.");

    std::shared_ptr<WritablePdxInstance> wpiPtr(pIPtr->createWriter());

    ASSERT(pIPtr != nullptr, "pIPtr != nullptr expected");
    int val = 0;
    int newVal = 0;
    ASSERT(pIPtr->hasField("m_int32") == true, "m_id1 = true expected");
    val = pIPtr->getIntField("m_int32");
    wpiPtr->setField("m_int32", val + 1);
    rptr->put(keyport, wpiPtr);
    auto newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_int32") == true, "m_int32 = true expected");
    newVal = newPiPtr->getIntField("m_int32");
    ASSERT(val + 1 == newVal, "val + 1 == newVal expected");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_int32", false);
      FAIL(
          "setField on m_int32 with bool value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_int32 with bool value caught expected "
          "IllegalStateException");
    }

    bool boolVal = true;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_bool", false);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_bool") == true, "m_bool = true expected");
    boolVal = newPiPtr->getBooleanField("m_bool");
    ASSERT(boolVal == false, "bool is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_bool", 500);
      FAIL(
          "setField on m_bool with int value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_bool with int value caught expected "
          "IllegalStateException");
    }

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_char", 500.0);
      FAIL(
          "setField on m_char with float value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_char with float value caught expected "
          "IllegalStateException");
    }

    signed char byteVal = 0;
    signed char setByteVal = 0x75;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_byte", setByteVal);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_byte") == true, "m_byte = true expected");
    byteVal = newPiPtr->getByteField("m_byte");
    ASSERT(byteVal == setByteVal, "byte is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    signed char setSByteVal = 0x57;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_sbyte", setSByteVal);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_sbyte") == true, "m_sbyte = true expected");
    byteVal = newPiPtr->getByteField("m_sbyte");
    ASSERT(byteVal == setSByteVal, "m_sbyte is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_byte", static_cast<int64_t>(0x56787878));
      FAIL(
          "setField on m_byte with int64_t value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_byte with int64_t value caught expected "
          "IllegalStateException");
    }

    int16_t shortVal = 0;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_int16", static_cast<int16_t>(0x5678));
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_int16") == true, "m_int16 = true expected");
    shortVal = newPiPtr->getShortField("m_int16");
    ASSERT(shortVal == 0x5678, "short is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_int16", static_cast<int64_t>(0x56787878));
      FAIL(
          "setField on m_int16 with int64_t value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_int16 with int64_t value caught expected "
          "IllegalStateException");
    }

    int64_t longVal = 0;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_long", static_cast<int64_t>(0x56787878));
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_long") == true, "m_long = true expected");
    longVal = newPiPtr->getLongField("m_long");
    ASSERT(longVal == 0x56787878, "long is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_long", 18389.34f);
      FAIL(
          "setField on m_long with float value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_long with float value caught expected "
          "IllegalStateException");
    }

    float fVal = 0.0f;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_float", 18389.34f);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_float") == true, "m_float = true expected");
    fVal = newPiPtr->getFloatField("m_float");
    ASSERT(fVal == 18389.34f, "fval is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_float", 'D');
      FAIL(
          "setField on m_float with char value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_float with char value caught expected "
          "IllegalStateException");
    }

    double dVal = 0.0;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_double", 18389.34);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_double") == true, "m_double = true expected");
    dVal = newPiPtr->getDoubleField("m_double");
    ASSERT(dVal == 18389.34, "dval is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_double", 'D');
      FAIL(
          "setField on m_double with char value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_double with char value caught expected "
          "IllegalStateException");
    }

    std::vector<bool> setBoolArray{true, false, true,  false,
                                   true, true,  false, true};
    wpiPtr->setField("m_boolArray", setBoolArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_boolArray") == true,
           "m_boolArray = true expected");
    auto getBoolArray = newPiPtr->getBooleanArrayField("m_boolArray");
    ASSERT(getBoolArray.size() == 8, "Arraylength == 8 expected");
    ASSERT(
        genericCompare(setBoolArray, getBoolArray, getBoolArray.size()) == true,
        "boolArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_boolArray", 'D');
      FAIL(
          "setField on m_boolArray with char value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_boolArray with char value caught expected "
          "IllegalStateException");
    }

    std::vector<int8_t> setByteArray{0x34, 0x64, 0x34, 0x64};
    wpiPtr->setField("m_byteArray", setByteArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_byteArray") == true,
           "m_byteArray = true expected");
    auto getByteArray = newPiPtr->getByteArrayField("m_byteArray");
    ASSERT(getByteArray.size() == 4, "Arraylength == 4 expected");
    ASSERT(
        genericCompare(setByteArray, getByteArray, getByteArray.size()) == true,
        "byteArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_byteArray", "string");
      FAIL(
          "setField on m_byteArray with string value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_byteArray with string value caught expected "
          "IllegalStateException");
    }

    std::vector<char16_t> setCharArray{'c', 'v', 'c', 'v'};
    wpiPtr->setField("m_charArray", setCharArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_charArray") == true,
           "m_charArray = true expected");
    auto getCharArray = newPiPtr->getCharArrayField("m_charArray");
    ASSERT(getCharArray.size() == 4, "Arraylength == 4 expected");
    ASSERT(
        genericCompare(setCharArray, getCharArray, getCharArray.size()) == true,
        "charArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_charArray", "string");
      FAIL(
          "setField on m_charArray with string value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_charArray with string value caught expected "
          "IllegalStateException");
    }

    std::vector<int16_t> setShortArray{0x2332, 0x4545, 0x88, 0x898};
    wpiPtr->setField("m_int16Array", setShortArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_int16Array") == true,
           "m_int16Array = true expected");
    auto getShortArray = newPiPtr->getShortArrayField("m_int16Array");
    ASSERT(getShortArray.size() == 4, "Arraylength == 4 expected");
    ASSERT(genericCompare(setShortArray, getShortArray, getShortArray.size()) ==
               true,
           "shortArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_int16Array", setCharArray);
      FAIL(
          "setField on m_int16Array with setCharArray value should throw "
          "expected IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_int16Array with setCharArray value caught expected "
          "IllegalStateException");
    }

    std::vector<int32_t> setIntArray{23, 676868, 34343};
    wpiPtr->setField("m_int32Array", setIntArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_int32Array") == true,
           "m_int32Array = true expected");
    auto newValArray = newPiPtr->getIntArrayField("m_int32Array");
    ASSERT(newValArray.size() == 3, "Arraylength == 3 expected");
    ASSERT(genericCompare(setIntArray, newValArray, newValArray.size()) == true,
           "intArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_int32Array", setShortArray);
      FAIL(
          "setField on m_int32Array with setShortArray value should throw "
          "expected IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_int32Array with setShortArray value caught expected "
          "IllegalStateException");
    }

    std::vector<int64_t> setLongArray{3245435, 3425435};
    wpiPtr->setField("m_longArray", setLongArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_longArray") == true,
           "m_longArray = true expected");
    auto getLongArray = newPiPtr->getLongArrayField("m_longArray");
    ASSERT(getLongArray.size() == 2, "Arraylength == 2 expected");
    ASSERT(
        genericCompare(setLongArray, getLongArray, getLongArray.size()) == true,
        "longArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_longArray", setIntArray);
      FAIL(
          "setField on m_longArray with setIntArray value should throw "
          "expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_longArray with setIntArray value caught expected "
          "IllegalStateException");
    }

    std::vector<float> setFloatArray{232.565f, 234323354.67f};
    wpiPtr->setField("m_floatArray", setFloatArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_floatArray") == true,
           "m_floatArray = true expected");
    auto getFloatArray = newPiPtr->getFloatArrayField("m_floatArray");
    ASSERT(getFloatArray.size() == 2, "Arraylength == 2 expected");
    ASSERT(genericCompare(setFloatArray, getFloatArray, getFloatArray.size()) ==
               true,
           "floatArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_floatArray", setLongArray);
      FAIL(
          "setField on m_floatArray with setLongArray value should throw "
          "expected IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_floatArray with setLongArray value caught expected "
          "IllegalStateException");
    }

    std::vector<double> setDoubleArray{23423432.00, 43242354315.00};
    wpiPtr->setField("m_doubleArray", setDoubleArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_doubleArray") == true,
           "m_doubleArray = true expected");
    auto getDoubleArray = newPiPtr->getDoubleArrayField("m_doubleArray");
    ASSERT(getDoubleArray.size() == 2, "Arraylength == 2 expected");
    ASSERT(genericCompare(setDoubleArray, getDoubleArray,
                          getDoubleArray.size()) == true,
           "doubleArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_doubleArray", setFloatArray);
      FAIL(
          "setField on m_doubleArray with setFloatArray value should throw "
          "expected IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_doubleArray with setFloatArray value caught expected "
          "IllegalStateException");
    }

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_string", setFloatArray);
      FAIL(
          "setField on m_string with setFloatArray value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_string with setFloatArray value caught expected "
          "IllegalStateException");
    }

    std::string setString = "change the string";
    wpiPtr->setField("m_string", setString);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_string") == true, "m_string = true expected");
    ASSERT(newPiPtr->hasField("m_string") == true, "m_string = true expected");
    auto stringVal = newPiPtr->getStringField("m_string");
    ASSERT(stringVal == setString, "stringVal should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::shared_ptr<CacheableDate> dateVal;
    wpiPtr = pIPtr->createWriter();
    time_t timeofday = 0;
    timeofday =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto datePtr = CacheableDate::create(timeofday);
    wpiPtr->setField("m_dateTime", datePtr);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_dateTime") == true,
           "m_dateTime = true expected");
    ASSERT(pIPtr->hasField("m_dateTime") == true, "m_date = true expected");
    dateVal = newPiPtr->getCacheableDateField("m_dateTime");
    ASSERT((*dateVal == *datePtr) == true, "dateObject should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_dateTime", setString);
      FAIL(
          "setField on m_dateTime with setString value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_dateTime with setString value caught expected "
          "IllegalStateException");
    }

    std::shared_ptr<Cacheable> object;
    wpiPtr = pIPtr->createWriter();
    auto setVec = CacheableVector::create();
    setVec->push_back(CacheableInt32::create(3));
    setVec->push_back(CacheableInt32::create(4));
    wpiPtr->setField("m_vector", setVec);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_vector") == true, "m_vector = true expected");
    ASSERT(pIPtr->hasField("m_vector") == true, "m_vector = true expected");
    object = newPiPtr->getCacheableField("m_vector");
    auto vecVal = std::dynamic_pointer_cast<CacheableVector>(object);
    ASSERT(genericValCompare(setVec->size(), vecVal->size()) == true,
           "vec size should be equal");
    for (size_t j = 0; j < vecVal->size(); j++) {
      genericValCompare(setVec->at(j), vecVal->at(j));
    }

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_vector", setString);
      FAIL(
          "setField on m_vector with setString value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_vector with setString value caught expected "
          "IllegalStateException");
    }

    wpiPtr = pIPtr->createWriter();
    auto setarr = CacheableArrayList::create();
    setarr->push_back(CacheableInt32::create(3));
    setarr->push_back(CacheableInt32::create(4));
    setarr->push_back(CacheableInt32::create(5));
    wpiPtr->setField("m_arraylist", setarr);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_arraylist") == true,
           "m_arraylist = true expected");
    ASSERT(pIPtr->hasField("m_arraylist") == true,
           "m_arraylist = true expected");
    object = newPiPtr->getCacheableField("m_arraylist");
    auto arrVal = std::dynamic_pointer_cast<CacheableArrayList>(object);
    ASSERT(genericValCompare(setarr->size(), arrVal->size()) == true,
           "arrList size should be equal");
    for (size_t j = 0; j < arrVal->size(); j++) {
      genericValCompare(setarr->at(j), arrVal->at(j));
    }

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_arraylist", setFloatArray);
      FAIL(
          "setField on m_arraylist with setFloatArray value should throw "
          "expected IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_arraylist with setFloatArray value caught expected "
          "IllegalStateException");
    }

    wpiPtr = pIPtr->createWriter();
    auto hashset = CacheableHashSet::create();
    hashset->insert(CacheableInt32::create(3));
    hashset->insert(CacheableInt32::create(4));
    hashset->insert(CacheableInt32::create(5));
    wpiPtr->setField("m_chs", hashset);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_chs") == true, "m_chs = true expected");
    ASSERT(pIPtr->hasField("m_chs") == true, "m_chs = true expected");
    object = newPiPtr->getCacheableField("m_chs");
    auto hashsetVal = std::dynamic_pointer_cast<CacheableHashSet>(object);
    ASSERT(genericValCompare(hashset->size(), hashsetVal->size()) == true,
           "m_chs size should be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_chs", 100);
      FAIL(
          "setField on m_chs with int value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_chs with int value caught expected "
          "IllegalStateException");
    }

    wpiPtr = pIPtr->createWriter();
    auto hashmap = CacheableHashMap::create();
    hashmap->emplace(CacheableInt32::create(3), CacheableInt32::create(3));
    hashmap->emplace(CacheableInt32::create(4), CacheableInt32::create(4));
    hashmap->emplace(CacheableInt32::create(5), CacheableInt32::create(5));
    wpiPtr->setField("m_map", hashmap);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_map") == true, "m_map = true expected");
    ASSERT(pIPtr->hasField("m_map") == true, "m_map = true expected");
    object = newPiPtr->getCacheableField("m_map");
    auto hashmapVal = std::dynamic_pointer_cast<CacheableHashMap>(object);
    ASSERT(genericValCompare(hashmap->size(), hashmapVal->size()) == true,
           "m_map size should be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_map", 0.0f);
      FAIL(
          "setField on m_map with float value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_map with float value caught expected "
          "IllegalStateException");
    }

    wpiPtr = pIPtr->createWriter();
    auto linkedhashset = CacheableLinkedHashSet::create();
    linkedhashset->insert(CacheableInt32::create(3));
    linkedhashset->insert(CacheableInt32::create(4));
    linkedhashset->insert(CacheableInt32::create(5));
    wpiPtr->setField("m_clhs", linkedhashset);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(newPiPtr->hasField("m_clhs") == true, "m_clhs = true expected");
    ASSERT(pIPtr->hasField("m_clhs") == true, "m_clhs = true expected");
    object = newPiPtr->getCacheableField("m_clhs");
    auto linkedhashsetVal =
        std::dynamic_pointer_cast<CacheableLinkedHashSet>(object);
    ASSERT(genericValCompare(linkedhashsetVal->size(), linkedhashset->size()) ==
               true,
           "m_clhs size should be equal");

    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_clhs", 'c');
      FAIL(
          "setField on m_clhs with char value should throw expected "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_clhs with char value caught expected "
          "IllegalStateException");
    }

    LOG_INFO("Testing byteByteArray");
    int8_t **getbyteByteArray = nullptr;
    int byteByteArrayLength = 0;
    int32_t *elementLength = nullptr;
    int8_t **setbyteByteArray = new int8_t *[4];
    setbyteByteArray[0] = new int8_t[1];
    setbyteByteArray[1] = new int8_t[2];
    setbyteByteArray[2] = new int8_t[1];
    setbyteByteArray[3] = new int8_t[2];
    setbyteByteArray[0][0] = 0x23;
    setbyteByteArray[1][0] = 0x34;
    setbyteByteArray[1][1] = 0x55;
    setbyteByteArray[2][0] = 0x23;
    setbyteByteArray[3][0] = 0x34;
    setbyteByteArray[3][1] = 0x55;
    wpiPtr = pIPtr->createWriter();
    int lengthArr[4];
    lengthArr[0] = 1;
    lengthArr[1] = 2;
    lengthArr[2] = 1;
    lengthArr[3] = 2;
    wpiPtr->setField("m_byteByteArray", setbyteByteArray, 4, lengthArr);
    LOG_INFO("Testing byteByteArray setField");
    rptr->put(keyport, wpiPtr);
    LOG_INFO("Testing byteByteArray put");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG_INFO("Testing byteByteArray get");
    ASSERT(newPiPtr->hasField("m_byteByteArray") == true,
           "m_byteByteArray = true expected");
    LOG_INFO("Testing byteByteArray hasField");
    ASSERT(pIPtr->hasField("m_byteByteArray") == true,
           "m_byteByteArray = true expected");
    newPiPtr->getField("m_byteByteArray", &getbyteByteArray,
                       byteByteArrayLength, elementLength);
    LOG_INFO("Testing byteByteArray getField");
    ASSERT(genericValCompare(4, byteByteArrayLength) == true,
           "byteByteArrayLength should be equal");
    ASSERT(generic2DCompare(getbyteByteArray, setbyteByteArray,
                            byteByteArrayLength, elementLength) == true,
           "byteByteArray should be equal");
    for (auto i = 0; i < byteByteArrayLength; i++) {
      delete[] getbyteByteArray[i];
      delete[] setbyteByteArray[i];
    }
    delete[] getbyteByteArray;
    delete[] setbyteByteArray;
    delete[] elementLength;
    wpiPtr = pIPtr->createWriter();
    try {
      wpiPtr->setField("m_byteByteArray", linkedhashset);
      FAIL(
          "setField on m_byteByteArray with linkedhashset value should throw "
          "expected IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("setField on m_byteByteArray with linkedhashset value caught "
          "expected "
          "IllegalStateException");
    }

    std::vector<std::string> setStringArray = {"test1", "test2", "test3"};
    wpiPtr->setField("m_stringArray", setStringArray.data(), 3);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    auto getStringArray = newPiPtr->getStringArrayField("m_stringArray");
    ASSERT(getStringArray.size() == 3, "Arraylength == 3 expected");
    for (size_t i = 0; i < getStringArray.size(); i++) {
      LOG_INFO("set string is %s ", setStringArray[i].c_str());
      LOG_INFO("get string is %s ", getStringArray[i].c_str());
      ASSERT(setStringArray[i] == getStringArray[i],
             "All stringVals should be equal");
    }
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    LOG_INFO("PdxInstancePtr for ParentPdx modifyPdxInstance ");
    pIPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    LOG_INFO("PdxInstancePtr for ParentPdx object got ");

    auto childObjPtr = pIPtr->getCacheableField("m_childPdx");
    ASSERT(childObjPtr != nullptr, "childObjPtr should not be nullptr");
    LOG_INFO("got childPdx field ");
    auto cpi = std::dynamic_pointer_cast<PdxInstance>(childObjPtr);
    LOG_INFO("cast to pdxinstance done ");
    wpiPtr = pIPtr->createWriter();
    auto childpdxobjPtr = std::make_shared<ChildPdx>(2);
    LOG_INFO("created new childPdx");
    wpiPtr->setField("m_childPdx", childpdxobjPtr);
    LOG_INFO("childPdx seField done");
    rptr->put(keyport1, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    LOG_INFO("ChildPdx object put get done");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");
    childObjPtr = newPiPtr->getCacheableField("m_childPdx");
    ASSERT(childObjPtr != nullptr, "childObjPtr should not be nullptr");
    LOG_INFO("got childPdx field ");
    auto cpi1 = std::dynamic_pointer_cast<PdxInstance>(childObjPtr);
    ASSERT((*cpi == *cpi1) == false, "PdxInstance should not be equal");

    auto cpo = std::dynamic_pointer_cast<ChildPdx>(cpi1->getObject());
    LOG_INFO("got childPdx getObject ");
    ASSERT(cpo->equals(*childpdxobjPtr) == true, "child pdx should be equal");

    char16_t parentCharSetVal = 'Z';
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_char", parentCharSetVal);
    rptr->put(keyport1, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    ASSERT(newPiPtr->hasField("m_char") == true, "m_char = true expected");
    auto parentCharVal = newPiPtr->getCharField("m_char");
    ASSERT(parentCharVal == parentCharSetVal, "char is not equal");

    std::vector<char16_t> setParentCharArray{'c', 'v', 'c', 'v'};
    wpiPtr->setField("m_charArray", setParentCharArray);
    rptr->put(keyport1, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport1));
    ASSERT(newPiPtr->hasField("m_charArray") == true,
           "m_charArray = true expected");
    auto getParentCharArray = newPiPtr->getCharArrayField("m_charArray");
    ASSERT(getParentCharArray.size() == 4, "Arraylength == 4 expected");
    ASSERT(genericCompare(setParentCharArray, getParentCharArray,
                          getParentCharArray.size()) == true,
           "charArray should be equal");
    LOG("modifyPdxInstance complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, modifyPdxInstanceAndCheckLocally)
  {
    LOG("modifyPdxInstanceAndCheckLocally started ");

    auto rptr = getHelper()->getRegion(regionNames[0]);
    auto keyport = CacheableKey::create("pdxput");

    auto pIPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG("modifyPdxInstanceAndCheckLocally get complete.");

    LOG("Statistics for for (PdxTests.PdxType) PdxInstance ");
    LocalRegion *lregPtr = (dynamic_cast<LocalRegion *>(rptr.get()));

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 0,
           "pdxInstanceDeserialization should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 1,
           "pdxInstanceCreations should be equal to 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    std::shared_ptr<WritablePdxInstance> wpiPtr(pIPtr->createWriter());

    ASSERT(pIPtr != nullptr, "pIPtr != nullptr expected");
    int val = 0;
    int newVal = 0;
    ASSERT(pIPtr->hasField("m_int32") == true, "m_int32 = true expected");
    val = pIPtr->getIntField("m_int32");
    LOG_INFO("PdxInstance val is %d ", val);
    ASSERT(val == 591768540, "val = 591768540 expected");

    wpiPtr->setField("m_int32", val + 1);
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put complete.");
    auto newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    LOG("modifyPdxInstanceAndCheckLocally get complete.");
    ASSERT(newPiPtr->hasField("m_int32") == true, "m_id1 = true expected");
    newVal = newPiPtr->getIntField("m_int32");
    LOG_INFO("PdxInstance newVal is %d ", newVal);
    ASSERT(val + 1 == newVal, "val + 1 == newVal expected");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<bool> setBoolArray{true, false, true,  false,
                                   true, true,  false, true};
    wpiPtr->setField("m_boolArray", setBoolArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_boolArray") == true,
           "m_boolArray = true expected");
    auto getBoolArray = newPiPtr->getBooleanArrayField("m_boolArray");
    auto arrayLen = getBoolArray.size();
    ASSERT(arrayLen == 8, "Arraylength == 8 expected");
    ASSERT(genericCompare(setBoolArray, getBoolArray, arrayLen) == true,
           "boolArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<int8_t> setByteArray{0x34, 0x64, 0x34, 0x64};
    wpiPtr->setField("m_byteArray", setByteArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_byteArray") == true,
           "m_byteArray = true expected");
    auto getByteArray = newPiPtr->getByteArrayField("m_byteArray");
    arrayLen = getByteArray.size();
    ASSERT(arrayLen == 4, "Arraylength == 4 expected");
    ASSERT(genericCompare(setByteArray, getByteArray, arrayLen) == true,
           "byteArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<int16_t> setShortArray{0x2332, 0x4545, 0x88, 0x898};
    wpiPtr->setField("m_int16Array", setShortArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_int16Array") == true,
           "m_int16Array = true expected");
    auto getShortArray = newPiPtr->getShortArrayField("m_int16Array");
    arrayLen = getShortArray.size();
    ASSERT(arrayLen == 4, "Arraylength == 4 expected");
    ASSERT(genericCompare(setShortArray, getShortArray, arrayLen) == true,
           "shortArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<int32_t> setIntArray{23, 676868, 34343};
    wpiPtr->setField("m_int32Array", setIntArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_int32Array") == true,
           "m_int32Array = true expected");
    auto newValArray = newPiPtr->getIntArrayField("m_int32Array");
    arrayLen = newValArray.size();
    ASSERT(arrayLen == 3, "Arraylength == 3 expected");
    ASSERT(genericCompare(setIntArray, newValArray, arrayLen) == true,
           "intArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<int64_t> setLongArray{3245435, 3425435};
    wpiPtr->setField("m_longArray", setLongArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_longArray") == true,
           "m_longArray = true expected");
    auto getLongArray = newPiPtr->getLongArrayField("m_longArray");
    arrayLen = getLongArray.size();
    ASSERT(arrayLen == 2, "Arraylength == 2 expected");
    ASSERT(genericCompare(setLongArray, getLongArray, arrayLen) == true,
           "longArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<float> setFloatArray{232.565f, 234323354.67f};
    wpiPtr->setField("m_floatArray", setFloatArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_floatArray") == true,
           "m_floatArray = true expected");
    auto getFloatArray = newPiPtr->getFloatArrayField("m_floatArray");
    arrayLen = getFloatArray.size();
    ASSERT(arrayLen == 2, "Arraylength == 2 expected");
    ASSERT(genericCompare(setFloatArray, getFloatArray, arrayLen) == true,
           "floatArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::vector<double> setDoubleArray{23423432.00, 43242354315.00};
    wpiPtr->setField("m_doubleArray", setDoubleArray);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_doubleArray") == true,
           "m_doubleArray = true expected");
    auto getDoubleArray = newPiPtr->getDoubleArrayField("m_doubleArray");
    arrayLen = getDoubleArray.size();
    ASSERT(arrayLen == 2, "Arraylength == 2 expected");
    ASSERT(genericCompare(setDoubleArray, getDoubleArray, arrayLen) == true,
           "doubleArray should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    bool boolVal = true;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_bool", false);
    LOG("modifyPdxInstanceAndCheckLocally setField bool done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_bool") == true, "m_bool = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    boolVal = newPiPtr->getBooleanField("m_bool");
    LOG("modifyPdxInstanceAndCheckLocally getField complete.");
    ASSERT(boolVal == false, "bool is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_bool", true);
    LOG("modifyPdxInstanceAndCheckLocally setField bool done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_bool") == true, "m_bool = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    boolVal = newPiPtr->getBooleanField("m_bool");
    LOG("modifyPdxInstanceAndCheckLocally getField complete.");
    ASSERT(boolVal == true, "bool is not equal");
    ASSERT((*pIPtr == *newPiPtr) == true, "PdxInstance should be equal");

    float fVal = 0.0f;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_float", 18389.34f);
    LOG("modifyPdxInstanceAndCheckLocally setField float done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_float") == true, "m_float = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    fVal = newPiPtr->getFloatField("m_float");
    LOG_INFO("modifyPdxInstanceAndCheckLocally getField complete. fval = %f",
             fVal);
    ASSERT(fVal == 18389.34f, "fval is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    double dVal = 0.0;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_double", 18389.34);
    LOG("modifyPdxInstanceAndCheckLocally setField float done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_double") == true, "m_double = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    dVal = newPiPtr->getDoubleField("m_double");
    LOG_INFO("modifyPdxInstanceAndCheckLocally getField complete. fval = %lf",
             dVal);
    ASSERT(dVal == 18389.34, "fval is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    signed char byteVal = 0;
    signed char setSByteVal = 0x74;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_byte", setSByteVal);
    LOG("modifyPdxInstanceAndCheckLocally setField byte done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_byte") == true, "m_byte = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    byteVal = newPiPtr->getByteField("m_byte");
    LOG_INFO("modifyPdxInstanceAndCheckLocally getField complete byteVal = %d ",
             byteVal);
    ASSERT(byteVal == setSByteVal, "byte is not equal");
    ASSERT((*pIPtr == *newPiPtr) == true, "PdxInstance should be equal");

    int16_t shortVal = 0;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_int16", static_cast<int16_t>(0x5678));
    LOG("modifyPdxInstanceAndCheckLocally setField short done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_int16") == true, "m_int16 = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    shortVal = newPiPtr->getShortField("m_int16");
    LOG_INFO(
        "modifyPdxInstanceAndCheckLocally getField complete shortVal = %d ",
        shortVal);
    ASSERT(shortVal == 0x5678, "short is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    int64_t longVal = 0;
    wpiPtr = pIPtr->createWriter();
    wpiPtr->setField("m_long", static_cast<int64_t>(0x56787878));
    LOG("modifyPdxInstanceAndCheckLocally setField short done.");
    rptr->put(keyport, wpiPtr);
    LOG("modifyPdxInstanceAndCheckLocally put again complete.");
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_long") == true, "m_long = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get again complete.");
    longVal = newPiPtr->getLongField("m_long");
    LOG_INFO(
        "modifyPdxInstanceAndCheckLocally getField complete longVal = %ld ",
        longVal);
    ASSERT(longVal == 0x56787878, "long is not equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::string str1 = "change the string";
    wpiPtr->setField("m_string", str1);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_string") == true, "m_string = true expected");
    LOG("modifyPdxInstanceAndCheckLocally get string complete.");
    auto getstringVal = newPiPtr->getStringField("m_string");
    LOG_INFO("modifyPdxInstanceAndCheckLocally getstringVal = %s , str1 = %s ",
             getstringVal.c_str(), str1.c_str());
    ASSERT(getstringVal == str1, "getstringVal should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    std::shared_ptr<CacheableDate> dateVal;
    wpiPtr = pIPtr->createWriter();

    auto timeofday =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto datePtr = CacheableDate::create(timeofday);

    wpiPtr->setField("m_dateTime", datePtr);
    rptr->put(keyport, wpiPtr);
    newPiPtr = std::dynamic_pointer_cast<PdxInstance>(rptr->get(keyport));
    ASSERT(pIPtr->hasField("m_dateTime") == true, "m_date = true expected");
    dateVal = newPiPtr->getCacheableDateField("m_dateTime");
    ASSERT((*(dateVal.get()) == *(datePtr.get())) == true,
           "dateObject should be equal");
    ASSERT((*pIPtr == *newPiPtr) == false, "PdxInstance should not be equal");

    LOG("modifyPdxInstanceAndCheckLocally complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, pdxIFPutGetTest)
  {
    LOG("pdxIFPutGetTest started ");
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    try {
      serializationRegistry->addPdxSerializableType(
          Address::createDeserializable);
      LOG("Address Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("Address IllegalStateException");
    }

    try {
      serializationRegistry->addPdxSerializableType(
          PdxTests::PdxType::createDeserializable);
      LOG("PdxObject Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("PdxObject IllegalStateException");
    }

    try {
      serializationRegistry->addPdxSerializableType(
          ChildPdx::createDeserializable);
      LOG("ChildPdx Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("ChildPdx IllegalStateException");
    }

    try {
      serializationRegistry->addPdxSerializableType(
          ParentPdx::createDeserializable);
      LOG("ParentPdx Registered Successfully....");
    } catch (apache::geode::client::IllegalStateException & /* ex*/) {
      LOG("ParentPdx IllegalStateException");
    }

    auto rptr = getHelper()->getRegion(regionNames[0]);

    auto pdxobj = std::make_shared<PdxTests::PdxType>();

    auto pdxFactory =
        cacheHelper->getCache()->createPdxInstanceFactory("PdxTests.PdxType");
    LOG("PdxInstanceFactoryPtr created....");

    int *lengthArr = new int[2];

    lengthArr[0] = 1;
    lengthArr[1] = 2;

    pdxFactory.writeBoolean("m_bool", pdxobj->getBool());
    try {
      pdxFactory.writeBoolean("m_bool", pdxobj->getBool());
      FAIL(
          "calling writeBoolean on same fieldname should have thrown "
          "IllegalStateException");
    } catch (IllegalStateException &) {
      LOG("Got expected IllegalStateException for m_bool");
    }
    pdxFactory.markIdentityField("m_bool");
    pdxFactory.writeByte("m_byte", pdxobj->getByte());
    pdxFactory.markIdentityField("m_byte");
    pdxFactory.writeShort("m_int16", pdxobj->getShort());
    pdxFactory.markIdentityField("m_int16");
    pdxFactory.writeInt("m_int32", pdxobj->getInt());
    pdxFactory.markIdentityField("m_int32");
    pdxFactory.writeLong("m_long", pdxobj->getLong());
    pdxFactory.markIdentityField("m_long");
    pdxFactory.writeFloat("m_float", pdxobj->getFloat());
    pdxFactory.markIdentityField("m_float");
    pdxFactory.writeDouble("m_double", pdxobj->getDouble());
    pdxFactory.markIdentityField("m_double");
    pdxFactory.writeString("m_string", pdxobj->getString());
    pdxFactory.markIdentityField("m_string");
    pdxFactory.writeDate("m_dateTime", pdxobj->getDate());
    pdxFactory.markIdentityField("m_dateTime");
    pdxFactory.writeBooleanArray("m_boolArray", pdxobj->getBoolArray());
    pdxFactory.markIdentityField("m_boolArray");
    pdxFactory.writeByteArray("m_byteArray", pdxobj->getByteArray());
    pdxFactory.markIdentityField("m_byteArray");
    pdxFactory.writeShortArray("m_int16Array", pdxobj->getShortArray());
    pdxFactory.markIdentityField("m_int16Array");
    pdxFactory.writeIntArray("m_int32Array", pdxobj->getIntArray());
    pdxFactory.markIdentityField("m_int32Array");
    pdxFactory.writeLongArray("m_longArray", pdxobj->getLongArray());
    pdxFactory.markIdentityField("m_longArray");
    pdxFactory.writeFloatArray("m_floatArray", pdxobj->getFloatArray());
    pdxFactory.markIdentityField("m_floatArray");
    pdxFactory.writeDoubleArray("m_doubleArray", pdxobj->getDoubleArray());
    pdxFactory.markIdentityField("m_doubleArray");
    pdxFactory.writeObject("m_map", pdxobj->getHashMap());
    pdxFactory.markIdentityField("m_map");
    pdxFactory.writeStringArray("m_stringArray", pdxobj->getStringArray());
    pdxFactory.markIdentityField("m_stringArray");
    pdxFactory.writeObjectArray("m_objectArray",
                                pdxobj->getCacheableObjectArray());
    pdxFactory.writeObject("m_pdxEnum", pdxobj->getEnum());
    pdxFactory.markIdentityField("m_pdxEnum");
    pdxFactory.writeObject("m_arraylist", pdxobj->getArrayList());
    pdxFactory.markIdentityField("m_arraylist");
    pdxFactory.writeObject("m_linkedlist", pdxobj->getLinkedList());
    pdxFactory.markIdentityField("m_linkedlist");
    pdxFactory.writeObject("m_hashtable", pdxobj->getHashTable());
    pdxFactory.markIdentityField("m_hashtable");
    pdxFactory.writeObject("m_vector", pdxobj->getVector());
    pdxFactory.markIdentityField("m_vector");
    pdxFactory.writeArrayOfByteArrays(
        "m_byteByteArray", pdxobj->getArrayOfByteArrays(), 2, lengthArr);
    pdxFactory.markIdentityField("m_byteByteArray");
    pdxFactory.writeChar("m_char", pdxobj->getChar());
    pdxFactory.markIdentityField("m_char");
    pdxFactory.writeCharArray("m_charArray", pdxobj->getCharArray());
    pdxFactory.markIdentityField("m_charArray");
    pdxFactory.writeObject("m_chs", pdxobj->getHashSet());
    pdxFactory.markIdentityField("m_chs");
    pdxFactory.writeObject("m_clhs", pdxobj->getLinkedHashSet());
    pdxFactory.markIdentityField("m_clhs");
    pdxFactory.writeByte("m_sbyte", pdxobj->getSByte());
    pdxFactory.markIdentityField("m_sbyte");
    pdxFactory.writeByteArray("m_sbyteArray", pdxobj->getSByteArray());
    pdxFactory.markIdentityField("m_sbyteArray");
    pdxFactory.writeShort("m_uint16", pdxobj->getUint16());
    pdxFactory.markIdentityField("m_uint16");
    pdxFactory.writeInt("m_uint32", pdxobj->getUInt());
    pdxFactory.markIdentityField("m_uint32");
    pdxFactory.writeLong("m_ulong", pdxobj->getULong());
    pdxFactory.markIdentityField("m_ulong");
    pdxFactory.writeShortArray("m_uint16Array", pdxobj->getUInt16Array());
    pdxFactory.markIdentityField("m_uint16Array");
    pdxFactory.writeIntArray("m_uint32Array", pdxobj->getUIntArray());
    pdxFactory.markIdentityField("m_uint32Array");
    pdxFactory.writeLongArray("m_ulongArray", pdxobj->getULongArray());
    pdxFactory.markIdentityField("m_ulongArray");

    pdxFactory.writeByteArray("m_byte252", pdxobj->getByte252());
    pdxFactory.markIdentityField("m_byte252");
    pdxFactory.writeByteArray("m_byte253", pdxobj->getByte253());
    pdxFactory.markIdentityField("m_byte253");
    pdxFactory.writeByteArray("m_byte65535", pdxobj->getByte65535());
    pdxFactory.markIdentityField("m_byte65535");
    pdxFactory.writeByteArray("m_byte65536", pdxobj->getByte65536());
    pdxFactory.markIdentityField("m_byte65536");
    pdxFactory.writeObject("m_address", pdxobj->getCacheableObjectArray());

    pdxFactory.writeObjectArray(
        "", pdxobj->getCacheableObjectArrayEmptyPdxFieldName());

    std::shared_ptr<PdxInstance> ret = pdxFactory.create();
    LOG("PdxInstancePtr created....");

    ASSERT(ret->getClassName() == "PdxTests.PdxType",
           "pdxInstance.getClassName should return PdxTests.PdxType.");

    auto psPtr = ret->getObject();
    LOG("getObject created....");

    LOG("Statistics for for (PdxTests.PdxType) PdxInstance ");
    auto &&lregPtr = std::dynamic_pointer_cast<LocalRegion>(rptr);

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 1,
           "pdxInstanceDeserialization should be equal to 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 0,
           "pdxInstanceCreations should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() == 0,
           "pdxInstanceDeserializationTime should be equal to 0.");

    PdxTests::PdxType *obj2 = pdxobj.get();
    PdxTests::PdxType *obj1 = dynamic_cast<PdxTests::PdxType *>(psPtr.get());

    LOG_INFO("Avinash Equal Starts");
    ASSERT(obj1->equals(*obj2, false) == true, "PdxObjects should be equal.");
    LOG_INFO("Avinash Equal Finish");

    auto key = CacheableKey::create("pi");
    rptr->put(key, ret);
    LOG("put done....");

    auto newPiPtr = std::dynamic_pointer_cast<PdxSerializable>(rptr->get(key));
    LOG("get done....");

    LOG_INFO(
        "pdxInstanceDeserializations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (PdxTests.PdxType) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(PdxTests.PdxType) PdxInstance  = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 1,
           "pdxInstanceDeserialization should be equal to 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 0,
           "pdxInstanceCreations should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    PdxTests::PdxType *obj3 = dynamic_cast<PdxTests::PdxType *>(newPiPtr.get());

    ASSERT(obj2->equals(*obj3, false) == true, "PdxObjects should be equal.");

    int pdxInstHashcode = ret->hashcode();
    LOG_INFO("pdxinstance hash code = %d ", pdxInstHashcode);

    auto javaPdxHCKey = CacheableKey::create("javaPdxHC");
    auto pIPtr2 = std::dynamic_pointer_cast<Cacheable>(rptr->get(javaPdxHCKey));
    LOG("In pdxIFPutGetTest get done");
    CacheableInt32 *val = dynamic_cast<CacheableInt32 *>(pIPtr2.get());
    LOG("In pdxIFPutGetTest cast done");
    int javaPdxHC = val->value();
    LOG_INFO("javaPdxHC hash code = %d ", javaPdxHC);

    ASSERT(javaPdxHC == pdxInstHashcode,
           "Pdxhashcode hashcode not matched with java pdx hash code.");

    auto pp = std::make_shared<ParentPdx>(10);
    auto pdxFactory2 = cacheHelper->getCache()->createPdxInstanceFactory(
        "testobject.ParentPdx");
    pdxFactory2.writeInt("m_parentId", pp->getParentId());
    pdxFactory2.writeObject("m_enum", pp->getEnum());
    pdxFactory2.writeString("m_parentName", pp->getParentName());
    pdxFactory2.writeObject("m_childPdx", pp->getChildPdx());
    pdxFactory2.writeChar("m_char", pp->getChar());
    pdxFactory2.writeChar("m_wideChar", pp->getChar());
    pdxFactory2.writeCharArray("m_charArray", pp->getCharArray());

    LOG("write set done....");
    std::shared_ptr<PdxInstance> ip2 = pdxFactory2.create();
    LOG("PdxInstancePtr created");

    LOG_INFO("PdxInstance getClassName = " + ip2->getClassName());
    ASSERT(ip2->getClassName() == "testobject.ParentPdx",
           "pdxInstance.getClassName should return testobject.ParentPdx.");

    auto keyport = CacheableKey::create("pp");
    rptr->put(keyport, ip2);
    LOG("put done....");

    newPiPtr = std::dynamic_pointer_cast<PdxSerializable>(rptr->get(keyport));
    LOG("get done....");

    LOG_INFO(
        "pdxInstanceDeserializations for (testobject::ParentPdx) PdxInstance = "
        "%d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializations());
    LOG_INFO(
        "pdxInstanceCreations for (testobject::ParentPdx) PdxInstance  = %d ",
        lregPtr->getCacheImpl()->getCachePerfStats().getPdxInstanceCreations());
    LOG_INFO(
        "pdxInstanceDeserializationTime for(testobject::ParentPdx) PdxInstance "
        "= %d ",
        lregPtr->getCacheImpl()
            ->getCachePerfStats()
            .getPdxInstanceDeserializationTime());

    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializations() == 1,
           "pdxInstanceDeserialization should be equal to 1.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceCreations() == 0,
           "pdxInstanceCreations should be equal to 0.");
    ASSERT(lregPtr->getCacheImpl()
                   ->getCachePerfStats()
                   .getPdxInstanceDeserializationTime() > 0,
           "pdxInstanceDeserializationTime should be greater than 0.");

    auto pp1 = std::dynamic_pointer_cast<ParentPdx>(newPiPtr);
    LOG("got std::shared_ptr<ParentPdx>....");

    ParentPdx *rawPP1 = dynamic_cast<ParentPdx *>(pp1.get());
    LOG("got rawPP1....");

    ParentPdx *rawPP2 = dynamic_cast<ParentPdx *>(pp.get());
    LOG("got rawpp2....");

    ASSERT(rawPP1->equals(*rawPP2, false) == true,
           "ParentPdx objects should be equal.");

    LOG("pdxIFPutGetTest complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, pdxInstanceWithEmptyKeys)
  {
    LOG("pdxInstanceWithEmptyKeys started ");
    auto rptr = getHelper()->getRegion(regionNames[0]);
    auto pdxFactory =
        cacheHelper->getCache()->createPdxInstanceFactory("EMPTY_KEY_NAME");
    LOG("PdxInstanceFactoryPtr created....");

    bool falseValue = false;
    pdxFactory.writeBoolean("", falseValue);
    std::shared_ptr<PdxInstance> putValue = pdxFactory.create();

    auto key = CacheableKey::create("pp");
    rptr->put(key, putValue);
    LOG("put done with boolean Value and Empty Key....");

    auto getValue = std::dynamic_pointer_cast<PdxInstance>(rptr->get(key));
    LOG("get done with boolean Value and Empty Key....");

    ASSERT(*putValue == *getValue,
           "Boolean Value Did not match in case of Empty PdxField Key");
    bool fieldValue = getValue->getBooleanField("");
    ASSERT(fieldValue == falseValue,
           "Mismatch in the PdxInstance with Empty key");

    LOG("pdxInstanceWithEmptyKeys complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseLocator1)
  {
    // stop locator
    if (isLocator) {
      CacheHelper::closeLocator(1);
      LOG("Locator1 stopped");
    }
  }
END_TASK_DEFINITION

void testPdxInstance() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(StepOne_Pooled_Locator);
  CALL_TASK(StepTwo_Pooled_Locator);

  CALL_TASK(pdxIFPutGetTest);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

void testPdxInstanceWithPdxReadSerializedAndCaching() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(StepOne_Pooled_Locator_PdxReadSerialized);
  CALL_TASK(StepTwo_Pooled_Locator_CachingEnabled_PdxReadSerialized);

  CALL_TASK(pdxPut);
  CALL_TASK(modifyPdxInstanceAndCheckLocally);
  CALL_TASK(pdxInstanceWithEmptyKeys);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

void testPdxInstanceWithPdxReadSerialized() {
  CALL_TASK(CreateLocator1);
  CALL_TASK(CreateServer1_With_Locator);

  CALL_TASK(StepOne_Pooled_Locator_PdxReadSerialized);
  CALL_TASK(StepTwo_Pooled_Locator_PdxReadSerialized);

  CALL_TASK(pdxPut);
  CALL_TASK(getObject);
  CALL_TASK(verifyPdxInstanceEquals);
  CALL_TASK(verifyPdxInstanceHashcode);
  CALL_TASK(accessPdxInstance);
  CALL_TASK(modifyPdxInstance);

  CALL_TASK(CloseCache1);
  CALL_TASK(CloseCache2);
  CALL_TASK(CloseServer1);

  CALL_TASK(CloseLocator1);
}

DUNIT_MAIN
  {
    //    testPdxInstance();

    //    testPdxInstanceWithPdxReadSerializedAndCaching();

    testPdxInstanceWithPdxReadSerialized();
  }
END_MAIN
