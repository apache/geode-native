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

#include "exampleAuthInitialize.hpp"

#include <iostream>

using apache::geode::client::AuthInitialize;
using apache::geode::client::Properties;

std::shared_ptr<Properties> ExampleAuthInitialize::getCredentials(
    const std::shared_ptr<Properties>& securityprops,
    const std::string& /*server*/) {
  std::cout << "ExampleAuthInitialize::GetCredentials called\n";

  securityprops->insert("security-username", "root");
  securityprops->insert("security-password", "root");

  return securityprops;
}

void ExampleAuthInitialize::close() {
  std::cout << "ExampleAuthInitialize::close called\n";
}

ExampleAuthInitialize::ExampleAuthInitialize() : AuthInitialize() {
  std::cout << "ExampleAuthInitialize::ExampleAuthInitialize called\n";
}
