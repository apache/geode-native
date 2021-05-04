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
#include "UserAttributes.hpp"

#include <geode/AuthenticatedView.hpp>

namespace apache {
namespace geode {
namespace client {

UserAttributes::UserAttributes(std::shared_ptr<Properties> credentials,
                               std::shared_ptr<Pool> pool,
                               AuthenticatedView* authenticatedView)
    : m_isUserAuthenticated(false), m_pool(pool) {
  m_credentials = credentials;

  m_authenticatedView = authenticatedView;
}

bool UserAttributes::isCacheClosed() { return m_authenticatedView->isClosed(); }

UserAttributes::~UserAttributes() {
  std::lock_guard<decltype(m_listLock)> guard(m_listLock);
  for (auto& it : m_connectionAttr) {
    auto uca = it.second;
    if (uca) {
      _GEODE_SAFE_DELETE(uca);
    }
  }
}

UserConnectionAttributes* UserAttributes::getConnectionAttribute() {
  LOG_DEBUG("UserConnectionAttributes* getConnectionAttribute().");
  return nullptr;
}

void UserAttributes::unAuthenticateEP(TcrEndpoint* endpoint) {
  LOG_DEBUG("UserAttributes::unAuthenticateEP.");
  if (m_connectionAttr.size() == 0) return;
  // TODO: it is always returning first one
  // need to take care when FE for onServers();
  // TODO: chk before returing whether endpoint is up or not
  // std::map<std::string, UserConnectionAttributes>::iterator it;

  std::lock_guard<decltype(m_listLock)> guard(m_listLock);
  auto uca = m_connectionAttr[endpoint->name()];
  if (uca) {
    m_connectionAttr.erase(endpoint->name());
    _GEODE_SAFE_DELETE(uca);
  }
  /*for( it = m_connectionAttr.begin(); it != m_connectionAttr.end(); it++ )
  {
    UserConnectionAttributes* uca = &((*it).second);
    if (uca->getEndpoint() == endpoint)
      uca->setUnAuthenticated();
  }*/
}
std::shared_ptr<Pool> UserAttributes::getPool() { return m_pool; }

UserConnectionAttributes* UserAttributes::getConnectionAttribute(
    TcrEndpoint* ep) {
  LOG_DEBUG("UserConnectionAttributes* getConnectionAttribute with EP.");
  if (m_connectionAttr.size() == 0) return nullptr;

  std::lock_guard<decltype(m_listLock)> guard(m_listLock);
  return m_connectionAttr[ep->name()];
}

bool UserAttributes::isEndpointAuthenticated(TcrEndpoint* ep) {
  LOG_DEBUG(
      "UserAttributes::isEndpointAuthenticated: (TcrEndpoint* ep) with EP.");
  if (m_connectionAttr.size() == 0) return false;

  std::lock_guard<decltype(m_listLock)> guard(m_listLock);
  auto uca = m_connectionAttr[ep->name()];
  if (uca && uca->isAuthenticated() && (uca->getEndpoint() == ep)) {
    return true;
  }
  return false;
}
std::shared_ptr<Properties> UserAttributes::getCredentials() {
  if (m_authenticatedView->isClosed()) {
    throw IllegalStateException("User cache has been closed");
  }
  if (m_credentials == nullptr) {
    LOG_DEBUG("getCredentials");
  } else {
    LOG_DEBUG("getCredentials not null ");
  }
  return m_credentials;
}
AuthenticatedView* UserAttributes::getAuthenticatedView() {
  return m_authenticatedView;
}

thread_local std::shared_ptr<UserAttributes>
    UserAttributes::threadLocalUserAttributes;

GuardUserAttributes::GuardUserAttributes(AuthenticatedView* authenticatedView) {
  setAuthenticatedView(authenticatedView);
}

void GuardUserAttributes::setAuthenticatedView(
    AuthenticatedView* authenticatedView) {
  m_authenticatedView = authenticatedView;
  LOG_DEBUG("GuardUserAttributes::GuardUserAttributes:");
  if (m_authenticatedView != nullptr && !authenticatedView->isClosed()) {
    UserAttributes::threadLocalUserAttributes =
        authenticatedView->m_userAttributes;
  } else {
    throw CacheClosedException("User Cache has been closed");
  }
}

GuardUserAttributes::GuardUserAttributes() { m_authenticatedView = nullptr; }

GuardUserAttributes::~GuardUserAttributes() {
  if (m_authenticatedView != nullptr) {
    UserAttributes::threadLocalUserAttributes = nullptr;
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
