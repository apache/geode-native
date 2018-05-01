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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <memory>

#include <geode/internal/geode_base.hpp>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#include "SerializationRegistry.hpp"
#include <geode/CacheableString.hpp>
#include <geode/GeodeTypeIds.hpp>
#include "GeodeTypeIdsImpl.hpp"

// Use to init lib when testing components.
#include <CppCacheLibrary.hpp>

#include "CacheHelper.hpp"

using namespace apache::geode::client;

#include "locator_globals.hpp"

int32_t g_classIdToReturn = 0x04;
int32_t g_classIdToReturn2 = 0x1234;
int32_t g_classIdToReturn4 = 0x123456;

template <class T>
std::shared_ptr<T> duplicate(const std::shared_ptr<T>& orig) {
  std::shared_ptr<T> result;
  auto dout = getHelper()->getCache()->createDataOutput();
  dout.writeObject(orig);

  size_t length = 0;
  const uint8_t* buffer = dout.getBuffer(&length);
  auto din = getHelper()->getCache()->createDataInput(buffer, length);
  din.readObject(result);

  return result;
}

struct CData {
  int a;
  bool b;
  char c;
  double d;
  uint64_t e;
};

class OtherType : public Serializable {
 public:
  CData m_struct;
  int32_t m_classIdToReturn;

  explicit OtherType(int32_t classIdToReturn = g_classIdToReturn)
      : m_classIdToReturn(classIdToReturn) {
    m_struct.a = 0;
    m_struct.b = 0;
    m_struct.c = 0;
    m_struct.d = 0;
  }

  void toData(DataOutput& output) const override {
    output.writeBytes((uint8_t*)&m_struct, sizeof(CData));
    output.writeInt(m_classIdToReturn);
  }

  size_t objectSize() const override { return sizeof(CData); }

  void fromData(DataInput& input) override {
    int32_t size = input.readArrayLength();
    input.readBytesOnly(reinterpret_cast<uint8_t*>(&m_struct), size);
    m_classIdToReturn = input.readInt32();
  }

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<OtherType>(g_classIdToReturn);
  }

  static std::shared_ptr<Serializable> createDeserializable2() {
    return std::make_shared<OtherType>(g_classIdToReturn2);
  }

  static std::shared_ptr<Serializable> createDeserializable4() {
    return std::make_shared<OtherType>(g_classIdToReturn4);
  }

  int32_t classId() const override { return m_classIdToReturn; }

  uint32_t size() const { return sizeof(CData); }

  static std::shared_ptr<Cacheable> uniqueCT(
      int32_t i, int32_t classIdToReturn = g_classIdToReturn) {
    auto ot = std::make_shared<OtherType>(classIdToReturn);
    ot->m_struct.a = (int)i;
    ot->m_struct.b = (i % 2 == 0) ? true : false;
    ot->m_struct.c = static_cast<char>(65) + i;
    ot->m_struct.d = ((2.0) * static_cast<double>(i));

    printf("Created OtherType: %d, %s, %c, %e\n", ot->m_struct.a,
           ot->m_struct.b ? "true" : "false", ot->m_struct.c, ot->m_struct.d);

    printf("double hex 0x%016" PRIX64 "\n", ot->m_struct.e);

    return ot;
  }

  static void validateCT(int32_t i, const std::shared_ptr<Cacheable> otPtr) {
    char logmsg[1000];
    sprintf(logmsg, "validateCT for %d", i);
    LOG(logmsg);
    XASSERT(otPtr != nullptr);
    auto ot = std::static_pointer_cast<OtherType>(otPtr);
    XASSERT(ot != nullptr);

    printf("Validating OtherType: %d, %s, %c, %e\n", ot->m_struct.a,
           ot->m_struct.b ? "true" : "false", ot->m_struct.c, ot->m_struct.d);

    printf("double hex 0x%016" PRIX64 "\n", ot->m_struct.e);

    XASSERT(ot->m_struct.a == (int)i);
    XASSERT(ot->m_struct.b == ((i % 2 == 0) ? true : false));
    XASSERT(ot->m_struct.c == (char)65 + i);
    XASSERT((ot->m_struct.d == (((double)2.0) * (double)i)));
  }
};

#define Sender s1p1
#define Receiver s1p2
std::shared_ptr<Region> regionPtr;

DUNIT_TASK(Receiver, SetupR)
  {
    CacheHelper::initLocator(1);
    CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                            locatorsG);
    LOG("SERVER started");
  }
ENDTASK

DUNIT_TASK(Sender, SetupAndPutInts)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    auto serializationRegistry =
        CacheRegionHelper::getCacheImpl(cacheHelper->getCache().get())
            ->getSerializationRegistry();
    serializationRegistry->addType(OtherType::createDeserializable);
    serializationRegistry->addType(OtherType::createDeserializable2);
    serializationRegistry->addType(OtherType::createDeserializable4);

    getHelper()->createPooledRegion("DistRegionAck", USE_ACK, locatorsG,
                                    "__TEST_POOL1__", true, true);
    LOG("SenderInit complete.");

    regionPtr = getHelper()->getRegion("DistRegionAck");
    for (int32_t i = 0; i < 10; i++) {
      regionPtr->put(i, CacheableInt32::create(i));
    }
  }
ENDTASK

DUNIT_TASK(Sender, SendCT)
  {
    for (int32_t i = 0; i < 30; i += 3) {
      try {
        regionPtr->put(i, OtherType::uniqueCT(i, g_classIdToReturn));
        regionPtr->put(i + 1, OtherType::uniqueCT(i + 1, g_classIdToReturn2));
        regionPtr->put(i + 2, OtherType::uniqueCT(i + 2, g_classIdToReturn4));
      } catch (const apache::geode::client::TimeoutException&) {
      }
    }
  }
ENDTASK

DUNIT_TASK(Sender, RValidateCT)
  {
    for (int32_t i = 0; i < 30; i += 3) {
      OtherType::validateCT(i, regionPtr->get(i));
      OtherType::validateCT(i + 1, regionPtr->get(i + 1));
      OtherType::validateCT(i + 2, regionPtr->get(i + 2));
    }
  }
ENDTASK

DUNIT_TASK(Receiver, CloseCacheR)
  {
    CacheHelper::closeServer(1);
    CacheHelper::closeLocator(1);
    LOG("SERVER closed");
  }
ENDTASK

DUNIT_TASK(Sender, CloseCacheS)
  {
    regionPtr = nullptr;
    cleanProc();
  }
ENDTASK
