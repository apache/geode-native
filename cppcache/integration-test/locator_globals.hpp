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

#ifndef GEODE_INTEGRATION_TEST_LOCATOR_GLOBALS_H_
#define GEODE_INTEGRATION_TEST_LOCATOR_GLOBALS_H_

namespace { // NOLINT

using apache::geode::client::CacheHelper;

static int numberOfLocators = 1;
bool isLocalServer = false;
bool isLocator = false;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_LOCATOR_GLOBALS_H_
