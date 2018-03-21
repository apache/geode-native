# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10)
project(integration-test-2 CXX)

set(CMAKE_CXX_STANDARD 11)

configure_file(framework/config.h.in config.h)

add_executable(${PROJECT_NAME}
  ExampleTest.cpp
  framework/Gfsh.cpp
  framework/Gfsh.h
  framework/Framework.cpp
  framework/Framework.h
  framework/Cluster.cpp
  framework/Cluster.h
  framework/GfshProcess.cpp
  framework/GfshProcess.h
  framework/GfshExecute.cpp
  framework/GfshExecute.h
  RegionPutGetAllTest.cpp
  )

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    apache-geode
    testobject
    GTest::GTest
    GTest::Main
    Boost::boost
    Boost::system
    Boost::log
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})