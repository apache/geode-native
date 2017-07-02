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

#define ROOT_NAME "testFWHelper"

#include <iostream>

#include "fw_helper.hpp"

/**
 * @brief Test a test runs.
 */
BEGIN_TEST(TestOne)
  std::cout << "test 1." << std::endl;
END_TEST(TestOne)

/**
 * @brief Test a test runs.
 */
BEGIN_TEST(TestTwo)
  std::cout << "test 2." << std::endl;
END_TEST(TestTwo)
