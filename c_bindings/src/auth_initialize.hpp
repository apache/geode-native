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

#include <memory>
#include <string>

#include "geode/AuthInitialize.hpp"
#include "geode/auth_initialize.h"

namespace apache::geode::client {
      class Properties;
}

class AuthInitializeWrapper : public apache::geode::client::AuthInitialize {
  void (*getCredentials_)(apache_geode_properties_t*);
  void (*close_)();

 public:
  std::shared_ptr<apache::geode::client::Properties> getCredentials(
      const std::shared_ptr<apache::geode::client::Properties>& securityprops,
      const std::string& /*server*/) override;

  void close() override;

  AuthInitializeWrapper(void (*getCredentials)(apache_geode_properties_t*),
                        void (*close)());

  ~AuthInitializeWrapper() override = default;
};
