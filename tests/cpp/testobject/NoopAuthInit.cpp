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

#include "NoopAuthInit.hpp"

#include <util/Log.hpp>

#include "geode/ExceptionTypes.hpp"
#include "geode/Properties.hpp"
#include "testobject_export.h"

namespace apache {
namespace geode {
namespace client {

extern "C" {
TESTOBJECT_EXPORT AuthInitialize* createNoopAuthInitInstance() {
  LOG_INFO("rjk: calling createNoopAuthInitInstance");
  return new NoopAuthInit();
}
}
std::shared_ptr<Properties> NoopAuthInit::getCredentials(
    const std::shared_ptr<Properties>&, const std::string&) {
  LOG_INFO("rjk: calling NoopAuthInit::getCredentials");
  auto credentials = Properties::create();
  return credentials;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
