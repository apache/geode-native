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

#include <geode/SystemProperties.hpp>

#include "DistributedSystemImpl.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

volatile bool DistributedSystemImpl::m_isCliCallbackSet = false;
std::map<int, CliCallbackMethod> DistributedSystemImpl::m_cliCallbackMap;
ACE_Recursive_Thread_Mutex DistributedSystemImpl::m_cliCallbackLock;

DistributedSystemImpl::DistributedSystemImpl(const char* name,
                                             DistributedSystem* implementee)
    : m_name(name == 0 ? "" : name), m_implementee(implementee) {
  if (m_implementee->getSystemProperties().isDhOn()) {
    // m_dh.initDhKeys(m_implementee->getSystemProperties()->getSecurityProperties());
  }
}

DistributedSystemImpl::~DistributedSystemImpl() {
  if (m_implementee->getSystemProperties().isDhOn()) {
    // m_dh.clearDhKeys();
  }
  LOGFINE("Destroyed DistributedSystemImpl");
}

void DistributedSystemImpl::connect() {}

void DistributedSystemImpl::disconnect() {
  LOGFINE("DistributedSystemImpl::disconnect done");
}

void DistributedSystemImpl::CallCliCallBack(Cache& cache) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> disconnectGuard(m_cliCallbackLock);
  if (m_isCliCallbackSet == true) {
    for (std::map<int, CliCallbackMethod>::iterator iter =
             m_cliCallbackMap.begin();
         iter != m_cliCallbackMap.end(); ++iter) {
      (*iter).second(cache);
    }
  }
}

void DistributedSystemImpl::registerCliCallback(int appdomainId,
                                                CliCallbackMethod clicallback) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> disconnectGuard(m_cliCallbackLock);
  m_cliCallbackMap[appdomainId] = clicallback;
  m_isCliCallbackSet = true;
}

void DistributedSystemImpl::unregisterCliCallback(int appdomainId) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> disconnectGuard(m_cliCallbackLock);
  std::map<int, CliCallbackMethod>::iterator iter =
      m_cliCallbackMap.find(appdomainId);
  if (iter != m_cliCallbackMap.end()) {
    m_cliCallbackMap.erase(iter);
    LOGFINE("Removing cliCallback %d", appdomainId);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
