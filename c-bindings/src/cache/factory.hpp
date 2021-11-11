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
#include "geode/CacheFactory.hpp"

// C client public headers
#include "geode/client.h"

// C client private headers
#include "auth_initialize.hpp"
#include "client.hpp"

class CacheFactoryWrapper : public ClientKeeper {
  apache::geode::client::CacheFactory cacheFactory_;
  std::shared_ptr<AuthInitializeWrapper> authInit_;

 public:
  CacheFactoryWrapper();

  ~CacheFactoryWrapper();

  const char* getVersion();

  const char* getProductDescription();

  void setPdxIgnoreUnreadFields(bool pdxIgnoreUnreadFields);

  void setAuthInitialize(void (*getCredentials)(apache_geode_properties_t*),
                         void (*close)());

  void setPdxReadSerialized(bool pdxReadSerialized);

  void setProperty(const std::string& key, const std::string& value);

  CacheWrapper* createCache();
};
