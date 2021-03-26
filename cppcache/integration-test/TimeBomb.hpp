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

#ifndef GEODE_INTEGRATION_TEST_TIMEBOMB_H_
#define GEODE_INTEGRATION_TEST_TIMEBOMB_H_

#include <ace/Task.h>
#include <assert.h>

#include <thread>

#include "Utils.hpp"

#define MAX_CLIENT 10

class ClientCleanup {
 private:
  void (*m_cleanupCallback[MAX_CLIENT])();
  int m_numberOfClient;

 public:
  ClientCleanup() : m_numberOfClient(0) {}

  void callClientCleanup() {
    printf("callClientCleanup ... %d \n", m_numberOfClient);
    for (int i = 0; i < m_numberOfClient; i++) {
      try {
        m_cleanupCallback[i]();
      } catch (...) {
      }
    }
  }

  bool registerCallback(void (*cleanupFunc)()) {
    if (m_numberOfClient < MAX_CLIENT) {
      m_cleanupCallback[m_numberOfClient++] = cleanupFunc;
      return true;
    }
    return false;
  }
};

// Automatic stack variable that exits the process after
// a time specified in the environment.

class TimeBomb : public ACE_Task_Base {
 private:
  // UNUSED int m_numberOfClient;
  void (*m_cleanupCallback)();
  void callClientCleanup() {
    if (m_cleanupCallback != nullptr) m_cleanupCallback();
  }

 public:
  std::chrono::seconds m_sleep;

  explicit TimeBomb(void (*cleanupFunc)() = nullptr)
      : m_sleep(0) /* UNUSED , m_numberOfClient( -1 )*/
  {
    std::string sleepEnv = apache::geode::client::Utils::getEnv("TIMEBOMB");
    if (!sleepEnv.empty()) {
      m_sleep = std::chrono::seconds{std::stoi(sleepEnv)};
    }

    m_cleanupCallback = cleanupFunc;
    arm();  // starting
  }

  int arm() {
    int thrAttrs = THR_NEW_LWP | THR_DETACHED;
#ifndef WIN32
    thrAttrs |= THR_INHERIT_SCHED;
#endif
    return activate(thrAttrs, 1);
  }

  int svc() override {
    if (m_sleep == std::chrono::seconds{}) {
      printf("###### TIMEBOMB Disabled. ######\n");
      fflush(stdout);
      return 0;
    }

    auto start = std::chrono::steady_clock::now();
    decltype(start) now;

    do {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      now = std::chrono::steady_clock::now();
    } while ((now - start) < m_sleep);

    printf("####### ERROR: TIMEBOMB WENT OFF, TEST TIMED OUT ########\n");
    fflush(stdout);

    callClientCleanup();

    exit(-1);
    return 0;
  }

  ~TimeBomb() noexcept override = default;
};

#endif  // GEODE_INTEGRATION_TEST_TIMEBOMB_H_
