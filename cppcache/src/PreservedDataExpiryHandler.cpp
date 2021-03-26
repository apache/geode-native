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
#include "PreservedDataExpiryHandler.hpp"

#include <boost/thread/lock_types.hpp>

#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

PreservedDataExpiryHandler::PreservedDataExpiryHandler(
    const std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
    const std::shared_ptr<PdxSerializable>& pdxObjectPtr)
    : m_pdxTypeRegistry(pdxTypeRegistry), m_pdxObjectPtr(pdxObjectPtr) {}

int PreservedDataExpiryHandler::handle_timeout(const ACE_Time_Value&,
                                               const void*) {
  auto& mutex = m_pdxTypeRegistry->getPreservedDataMutex();
  boost::unique_lock<std::remove_reference<decltype(mutex)>::type> guard{mutex};

  auto map = m_pdxTypeRegistry->getPreserveDataMap();
  LOGDEBUG(
      "Entered PreservedDataExpiryHandler "
      "PdxTypeRegistry::getPreserveDataMap().size() = %zu",
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
  delete this;
  return 0;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
