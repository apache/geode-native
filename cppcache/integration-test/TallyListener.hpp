#pragma once

#ifndef GEODE_INTEGRATION_TEST_TALLYLISTENER_H_
#define GEODE_INTEGRATION_TEST_TALLYLISTENER_H_

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

#include <geode/EntryEvent.hpp>
#include <string>
#include <util/Log.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testing {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheListener;
using apache::geode::client::EntryEvent;
using apache::geode::client::RegionEvent;

class TallyListener : public CacheListener {
 private:
  int m_creates;
  int m_updates;
  int m_invalidates;
  int m_destroys;
  int m_clears;
  bool isListnerInvoked;
  bool isCallbackCalled;
  std::shared_ptr<CacheableKey> m_lastKey;
  std::shared_ptr<Cacheable> m_lastValue;
  std::shared_ptr<CacheableKey> m_callbackArg;
  bool m_ignoreTimeout;
  bool m_quiet;

 public:
  TallyListener()
      : CacheListener(),
        m_creates(0),
        m_updates(0),
        m_invalidates(0),
        m_destroys(0),
        m_clears(0),
        isListnerInvoked(false),
        isCallbackCalled(false),
        m_lastKey(),
        m_lastValue(),
        m_callbackArg(nullptr),
        m_ignoreTimeout(false),
        m_quiet(false) {
    LOG("TallyListener contructor called");
  }

  ~TallyListener() noexcept override = default;

  void beQuiet(bool v) { m_quiet = v; }

  int expectCreates(int expected) {
    int tries = 0;
    while ((m_creates < expected) && (tries < 200)) {
      SLEEP(100);
      tries++;
    }
    return m_creates;
  }

  int getCreates() { return m_creates; }

  int expectUpdates(int expected) {
    int tries = 0;
    while ((m_updates < expected) && (tries < 200)) {
      SLEEP(100);
      tries++;
    }
    return m_updates;
  }
  void resetListnerInvokation() {
    isListnerInvoked = false;
    isCallbackCalled = false;
  }
  int getUpdates() { return m_updates; }

  int getInvalidates() { return m_invalidates; }
  int getDestroys() { return m_destroys; }
  bool isListenerInvoked() { return isListnerInvoked; }
  void setCallBackArg(const std::shared_ptr<CacheableKey>& callbackArg) {
    m_callbackArg = callbackArg;
  }
  std::shared_ptr<CacheableKey> getLastKey() { return m_lastKey; }

  std::shared_ptr<Cacheable> getLastValue() { return m_lastValue; }
  bool isCallBackArgCalled() { return isCallbackCalled; }
  void checkcallbackArg(const EntryEvent& event) {
    if (!isListnerInvoked) isListnerInvoked = true;
    if (m_callbackArg != nullptr) {
      auto callbkArg =
          std::dynamic_pointer_cast<CacheableKey>(event.getCallbackArgument());
      if (strcmp(m_callbackArg->toString().c_str(),
                 callbkArg->toString().c_str()) == 0) {
        isCallbackCalled = true;
      }
    }
  }

  int getClears() { return m_clears; }

  void afterCreate(const EntryEvent& event) override;

  void afterUpdate(const EntryEvent& event) override;

  void afterInvalidate(const EntryEvent& event) override;

  void afterDestroy(const EntryEvent& event) override;

  void afterRegionClear(const RegionEvent& event) override {
    CacheListener::afterRegionClear(event);
  }

  void afterRegionInvalidate(const RegionEvent&) override {}

  void afterRegionDestroy(const RegionEvent&) override {}

  void showTallies() {
    char buf[1024];
    sprintf(buf,
            "TallyListener state: (updates = %d, creates = %d , invalidates = "
            "%d destroys = %d Regionclears = %d)",
            getUpdates(), getCreates(), getInvalidates(), getDestroys(),
            getClears());
    LOG(buf);
  }
};

void TallyListener::afterCreate(const EntryEvent& event) {
  m_creates++;
  LOG_DEBUG("TallyListener::afterCreate called m_creates = %d ", m_creates);
  m_lastKey = event.getKey();
  m_lastValue = event.getNewValue();
  checkcallbackArg(event);

  auto strPtr = std::dynamic_pointer_cast<CacheableString>(event.getNewValue());
  if (!m_quiet) {
    char buf[1024];
    sprintf(buf, "TallyListener create - key = \"%s\", value = \"%s\"",
            m_lastKey->toString().c_str(), strPtr->value().c_str());
    LOG_DEBUG(buf);
  }
  std::string keyString(m_lastKey->toString().c_str());
  if ((!m_ignoreTimeout) && (keyString.find("timeout") != std::string::npos)) {
    LOG("TallyListener: Sleeping 10 seconds to force a timeout.");
    SLEEP(10000);  // this should give the client cause to timeout...
    LOG("TallyListener: done sleeping..");
  }
}

void TallyListener::afterUpdate(const EntryEvent& event) {
  m_updates++;
  m_lastKey = event.getKey();
  m_lastValue = event.getNewValue();
  checkcallbackArg(event);
  auto strPtr = std::dynamic_pointer_cast<CacheableString>(event.getNewValue());
  if (!m_quiet) {
    char buf[1024];
    sprintf(buf, "TallyListener update - key = \"%s\", value = \"%s\"",
            m_lastKey->toString().c_str(), strPtr->value().c_str());
    LOG(buf);
  }
  std::string keyString(m_lastKey->toString().c_str());
  if ((!m_ignoreTimeout) && (keyString.find("timeout") != std::string::npos)) {
    LOG("TallyListener: Sleeping 10 seconds to force a timeout.");
    SLEEP(10000);  // this should give the client cause to timeout...
    LOG("TallyListener: done sleeping..");
  }
}
void TallyListener::afterInvalidate(const EntryEvent& event) {
  m_invalidates++;
  checkcallbackArg(event);
}
void TallyListener::afterDestroy(const EntryEvent& event) {
  m_destroys++;
  checkcallbackArg(event);
}

}  // namespace testing
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTEGRATION_TEST_TALLYLISTENER_H_
