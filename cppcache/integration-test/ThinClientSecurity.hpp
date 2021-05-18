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

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTSECURITY_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTSECURITY_H_

#include <geode/AuthenticatedView.hpp>
#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"
#include "CacheImplHelper.hpp"
#include "testUtils.hpp"

namespace { // NOLINT(google-build-namespaces)

using apache::geode::client::AuthenticatedView;

static bool isLocalServer = false;
static bool isLocator = false;
static int numberOfLocators = 1;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

void setCacheListener(const std::string& regName,
                      const std::shared_ptr<CacheListener>& listener) {
  if (listener != nullptr) {
    auto reg = getHelper()->getRegion(regName);
    auto attrMutator = reg->getAttributesMutator();
    attrMutator->setCacheListener(listener);
  }
}

void createRegionForSecurity(
    const std::string& name, bool ackMode,
    bool clientNotificationEnabled = false,
    const std::shared_ptr<CacheListener>& listener = nullptr,
    bool caching = true, int connections = -1, bool isMultiuserMode = false,
    int subscriptionRedundancy = -1) {
  char msg[128] = {'\0'};
  LOG(msg);
  LOG(" pool is creating");
  char buff[128] = {'\0'};
  auto poolName = name;

  if (getHelper()->getCache()->getPoolManager().find(name)) {
    static unsigned int index = 0;
    poolName += "_" + std::to_string(index++);
  }

  printf("createRegionForSecurity poolname = %s \n", poolName.c_str());

  getHelper()->createPoolWithLocators(
      poolName, locatorsG, clientNotificationEnabled, subscriptionRedundancy,
      std::chrono::milliseconds::zero(), connections, isMultiuserMode);

  createRegionAndAttachPool(name, ackMode, poolName, caching);
  setCacheListener(name, listener);
}

std::shared_ptr<Pool> getPool(const std::string& name) {
  return getHelper()->getCache()->getPoolManager().find(name);
}

AuthenticatedView getVirtualCache(std::shared_ptr<Properties> creds,
                                  std::shared_ptr<Pool> pool) {
  auto cachePtr = getHelper()->getCache();
  return cachePtr->createAuthenticatedView(creds, pool->getName());
}

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTSECURITY_H_
