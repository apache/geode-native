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

cmake_minimum_required( VERSION 3.15 )
project(Apache.Geode.Client.UnitTests CSharp )

find_program(NUNIT_CONSOLE "nunit-console" REQUIRED PATH_SUFFIXES "NUnit 2.6.4/bin")
message(STATUS "Found nunit-console: ${NUNIT_CONSOLE}")

file(GLOB_RECURSE SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cs")
file(GLOB_RECURSE XML_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.xml")
set(RESOURCES
  ${XML_SOURCE}
  geode.properties.mixed
  geode.properties.nativeclient
  system.properties
)

add_library(Apache.Geode.Client.UnitTests SHARED
  ${SOURCE}
  ${RESOURCES}
)

source_group("Source Files" FILES "${SOURCE}")
source_group("Resource Files" FILES ${RESOURCES})

set_source_files_properties(
  ${RESOURCES}
  PROPERTIES
    VS_COPY_TO_OUT_DIR PreserveNewest
    VS_TOOL_OVERRIDE None
)

target_link_libraries(Apache.Geode.Client.UnitTests
  PUBLIC
    Apache.Geode
    Apache.Geode.DUnitFramework
    NewTestObject
    FwkClient
    PdxClassLibrary
    PdxVersion1Lib
    PdxVersion2Lib
    QueryWrapper
    SecurityUtil
)

set_target_properties(Apache.Geode.Client.UnitTests PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE "Apache.Geode.Client.UnitTests"
  DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2"
  VS_GLOBAL_TreatWarningsAsErrors False
  VS_GLOBAL_IsTestProject True
  VS_GLOBAL_TestProjectType UnitTest
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCES "System;System.Xml;System.Web;System.Configuration;System.Management"
  VS_PACKAGE_REFERENCES "NUnit_2.6.4"
  FOLDER cli/test/integration
)

if(NOT "${STRONG_NAME_KEY}" STREQUAL "")
  set_target_properties(Apache.Geode.Client.UnitTests PROPERTIES
    VS_GLOBAL_SignAssembly "true"
    VS_GLOBAL_AssemblyOriginatorKeyFile ${STRONG_NAME_KEY}
  )
endif()

enable_testing()

set(PATH
  $<TARGET_LINKER_FILE_DIR:apache-geode>
  $<TARGET_LINKER_FILE_DIR:SqLiteImpl>
  $<TARGET_PROPERTY:SQLite::sqlite3,INTERFACE_LIBRARY_DIR>
  $<TARGET_PROPERTY:SQLite::sqlite3,INTERFACE_RUNTIME_DIR>
  $<TARGET_LINKER_FILE_DIR:unit_test_callbacks>
  $<TARGET_LINKER_FILE_DIR:testobject>
)

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

    add_test(NAME ${TEST}
        COMMAND "$<SHELL_PATH:${NUNIT_CONSOLE}>" /nologo /nodots /timeout 1000000 /labels /run:${NAMESPACE}.${TESTCLASS} "$<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/UnitTests.dll>"
        WORKING_DIRECTORY ${TEST_DIR}
    )

    set(ENVIRONMENT
      "TESTSRC=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
      GFE_LOGLEVEL=config
      GFE_SECLOGLEVEL=config
      "GF_JAVA=$<SHELL_PATH:${Java_JAVA_EXECUTABLE}>"
      "GFE_DIR=$<SHELL_PATH:${Geode_PATH}>"
      "GF_CLASSPATH=$<SHELL_PATH:${CMAKE_BINARY_DIR}/tests/javaobject/javaobject.jar>"
      "PATH=$<JOIN:$<SHELL_PATH:${PATH}>,\\$<SEMICOLON>>"
    )

    set_tests_properties(${TEST} PROPERTIES
      ENVIRONMENT "${ENVIRONMENT}"
    )

    file(STRINGS ${FILE} IsDeprecated REGEX "\\[Category.*deprecated.*\\]")
    if (IsDeprecated)
      set_tests_properties(${TEST} PROPERTIES DISABLED TRUE)
	  endif()
  endif()
endforeach()

set_tests_properties(ThinClientCqStatusTestTwoServers
    # flaky
    ThinClientPoolTestsN
    ThinClientQueryTestsN
    # broken
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

