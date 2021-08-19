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

// Standard headers
#include <memory>
#include <string>

// C++ client public headers
#include <geode/Properties.hpp>

// C++ client private headers
#include "util/Log.hpp"

// C client public headers
#include "geode/auth_initialize.h"

// C client private headers
#include "auth_initialize.hpp"

void apache_geode_AuthInitialize_AddProperty(
    apache_geode_properties_t* properties, const char* key, const char* value) {
  auto securityProperties =
      reinterpret_cast<apache::geode::client::Properties*>(properties);
  securityProperties->insert(key, value);
  LOGDEBUG("AuthInitializeWrapper::%s: added (k, v) = (\"%s\", \"%s\")",
           __FUNCTION__, key, value);
}

std::shared_ptr<apache::geode::client::Properties>
AuthInitializeWrapper::getCredentials(
    const std::shared_ptr<apache::geode::client::Properties>& securityprops,
    const std::string& /*server*/) {
  LOGDEBUG("AuthInitializeWrapper::%s(%p): entry", __FUNCTION__, this);

  if (getCredentials_) {
    LOGDEBUG("AuthInitializeWrapper::%s(%p): calling getCredentials()",
             __FUNCTION__, this);
    getCredentials_(
        reinterpret_cast<apache_geode_properties_t*>(securityprops.get()));
  }

  LOGDEBUG("AuthInitializeWrapper::%s(%p): exit", __FUNCTION__, this);
  return securityprops;
}

void AuthInitializeWrapper::close() {
  LOGDEBUG("AuthInitializeWrapper::%s(%p): entry", __FUNCTION__, this);

  if (close_) {
    LOGDEBUG("AuthInitializeWrapper::%s(%p): calling close()", __FUNCTION__,
             this);
    close_();
  }
  LOGDEBUG("AuthInitializeWrapper::%s(%p): exit", __FUNCTION__, this);
}

AuthInitializeWrapper::AuthInitializeWrapper(
    void (*getCredentials)(apache_geode_properties_t*), void (*close)())
    : AuthInitialize(), getCredentials_(getCredentials), close_(close) {}
