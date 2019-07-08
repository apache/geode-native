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

add_executable(apache-geode_unittests
  AutoDeleteTest.cpp
  ByteArray.cpp
  ByteArray.hpp
  ByteArrayFixture.cpp
  ByteArrayFixture.hpp
  ByteArrayTest.cpp
  CacheableDateTest.cpp
  CacheableKeyCreateTests.cpp
  CacheableKeysTest.cpp
  CacheableStringEqualityTest.cpp
  CacheableStringTests.cpp
  CacheTest.cpp
  CacheXmlParserTest.cpp
  ChunkedHeaderTest.cpp
  ClientConnectionResponseTest.cpp
  ClientProxyMembershipIDTest.cpp
  ConnectionQueueTest.cpp
  DataInputTest.cpp
  DataOutputTest.cpp
  ExceptionTypesTest.cpp
  ExpiryTaskTest.cpp
  ExpiryTaskManagerTest.cpp
  GatewaySenderEventCallbackArgumentTest.cpp
  geodeBannerTest.cpp
  gtest_extensions.h
  gmock_extensions.h
  InterestResultPolicyTest.cpp
  LocalRegionTest.cpp
  LoggingTest.cpp
  LRUQueueTest.cpp
  PartitionTest.cpp
  LoggingTest.cpp
  PdxInstanceImplTest.cpp
  PdxTypeTest.cpp
  QueueConnectionRequestTest.cpp
  RegionAttributesFactoryTest.cpp
  SerializableCreateTests.cpp
  StringPrefixPartitionResolverTest.cpp
  StructSetTest.cpp
  TcrMessageTest.cpp
  ThreadPoolTest.cpp
  TXIdTest.cpp
  mock/MockExpiryTask.hpp
  mock/MapEntryImplMock.hpp
  statistics/HostStatSamplerTest.cpp
  util/functionalTests.cpp
  util/JavaModifiedUtf8Tests.cpp
  util/queueTest.cpp
  util/synchronized_mapTest.cpp
  util/synchronized_setTest.cpp
  util/TestableRecursiveMutex.hpp
  util/chrono/durationTest.cpp)

target_compile_definitions(apache-geode_unittests
  PUBLIC
    GTEST_ELLIPSIS_NEEDS_POD_
)

if (MSVC)
  target_compile_options(apache-geode_unittests PRIVATE "/MD$<$<CONFIG:Debug>:d>")
endif()

target_link_libraries(apache-geode_unittests
  PRIVATE
    apache-geode-static
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    _WarningsAsError
    _CppCodeCoverage
)

target_include_directories(apache-geode_unittests
  PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

add_dependencies(unit-tests apache-geode_unittests)

set_target_properties(apache-geode_unittests PROPERTIES
  FOLDER cpp/test/unit
  )

add_clangformat(apache-geode_unittests)

enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)


add_custom_target(run-cppcache-unit-tests
  COMMAND ctest -C $<CONFIGURATION> ${CTEST_${PROJECT_NAME}_VERBOSITY}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)
set_target_properties(run-cppcache-unit-tests PROPERTIES
  FOLDER cpp/test/unit
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
)
add_dependencies(run-unit-tests run-cppcache-unit-tests)
