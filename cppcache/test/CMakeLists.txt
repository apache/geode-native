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

cmake_minimum_required( VERSION 3.10 )
project(apache-geode_unittests LANGUAGES CXX)

file(GLOB_RECURSE SOURCES "*.cpp")

add_executable(${PROJECT_NAME} 
  AutoDeleteTest.cpp
  ByteArray.cpp
  ByteArray.hpp
  ByteArrayFixture.cpp
  ByteArrayFixture.hpp
  ByteArrayTest.cpp
  CacheableKeyCreateTests.cpp
  CacheableKeysTest.cpp
  CacheableStringEqualityTest.cpp
  CacheableStringTests.cpp
  CacheXmlParserTest.cpp
  ClientConnectionResponseTest.cpp
  ClientProxyMembershipIDFactoryTest.cpp
  DataInputTest.cpp
  DataOutputTest.cpp
  ExceptionTypesTest.cpp
  geodeBannerTest.cpp
  gtest_extensions.h
  InterestResultPolicyTest.cpp
  RegionAttributesFactoryTest.cpp
  SerializableCreateTests.cpp
  StructSetTest.cpp
  TcrMessage_unittest.cpp
)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE "/MD$<$<CONFIG:Debug>:d>")
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    apache-geode-static
    GTest::GTest
    GTest::Main
    Boost::boost
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

add_dependencies(unit-tests ${PROJECT_NAME})


enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)


add_custom_target(run-cppcache-unit-tests
  COMMAND ctest -C $<CONFIGURATION> ${CTEST_${PROJECT_NAME}_VERBOSITY}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)
set_target_properties(run-cppcache-unit-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
)
add_dependencies(run-unit-tests run-cppcache-unit-tests)
