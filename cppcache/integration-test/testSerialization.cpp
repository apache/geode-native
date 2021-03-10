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

#include <cinttypes>
#include <memory>

#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#include "SerializationRegistry.hpp"
#include <geode/CacheableString.hpp>

// Use to init lib when testing components.
#include <CppCacheLibrary.hpp>

#include "CacheHelper.hpp"

#include "locator_globals.hpp"

using apache::geode::client::Cacheable;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;

int32_t g_classIdToReturn = 0x04;

template <class T>
std::shared_ptr<T> duplicate(const std::shared_ptr<T> &orig) {
  std::shared_ptr<T> result;
  auto dout = getHelper()->getCache()->createDataOutput();
  dout.writeObject(orig);

  size_t length = 0;
  auto &&buffer = dout.getBuffer(&length);
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

class OtherType : public DataSerializable {
 public:
  CData m_struct;
  int32_t m_classIdToReturn;

  OtherType() {
    m_struct.a = 0;
    m_struct.b = 0;
    m_struct.c = 0;
    m_struct.d = 0;
  }

  void toData(DataOutput &output) const override {
    // TODO: refactor - this insane
    output.writeBytes(reinterpret_cast<const uint8_t *>(&m_struct),
                      sizeof(CData));
    output.writeInt(m_classIdToReturn);
  }

  size_t objectSize() const override { return sizeof(CData); }

  void fromData(DataInput &input) override {
    int32_t size = input.readArrayLength();
    input.readBytesOnly(reinterpret_cast<uint8_t *>(&m_struct), size);
    m_classIdToReturn = input.readInt32();
  }

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<OtherType>();
  }

  uint32_t size() const { return sizeof(CData); }

  static std::shared_ptr<Cacheable> uniqueCT(int32_t i) {
    auto ot = std::make_shared<OtherType>();
    ot->m_struct.a = static_cast<int>(i);
    ot->m_struct.b = (i % 2 == 0) ? true : false;
    ot->m_struct.c = static_cast<char>(65) + i;
    ot->m_struct.d = ((2.0) * static_cast<double>(i));
    ot->m_struct.e = (static_cast<uint64_t>(i) << 32) + i;

    printf("Created OtherType: %d, %s, %c, %e\n", ot->m_struct.a,
           ot->m_struct.b ? "true" : "false", ot->m_struct.c, ot->m_struct.d);

    printf("double hex 0x%016" PRIX64 "\n", ot->m_struct.e);

    return std::move(ot);
  }

  static void validateCT(int32_t i, const std::shared_ptr<Cacheable> otPtr) {
    char logmsg[1000];
    sprintf(logmsg, "validateCT for %d", i);
    LOG(logmsg);
    XASSERT(otPtr != nullptr);
    auto ot = std::dynamic_pointer_cast<OtherType>(otPtr);
    XASSERT(ot != nullptr);

    printf("Validating OtherType: %d, %s, %c, %e\n", ot->m_struct.a,
           ot->m_struct.b ? "true" : "false", ot->m_struct.c, ot->m_struct.d);

    printf("double hex 0x%016" PRIX64 "\n", ot->m_struct.e);

    XASSERT(ot->m_struct.a == static_cast<int>(i));
    XASSERT(ot->m_struct.b == ((i % 2 == 0) ? true : false));
    XASSERT(ot->m_struct.c == static_cast<char>(65) + i);
    XASSERT((ot->m_struct.d ==
             ((static_cast<double>(2.0)) * static_cast<double>(i))));
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
    serializationRegistry->addDataSerializableType(
        OtherType::createDeserializable, g_classIdToReturn);

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
        regionPtr->put(i, OtherType::uniqueCT(i));
      } catch (const apache::geode::client::TimeoutException &) {
      }
    }
  }
ENDTASK

DUNIT_TASK(Sender, RValidateCT)
  {
    for (int32_t i = 0; i < 30; i += 3) {
      OtherType::validateCT(i, regionPtr->get(i));
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
