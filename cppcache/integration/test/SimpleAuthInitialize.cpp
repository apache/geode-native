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

#include "SimpleAuthInitialize.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <gtest/gtest.h>

using apache::geode::client::AuthInitialize;
using apache::geode::client::Properties;

std::shared_ptr<Properties> SimpleAuthInitialize::getCredentials(
    const std::shared_ptr<Properties>& securityprops,
    const std::string& /*server*/) {
  std::cout << "SimpleAuthInitialize::GetCredentials called\n";

  securityprops->insert("security-username", username_);
  securityprops->insert("security-password", password_);

  countOfGetCredentialsCalls_++;
  return securityprops;
}

void SimpleAuthInitialize::close() {
  std::cout << "SimpleAuthInitialize::close called\n";
}

SimpleAuthInitialize::SimpleAuthInitialize()
    : AuthInitialize(),
      username_("root"),
      password_("root-password"),
      countOfGetCredentialsCalls_(0) {
  std::cout << "SimpleAuthInitialize::SimpleAuthInitialize called\n";
}

SimpleAuthInitialize::SimpleAuthInitialize(std::string username,
                                           std::string password)
    : username_(std::move(username)),
      password_(std::move(password)),
      countOfGetCredentialsCalls_(0) {}

int32_t SimpleAuthInitialize::getGetCredentialsCallCount() {
  return countOfGetCredentialsCalls_;
}
