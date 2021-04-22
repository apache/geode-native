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

#include <iostream>

#include "fw_dunit.hpp"

int getWorkerTest() {
  auto expected = dunit::globals().find<int>("test_alive_workers").first;
  ASSERT(expected != nullptr, "test_alive_workers is nullptr");
  return *expected;
}

// while this itself isn't thread/process safe, there shouldn't be concurrency
// in a dunit test anyway.
void incrementWorkerTest() {
  auto value = dunit::globals().find<int>("test_alive_workers").first;
  ASSERT(value != nullptr, "test_alive_workers is nullptr");
  ++*value;
}

DUNIT_TASK(s1p1, One)
  {
    dunit::globals().construct<int>("test_alive_workers")(0);
    dunit::globals().construct<int>("from1")(100);
    LOG("bound from1 = 100");
    incrementWorkerTest();
  }
END_TASK(One)

DUNIT_TASK(s1p2, Two)
  {
    auto expected = dunit::globals().find<int>("from1").first;
    ASSERT(expected != nullptr, "from1 is nullptr");
    ASSERT(*expected == 100, "expected 100");
    LOG("looked up from1, found 100");
    incrementWorkerTest();
  }
END_TASK(Two)

DUNIT_TASK(s2p1, Three)
  { incrementWorkerTest(); }
END_TASK(Three)

DUNIT_TASK(s2p2, Four)
  { incrementWorkerTest(); }
END_TASK(Four)

// Now test that none of the workers are dead after executing their first
// task.

DUNIT_TASK(s1p1, Test1)
  { incrementWorkerTest(); }
END_TASK(Test1)

DUNIT_TASK(s1p2, Test2)
  { incrementWorkerTest(); }
END_TASK(Test2)

DUNIT_TASK(s2p1, Test3)
  { incrementWorkerTest(); }
END_TASK(Test3)

DUNIT_TASK(s2p2, Test4)
  { incrementWorkerTest(); }
END_TASK(Test4)

DUNIT_TASK(s1p1, TestA)
  {
    std::cout << "WorkerTest = " << getWorkerTest() << std::endl;
    ASSERT(getWorkerTest() == 8,
           "a previous worker must have failed undetected.");
  }
END_TASK(TestA)
