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
  return dunit::globals()->getIntValue("test_alive_workers");
}

// while this itself isn't thread/process safe, there shouldn't be concurrency
// in a dunit test anyway.
void incrementWorkerTest() {
  dunit::globals()->rebind("test_alive_workers", getWorkerTest() + 1);
}

DUNIT_TASK(s1p1, One)
  {
    dunit::globals()->rebind("from1", 100);
    LOG("bound from1 = 100");
    incrementWorkerTest();
  }
END_TASK(One)

DUNIT_TASK(s1p2, Two)
  {
    ASSERT(dunit::globals()->getIntValue("from1") == 100, "expected 100");
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
    dunit::globals()->dump();
  }
END_TASK(TestA)
