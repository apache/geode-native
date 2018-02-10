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

#define ROOT_NAME "testThinClientCacheablesLimits"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "fw_dunit.hpp"
#include "BuiltinCacheableWrappers.hpp"

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include <cstring>


#include "CacheHelper.hpp"
#include "ThinClientHelper.hpp"

using namespace apache::geode::client;
using namespace test;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

static bool isLocator = false;
static bool isLocalServer = true;
static int numberOfLocators = 1;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
#include "LocatorHelper.hpp"

static int keyArr[] = {123,   125,   126,   127,   128,   129,   250,   251,
                       252,   253,   254,   255,   256,   257,   16380, 16381,
                       16382, 16383, 16384, 16385, 16386, 32765, 32766, 32767,
                       32768, 32769, 65533, 65534, 65535, 65536, 65537};
static char charArray[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B',
    'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '_', '-', '+'};
#define KEY_BYTE "key_byte"
#define KEY_EMPTY_BYTESARR "key_empty_bytes_array"
#define KEY_STRING "key_string"
#define FAIL_MESSAGE                                               \
  "Byte sent and received at boundAry condition are not same for " \
  "CacheableBytes or CacheableString for item %d"

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr = getHelper()->createRegion(name, ackMode, false, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
uint8_t* createRandByteArray(int size) {
  uint8_t* ptr = new uint8_t[size];
  for (int i = 0; i < size; i++) {
    ptr[i] = (rand()) % 256;
  }
  return ptr;
}
char* createRandCharArray(int size) {
  char* ch;
  ch = (char *) std::malloc((size + 1) * sizeof(char));
  if (ch == nullptr) {
    throw OutOfMemoryException(
        "Out of Memory while resizing buffer");
  }
  ch[size] = '\0';
  int length = sizeof(charArray) / sizeof(char);
  for (int i = 0; i < size; i++) {
    ch[i] = charArray[(rand()) % (length)];
  }
  return ch;
}
const char* _regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

DUNIT_TASK_DEFINITION(CLIENT1, StepOne)
  {
    initClientWithPool(true, "__TEST_POOL1__", locatorsG, nullptr, nullptr, 0,
                       true);
    getHelper()->createPooledRegion(_regionNames[1], NO_ACK, locatorsG,
                                    "__TEST_POOL1__", false, false);
    LOG("StepOne complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, PutsTask)
  {
    int sizeArray = sizeof(keyArr) / sizeof(int);
    auto verifyReg = getHelper()->getRegion(_regionNames[1]);
    for (int count = 0; count < sizeArray; count++) {
      uint8_t* ptr = createRandByteArray(keyArr[count]);
      char* ptrChar = createRandCharArray(keyArr[count]);

      auto emptyBytesArr = CacheableBytes::create();
      auto bytePtrSent = CacheableBytes::create(std::vector<int8_t>(ptr, ptr + keyArr[count]));
      auto stringPtrSent =
          CacheableString::create(std::string(ptrChar, keyArr[count]));

      verifyReg->put(KEY_BYTE, bytePtrSent);
      verifyReg->put(KEY_STRING, stringPtrSent);
      verifyReg->put(KEY_EMPTY_BYTESARR, emptyBytesArr);

      char msgAssert1[100];
      char msgAssert2[100];

      sprintf(msgAssert1, "Contains Key byte failed for item %d ", count);
      sprintf(msgAssert2, "Contains Key String failed for item %d ", count);

      ASSERT(!verifyReg->containsKey(KEY_BYTE), msgAssert1);
      ASSERT(!verifyReg->containsKey(KEY_STRING), msgAssert2);
      ASSERT(!verifyReg->containsKey(KEY_EMPTY_BYTESARR),
             "Contains key failed for empty bytes array");
      ASSERT(!verifyReg->containsValueForKey(KEY_EMPTY_BYTESARR),
             "Contains value key failed for empty bytes array");

      auto bytePtrReturn =
          std::dynamic_pointer_cast<CacheableBytes>(verifyReg->get(KEY_BYTE));
      auto stringPtrReturn = std::dynamic_pointer_cast<CacheableString>(
          verifyReg->get(KEY_STRING));
      auto emptyBytesArrReturn = std::dynamic_pointer_cast<CacheableBytes>(
          verifyReg->get(KEY_EMPTY_BYTESARR));

      ASSERT(bytePtrReturn != nullptr, "Byte val is nullptr");
      ASSERT(stringPtrReturn != nullptr, "String val is nullptr");
      ASSERT(emptyBytesArrReturn != nullptr,
             "Empty Bytes Array ptr is nullptr");

      bool isSameBytes = (bytePtrReturn->length() == bytePtrSent->length() &&
                          !memcmp(bytePtrReturn->value().data(), bytePtrSent->value().data(),
                                  bytePtrReturn->length()));
      bool isSameString =
          (stringPtrReturn->length() == stringPtrSent->length() &&
           !memcmp(stringPtrReturn->value().c_str(),
                   stringPtrSent->value().c_str(), stringPtrReturn->length()));
      if (isSameBytes && isSameString) {
        char logMSG[100];
        sprintf(logMSG, "Compare %d Passed for length %d", count,
                keyArr[count]);
        LOG(logMSG);
      }
      char failmsg[250];
      sprintf(failmsg, FAIL_MESSAGE, count);
      ASSERT((isSameBytes && isSameString), failmsg);

      ASSERT(emptyBytesArrReturn->length() == 0,
             "Empty Bytes Array  length is not 0.");

      verifyReg->put(KEY_EMPTY_BYTESARR,
                     emptyBytesArr);  // put the empty byte array second time
      ASSERT(!verifyReg->containsKey(KEY_EMPTY_BYTESARR),
             "Contains key failed for empty bytes array");
      ASSERT(!verifyReg->containsValueForKey(KEY_EMPTY_BYTESARR),
             "Contains value key failed for empty bytes array");

      auto emptyBytesArrReturn1 = std::dynamic_pointer_cast<CacheableBytes>(
          verifyReg->get(KEY_EMPTY_BYTESARR));
      ASSERT(emptyBytesArrReturn1 != nullptr,
             "Empty Bytes Array ptr is nullptr");
      ASSERT(emptyBytesArrReturn1->length() == 0,
             "Empty Bytes Array  length is not 0.");
    }
    LOG("Doing clean exit");
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

DUNIT_MAIN
  {
    // CacheableHelper::registerBuiltins();
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator);
    CALL_TASK(StepOne);
    CALL_TASK(PutsTask);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseServer1);
    CALL_TASK(CloseLocator1);
  }
END_MAIN
