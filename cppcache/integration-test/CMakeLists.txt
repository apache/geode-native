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
  BBNamingContext.cpp
  CacheHelper.cpp
  CacheableWrapper.cpp
)
target_link_libraries(test-cppcache-utils
  PRIVATE
    ACE::ACE
    _WarningsAsError
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

# Function to resolve both config and generate stage variables.
macro(generate_config INPUT TEMP OUTPUT)
    configure_file(${INPUT} ${TEMP})
    file(GENERATE OUTPUT ${OUTPUT}
      INPUT ${TEMP}
      CONDITION 1
    )
endmacro()

# Add lazy loaded shared library paths to test environment
foreach( lib SQLite::sqlite3 )
  get_target_property(library_path ${lib} INTERFACE_LIBRARY_DIR)
  set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${library_path})
  get_target_property(runtime_path ${lib} INTERFACE_RUNTIME_DIR)
  set(PATH ${PATH} ${runtime_path})
endforeach()

set (PORT 20000)

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
  add_dependencies(integration-tests ${TEST})
  add_executable(${TEST} ${FILE})
  set_target_properties(${TEST} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER cpp/test/integration/legacy
  )

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(_WarningsAsError INTERFACE
          -Wno-missing-variable-declarations
          )
endif()

  target_link_libraries(${TEST}
    PRIVATE
      ACE::ACE
      test-cppcache-utils
      _WarningsAsError
      _CppCodeCoverage
      internal
    PUBLIC
      apache-geode
      testobject
      framework
  )
  target_include_directories(${TEST} PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
  )

  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${TEST} PRIVATE
        -Wno-unused-function
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TEST} PRIVATE
        -Wno-unused-member-function
    )
  endif()

  # Some tests depend on these library
  add_dependencies(${TEST} securityImpl SqLiteImpl)

  add_clangformat(${TEST})

  set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/.tests/${TEST})

  math(EXPR PORT "${PORT} + 1")

  if (WIN32)
    set (TEST_SCRIPT_SUFFIX .bat)
    generate_config(${CMAKE_CURRENT_SOURCE_DIR}/test.bat.in ${CMAKE_CURRENT_BINARY_DIR}/.${TEST}.bat.in $<TARGET_FILE:${TEST}>${TEST_SCRIPT_SUFFIX})

    set(TEST_COMMAND $<TARGET_FILE:${TEST}>${TEST_SCRIPT_SUFFIX})

  else()
    set (TEST_SCRIPT_SUFFIX .sh)
    generate_config(${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/.${TEST}.sh.in $<TARGET_FILE:${TEST}>${TEST_SCRIPT_SUFFIX})

    # TODO fix file permissions on .sh
    set(TEST_COMMAND bash $<TARGET_FILE:${TEST}>${TEST_SCRIPT_SUFFIX})

  endif()

  add_test(NAME ${TEST}
      COMMAND ${TEST_COMMAND}
  )

  set_tests_properties(${TEST} PROPERTIES LABELS STABLE)
  add_dependencies(cppcache-integration-tests ${TEST})
endforeach()

configure_file(CTestCustom.cmake.in CTestCustom.cmake)

#TODO this is really bad that we include the root of tests
include_directories(${CMAKE_SOURCE_DIR}/tests/cpp)

set_tests_properties(testDataOutput
    testFWHelper
    testSystemProperties
    testLinkage
    testRegionTemplateArgs
    testRegionMap
    testXmlCacheCreationWithRefid
    testRegionAttributesFactory
    testXmlCacheCreationWithOverFlow
    testThinClientRemoveAllLocal
    testDunit
    testSpinLock
    testSubRegions
  PROPERTIES
    LABELS "STABLE;QUICK"
)

set_tests_properties(testOverflowPutGetSqLite 
    testThinClientAfterRegionLive
    testThinClientCacheables
    testThinClientCq
    testThinClientCqFailover
    testThinClientCqHAFailover
    testThinClientDurableConnect
    testThinClientDurableDisconnectNormal
    testThinClientDurableDisconnectTimeout
    testThinClientDurableFailoverClientClosedNoRedundancy
    testThinClientDurableFailoverClientNotClosedRedundancy
    testThinClientDurableKeepAliveFalseTimeout
    testThinClientDurableKeepAliveTrueNormal
    testThinClientDurableReconnect
    testThinClientFailover
    testThinClientFailover2
    testThinClientFailoverInterest
    testThinClientFailoverInterest2
    testThinClientFailoverRegex
    testThinClientFixedPartitionResolver
    testThinClientHADistOps
    testThinClientHAEventIDMap
    testThinClientHAFailover
    testThinClientHAMixedRedundancy
    testThinClientHAQueryFailover
    testThinClientLRUExpiration
    testThinClientLargePutAllWithCallBackArg
    testThinClientLocatorFailover
    testThinClientMultiDS
    testThinClientPRPutAllFailover
    testThinClientPartitionResolver
    testThinClientPdxDeltaWithNotification
    testThinClientPdxTests
    testThinClientPoolRedundancy
    testThinClientPoolServer
    testThinClientPutAll
    testThinClientPutAllPRSingleHop
    testThinClientPutAllWithCallBackArgWithoutConcurrency
    testThinClientSecurityCQAuthorizationMU
    testThinClientTXFailover
    testThinClientTransactionsXA
    testTimedSemaphore
  PROPERTIES
    DISABLED TRUE
    LABELS FLAKY
)

set_tests_properties(testFwPerf
    testThinClientCqDurable
    testThinClientGatewayTest
    testThinClientHAFailoverRegex
    testThinClientPRSingleHop
    testThinClientPoolAttrTest
    testThinClientPoolLocator
    testThinClientPutWithDelta
    testThinClientRemoteQueryTimeout
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
   LABELS OMITTED
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
