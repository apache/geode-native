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

project(Apache.Geode.Client.UnitTests CSharp )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)

file(GLOB_RECURSE SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cs")
file(GLOB_RECURSE XML_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.xml")
set(RESOURCES
  ${XML_SOURCE}
  geode.properties.mixed
  geode.properties.nativeclient
  system.properties
)

add_library(Apache.Geode.Client.UnitTests SHARED
  NUnitTestAddTests.ps1
  packages.config
  test.runsettings.in
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
    DUnitFramework
    NewTestObject
    PdxClassLibrary
    PdxVersion1Lib
    PdxVersion2Lib
    QueryWrapper
    SecurityUtil
)

set_target_properties(Apache.Geode.Client.UnitTests PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE "Apache.Geode.Client.UnitTests"
  VS_GLOBAL_TreatWarningsAsErrors False
  VS_GLOBAL_IsTestProject True
  VS_GLOBAL_TestProjectType UnitTest
  VS_GLOBAL_RunSettingsFilePath "\$(MSBuildProjectDirectory)\\test.runsettings"
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCES "System;System.Xml;System.Web;System.Configuration;System.Management"
  VS_DOTNET_REFERENCE_NUnit "${CMAKE_BINARY_DIR}/packages/NUnit.2.6.4/lib/nunit.framework.dll"
  FOLDER cli/test/integration
)

add_dependencies(Apache.Geode.Client.UnitTests FwkClient SqLiteImpl)
add_dependencies(Apache.Geode.Client.UnitTests nuget-restore)


configure_file(test.runsettings.in test.runsettings.gen)
file(GENERATE OUTPUT test.runsettings INPUT ${CMAKE_CURRENT_BINARY_DIR}/test.runsettings.gen)

enable_testing()

set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/NUnit")
set(ctest_include_file "${ctest_file_base}_include.cmake")
set(ctest_tests_file "${ctest_file_base}_tests.cmake")

add_custom_command(
  TARGET Apache.Geode.Client.UnitTests POST_BUILD
  BYPRODUCTS "${ctest_tests_file}"
  COMMAND pwsh -File 
          "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/NUnitTestAddTests.ps1>"
          -AssemblyPath "$<SHELL_PATH:$<TARGET_FILE:Apache.Geode.Client.UnitTests>>"
          -NUnitConsolePath "$<SHELL_PATH:${CMAKE_BINARY_DIR}/packages/NUnit.Runners.2.6.4/tools/nunit-console.exe>"
          -WorkingDirectory "$<TARGET_FILE_DIR:Apache.Geode.Client.UnitTests>"
          -CTestFile "$<SHELL_PATH:${ctest_tests_file}>"
          -SourcePath "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
          -ServerLogLevel "config"
          -ServerSecurityLogLevel "config"
          -ServerClasspath "$<SHELL_PATH:${CMAKE_BINARY_DIR}/tests/javaobject/javaobject.jar>"
          -GeodePath "$<SHELL_PATH:${Geode_PATH}>"
          -JavaPath "$<SHELL_PATH:${Java_JAVA_EXECUTABLE}>"
          -EnvPath "$<SHELL_PATH:${CMAKE_BINARY_DIR}/packages/NUnit.Runners.2.6.4/tools>"
  WORKING_DIRECTORY "$<TARGET_FILE_DIR:Apache.Geode.Client.UnitTests>"
)

file(WRITE "${ctest_include_file}"
  "if(EXISTS \"${ctest_tests_file}\")\n"
  "  include(\"${ctest_tests_file}\")\n"
  "else()\n"
  "  add_test(${TARGET}_NOT_BUILT ${TARGET}_NOT_BUILT)\n"
  "endif()\n"
)

set_property(DIRECTORY
  APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}"
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

