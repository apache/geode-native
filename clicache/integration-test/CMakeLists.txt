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

project(nativeclient.tests.clicache)

set (NUNIT "C:\\Program Files (x86)\\NUnit 2.6.4")

if (64 EQUAL ${BUILD_BITS})
  set (NUNIT_CONSOLE "nunit-console")
else()
  set (NUNIT_CONSOLE "nunit-console-x86")
endif()


foreach(var CMAKE_CURRENT_SOURCE_DIR CMAKE_SOURCE_DIR CMAKE_BINARY_DIR CMAKE_CURRENT_BINARY_DIR)
  file(TO_NATIVE_PATH ${${var}} ${var}_NATIVE)
endforeach()

file(GLOB_RECURSE CSPROJECTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.csproj.in")
if(EXISTS "${STRONG_NAME_KEY}")
  set(STRONG_NAME_KEY_ENABLED true)
endif()

foreach(FILE ${CSPROJECTS})
  # TODO: make this a one line MATCH statement
  string(REGEX REPLACE "\\.csproj.in" "" PROJDIR ${FILE})
  string(REGEX REPLACE ".*/" "" PROJNAME ${PROJDIR})
  configure_file(${FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROJNAME}.csproj)
  include_external_msproject(
      ${PROJNAME}  ${CMAKE_CURRENT_BINARY_DIR}/${PROJNAME}.csproj
      TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
  set_target_properties(${PROJNAME} PROPERTIES FOLDER cli/test/integration)

endforeach()

enable_testing()

# Function to resolve both config and generate stage variables.
macro(generate_config INPUT TEMP OUTPUT)
    configure_file(${INPUT} ${TEMP})
    file(GENERATE OUTPUT ${OUTPUT}
      INPUT ${TEMP}
      CONDITION 1
    )
endmacro()

foreach( lib SQLite::sqlite3 )
  get_target_property(runtime_path ${lib} INTERFACE_RUNTIME_DIR)
  set(PATH ${PATH} ${runtime_path})
endforeach()

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cs")

foreach(FILE ${SOURCES})
  # Find all non-deprecated tests
  file(STRINGS ${FILE} HasTests REGEX "\\[Test\\]")

  LIST(LENGTH HasTests length)
  if (${length} GREATER 0)
    # Get the namespace
    file(STRINGS ${FILE} NameSpaceLine REGEX "namespace Apache.Geode.Client.UnitTests")
    string(REGEX REPLACE "namespace.*Apache" "Apache" NAMESPACE ${NameSpaceLine})

    string(REGEX REPLACE "\\.cs" "" TEST ${FILE})
    set(TESTS ${TESTS} ${TEST})
    set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/.tests/${TEST})

    # Get the test class (NewAPI tests have N in the file name but not test class name)
    string(REGEX REPLACE "N$" "" TESTCLASS ${TEST})

    set(TEST_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${TEST}.bat)
    generate_config(${CMAKE_CURRENT_SOURCE_DIR}/test.bat.in ${CMAKE_CURRENT_BINARY_DIR}/.${TEST}.bat.in ${TEST_SCRIPT})

    add_test(NAME ${TEST} COMMAND ${TEST_SCRIPT})
    set_tests_properties(${TEST} PROPERTIES LABELS STABLE)

    file(STRINGS ${FILE} IsDeprecated REGEX "\\[Category.*deprecated.*\\]")
    if (IsDeprecated)
      set_tests_properties(${TEST} PROPERTIES DISABLED TRUE)
	  endif()
  endif()
endforeach()

# Label any flaky tests here
set_tests_properties(ThinClientCqStatusTestTwoServers
    ThinClientPoolTestsN
    ThinClientQueryTestsN
  PROPERTIES
    DISABLED TRUE
    LABELS FLAKY
)

# Label any tests that always fail here
set_tests_properties(OverflowTestsN
    ThinClientDeltaTestFailing
    ThinClientDurableTestsN
    ThinClientHARegionTestsN
    ThinClientSecurityAuthTestsMUN
    ThinClientSecurityAuthTestsN
    RegionFailingTests
    ThinClientSecurityAuthzTestsMUN
    ThinClientSecurityAuthzTestsN
  PROPERTIES
    DISABLED TRUE
    LABELS OMITTED
)

add_custom_target(run-stable-clicache-integration-tests
  COMMAND ctest -C $<CONFIGURATION> -L STABLE
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL)
add_dependencies(run-integration-tests run-stable-clicache-integration-tests)
set_target_properties(run-stable-clicache-integration-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
  FOLDER cli/test/integration)

