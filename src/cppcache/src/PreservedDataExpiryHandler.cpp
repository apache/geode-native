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
/*
 * PreservedDataExpiryHandler.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: npatel
 */
#include "ace/Timer_Queue.h"
#include "ace/Timer_Heap.h"
#include "ace/Reactor.h"
#include "ace/svc_export.h"
#include "ace/Timer_Heap_T.h"
#include "ace/Timer_Queue_Adapters.h"

#include "PreservedDataExpiryHandler.hpp"
#include "PdxTypeRegistry.hpp"

using namespace apache::geode::client;

PreservedDataExpiryHandler::PreservedDataExpiryHandler(
	const std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
    const PdxSerializablePtr& pdxObjectPtr, uint32_t duration)
    :  // UNUSED m_duration(duration),
      m_pdxObjectPtr(pdxObjectPtr),
	  m_pdxTypeRegistry(pdxTypeRegistry) {}

int PreservedDataExpiryHandler::handle_timeout(
    const ACE_Time_Value& current_time, const void* arg) {
  WriteGuard guard(m_pdxTypeRegistry->getPreservedDataLock());
  auto map = m_pdxTypeRegistry->getPreserveDataMap();
  LOGDEBUG(
      "Entered PreservedDataExpiryHandler "
      "PdxTypeRegistry::getPreserveDataMap().size() = %d",
      map.size());

  try {
    // remove the entry from the map
    map.erase(m_pdxObjectPtr);
  } catch (...) {
    // Ignore whatever exception comes
    LOGDEBUG(
        "PreservedDataExpiry:: Error while Clearing PdxObject and its "
        "preserved data. Ignoring the error");
  }
  return 0;
}

int PreservedDataExpiryHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask) {
  return 0;
}
