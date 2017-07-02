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
#include <geode/PoolManager.hpp>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>

using namespace apache::geode::client;

// TODO: make this a member of TcrConnectionManager.
HashMapOfPools* connectionPools = nullptr; /*new HashMapOfPools( )*/
ACE_Recursive_Thread_Mutex connectionPoolsLock;

void removePool(const char* name) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connectionPoolsLock);
  connectionPools->erase(name);
}

PoolFactoryPtr PoolManager::createFactory() {
  if (connectionPools == nullptr) {
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connectionPoolsLock);
    if (connectionPools == nullptr) {
      connectionPools = new HashMapOfPools();
    }
  }
  return PoolFactoryPtr(new PoolFactory());
}

void PoolManager::close(bool keepAlive) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connectionPoolsLock);

  if (connectionPools == nullptr) {
    return;
  }

  std::vector<PoolPtr> poolsList;

  for (const auto& c : *connectionPools) {
    poolsList.push_back(c.second);
  }

  for (const auto& iter : poolsList) {
    iter->destroy(keepAlive);
  }

  GF_SAFE_DELETE(connectionPools);
}

PoolPtr PoolManager::find(const char* name) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connectionPoolsLock);

  if (connectionPools == nullptr) {
    connectionPools = new HashMapOfPools();
  }

  if (name) {
    const auto& iter = connectionPools->find(name);

    PoolPtr poolPtr = nullptr;

    if (iter != connectionPools->end()) {
      poolPtr = iter->second;
      GF_DEV_ASSERT(poolPtr != nullptr);
    }

    return poolPtr;
  } else {
    return nullptr;
  }
}

PoolPtr PoolManager::find(RegionPtr region) {
  return find(region->getAttributes()->getPoolName());
}

const HashMapOfPools& PoolManager::getAll() {
  if (connectionPools == nullptr) {
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(connectionPoolsLock);
    if (connectionPools == nullptr) {
      connectionPools = new HashMapOfPools();
    }
  }
  return *connectionPools;
}
