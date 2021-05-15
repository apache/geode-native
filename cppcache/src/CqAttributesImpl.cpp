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

#include "CqAttributesImpl.hpp"

#include <geode/ExceptionTypes.hpp>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

CqAttributes::listener_container_type CqAttributesImpl::getCqListeners() {
  std::lock_guard<decltype(m_mutex)> _guard(m_mutex);
  return m_cqListeners;
}

void CqAttributesImpl::addCqListener(const std::shared_ptr<CqListener>& cql) {
  if (cql == nullptr) {
    throw IllegalArgumentException("addCqListener parameter was null");
  }
  std::lock_guard<decltype(m_mutex)> _guard(m_mutex);
  m_cqListeners.push_back(cql);
}

CqAttributesImpl* CqAttributesImpl::clone() {
  auto clone = new CqAttributesImpl();
  clone->setCqListeners(m_cqListeners);
  return clone;
}

void CqAttributesImpl::setCqListeners(
    const listener_container_type& addedListeners) {
  if (addedListeners.empty() == true) {
    LOG_WARN("setCqListeners parameter had a null element, nothing to be set");
    return;
  }

  decltype(m_cqListeners) oldListeners(m_cqListeners);
  {
    std::lock_guard<decltype(m_mutex)> _guard(m_mutex);
    m_cqListeners = addedListeners;
  }
  if (!oldListeners.empty()) {
    for (auto l : oldListeners) {
      try {
        l->close();
        // Handle client side exceptions.
      } catch (Exception& ex) {
        LOG_WARN("Exception occured while closing CQ Listener %s Error",
                 ex.what());
      }
    }
    oldListeners.clear();
  }
}

void CqAttributesImpl::removeCqListener(
    const std::shared_ptr<CqListener>& cql) {
  if (cql == nullptr) {
    throw IllegalArgumentException("removeCqListener parameter was null");
  }
  std::lock_guard<decltype(m_mutex)> _guard(m_mutex);
  if (!m_cqListeners.empty()) {
    m_cqListeners.erase(
        std::remove_if(
            m_cqListeners.begin(), m_cqListeners.end(),
            [cql](std::shared_ptr<CqListener> l) -> bool { return cql == l; }),
        m_cqListeners.end());
    try {
      cql->close();
      // Handle client side exceptions.
    } catch (Exception& ex) {
      LOG_WARN("Exception closing CQ Listener %s Error ", ex.what());
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
