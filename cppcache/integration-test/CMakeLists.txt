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
project(nativeclient.tests.cppcache LANGUAGES CXX)

add_library(test-cppcache-utils STATIC
  fw_dunit.cpp
  CacheHelper.cpp
  CacheableWrapper.cpp
)
target_link_libraries(test-cppcache-utils
  PRIVATE
    integration-framework
    ACE::ACE
    _WarningsAsError
    spdlog::spdlog
  PUBLIC
    apache-geode
    framework
)
set_target_properties(test-cppcache-utils PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
	FOLDER cpp/test/integration
)
target_compile_definitions(test-cppcache-utils
  PRIVATE
    $<MAKE_C_IDENTIFIER:test-cppcache-utils>_static=1
)
target_include_directories(test-cppcache-utils
  PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)
add_clangformat(test-cppcache-utils)


add_library(unit_test_callbacks SHARED LibraryCallbacks.cpp)
target_link_libraries(unit_test_callbacks
  PRIVATE
    ACE::ACE
    _WarningsAsError
    spdlog::spdlog
  PUBLIC
    apache-geode
)
target_include_directories(unit_test_callbacks
  PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

set_target_properties(unit_test_callbacks PROPERTIES FOLDER cpp/test/integration)

add_clangformat(unit_test_callbacks)


enable_testing()

set(LIBRARY_PATH_ENVIRONMENT_VARIABLE LD_LIBRARY_PATH)
if (APPLE)
  set(LIBRARY_PATH_ENVIRONMENT_VARIABLE DYLD_LIBRARY_PATH)
elseif(WIN32)
  set(LIBRARY_PATH_ENVIRONMENT_VARIABLE PATH)
endif()
set(LIBRARY_PATH
  $<TARGET_LINKER_FILE_DIR:apache-geode>
  $<TARGET_LINKER_FILE_DIR:SqLiteImpl>
  $<TARGET_PROPERTY:SQLite::sqlite3,INTERFACE_LIBRARY_DIR>
  $<TARGET_PROPERTY:SQLite::sqlite3,INTERFACE_RUNTIME_DIR>
  $<TARGET_LINKER_FILE_DIR:unit_test_callbacks>
  $<TARGET_LINKER_FILE_DIR:testobject>
)

add_custom_target(cppcache-integration-tests)
set_target_properties(cppcache-integration-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
  FOLDER cpp/test/integration
)
add_dependencies(integration-tests cppcache-integration-tests)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test*.cpp")
foreach(FILE ${SOURCES})
  string(REGEX REPLACE "\\.cpp" "" TEST ${FILE})
  set(TESTS ${TESTS} ${TEST})

  add_executable(${TEST} ${FILE})

  add_dependencies(integration-tests ${TEST})

  set_target_properties(${TEST} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER cpp/test/integration/legacy
  )

  target_link_libraries(${TEST}
    PRIVATE
      ACE::ACE
      Boost::boost
      Boost::iostreams
      test-cppcache-utils
      _WarningsAsError
      _CppCodeCoverage
      internal
      spdlog::spdlog
    PUBLIC
      apache-geode
      testobject
      framework
      security
  )

  target_include_directories(${TEST}
    PRIVATE
      $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
  )

  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${TEST}
      PRIVATE
        -Wno-unused-function
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TEST}
      PRIVATE
        -Wno-unused-member-function
        -Wno-missing-variable-declarations
    )
  endif()

  # Some tests depend on these library
  add_dependencies(${TEST} securityImpl SqLiteImpl)

  add_clangformat(${TEST})

  set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/.tests/${TEST})
  file(MAKE_DIRECTORY ${TEST_DIR})

  add_test(NAME ${TEST}
      COMMAND ${TEST}
      WORKING_DIRECTORY ${TEST_DIR}
  )

  set(ENVIRONMENT
    TESTNAME=${TEST}
    "TESTSRC=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
    TIMEBOMB=1000
    MCAST_ADDR=0.0.0.0 # unused but framework expects
    MCAST_PORT=0       # unused but framework expects
    GFE_LOGLEVEL=config
    GFE_SECLOGLEVEL=config
    "GF_JAVA=$<SHELL_PATH:${Java_JAVA_EXECUTABLE}>"
    "GFJAVA=$<SHELL_PATH:${Geode_PATH}>"
    "GF_CLASSPATH=$<SHELL_PATH:${CMAKE_BINARY_DIR}/tests/javaobject/javaobject.jar>"
    "${LIBRARY_PATH_ENVIRONMENT_VARIABLE}=$<JOIN:$<SHELL_PATH:${LIBRARY_PATH}>,\\$<SEMICOLON>>"
  )

  set_tests_properties(${TEST} PROPERTIES
    ENVIRONMENT "${ENVIRONMENT}"
  )

  add_dependencies(cppcache-integration-tests ${TEST})
endforeach()

configure_file(CTestCustom.cmake.in CTestCustom.cmake)

#TODO this is really bad that we include the root of tests
include_directories(${CMAKE_SOURCE_DIR}/tests/cpp)

set_tests_properties(
    # flaky
    testOverflowPutGetSqLite
    testThinClientAfterRegionLive
    testThinClientCacheables
    testThinClientCq
    testThinClientDurableDisconnectNormal
    testThinClientDurableDisconnectTimeout
    testThinClientDurableReconnect
    testThinClientHAFailover
    testThinClientHAMixedRedundancy
    testThinClientHAQueryFailover
    testThinClientLRUExpiration
    testThinClientLocatorFailover
    testThinClientMultiDS
    testThinClientPartitionResolver
    testThinClientPdxDeltaWithNotification
    testThinClientPdxTests
    testThinClientPutAll
    testThinClientPutAllWithCallBackArgWithoutConcurrency
    testThinClientSecurityCQAuthorizationMU
    testThinClientTXFailover
    testTimedSemaphore
    # broken
    testThinClientCqDurable
    testThinClientGatewayTest
    testThinClientPoolAttrTest
    testThinClientPoolLocator
    testThinClientPoolRedundancy
    testThinClientRemoveOps
    testThinClientSecurityAuthentication
    testThinClientSecurityAuthenticationMU
    testThinClientSecurityAuthorization
    testThinClientSecurityAuthorizationMU
    testThinClientSecurityDurableCQAuthorizationMU
    testThinClientSecurityPostAuthorization
    testThinClientTicket303
    testThinClientTicket304
    testThinClientTracking
    testThinClientWriterException
  PROPERTIES
    DISABLED TRUE
)

add_custom_target(run-cppcache-integration-tests
  COMMAND ctest -C $<CONFIG>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)
add_dependencies(run-cppcache-integration-tests cppcache-integration-tests)
add_dependencies(run-integration-tests run-cppcache-integration-tests)
set_target_properties(run-cppcache-integration-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
	FOLDER cpp/test/integration
)
