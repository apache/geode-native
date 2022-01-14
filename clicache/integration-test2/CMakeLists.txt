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

cmake_minimum_required( VERSION 3.9 )
project(Apache.Geode.IntegrationTests2 CSharp)

set(CMAKE_CSharp_FLAGS "/langversion:5")

get_target_property(JAVAOBJECT_JAR_PATH javaobject JAR_FILE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cs.in ${CMAKE_CURRENT_BINARY_DIR}/Config.cs)

add_library( Apache.Geode.IntegrationTests2 SHARED
  Framework.cs
  Cluster.cs
  ClusterTest.cs
  Config.cs.in
  ${CMAKE_CURRENT_BINARY_DIR}/Config.cs
  DataInputTest.cs
  GfshTest.cs
  GfshExecuteTest.cs
  Gfsh.cs
  GfshExecute.cs
  CacheXml.cs
  CacheXmlTests.cs
  CqOperationTest.cs
  FunctionExecutionTest.cs
  GarbageCollectCache.cs
  QueryTest.cs
  RegionTest.cs
  RegionSSLTest.cs
  Position.cs
  PositionKey.cs
  TestBase.cs
  cache.xml
  geode.properties
  xunit.runner.json
  xUnitTestAddTests.ps1
  packages.config
  AutoSerializationTests.cs
  SerializationTests.cs
)

set_source_files_properties(
  cache.xml
  server.xml
  xunit.runner.json
  geode.properties
  PROPERTIES
    VS_COPY_TO_OUT_DIR PreserveNewest
    VS_TOOL_OVERRIDE None
)

target_link_libraries(Apache.Geode.IntegrationTests2
  PUBLIC
    Apache.Geode
    PdxClassLibrary
)

set_target_properties(Apache.Geode.IntegrationTests2 PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE Apache.Geode.IntegrationTests2
  VS_GLOBAL_TreatWarningsAsErrors True
  VS_GLOBAL_IsTestProject True
  VS_GLOBAL_TestProjectType UnitTest
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCES "System;System.Xml;System.Web;System.Configuration"
  VS_DOTNET_REFERENCE_xunit.core "${CMAKE_BINARY_DIR}/packages/xunit.extensibility.core.2.4.0/lib/netstandard1.1/xunit.core.dll"
  VS_DOTNET_REFERENCE_xunit.assert "${CMAKE_BINARY_DIR}/packages/xunit.assert.2.4.0/lib/netstandard1.1/xunit.assert.dll"
  VS_DOTNET_REFERENCE_xunit.abstractions "${CMAKE_BINARY_DIR}/packages/xunit.abstractions.2.0.2/lib/net35/xunit.abstractions.dll"
  VS_DOTNET_REFERENCE_xunit.execution.desktop "${CMAKE_BINARY_DIR}/packages/xunit.extensibility.execution.2.4.0/lib/net452/xunit.execution.desktop.dll"
  VS_DOTNET_REFERENCE_xunit.runner.visualstudio.testadapter "${CMAKE_BINARY_DIR}/packages/xunit.runner.visualstudio.2.4.0/build/_common/xunit.runner.visualstudio.testadapter.dll"
  FOLDER cli/test/integration
)

add_dependencies(Apache.Geode.IntegrationTests2 nuget-restore)

enable_testing()

set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/xUnit")
set(ctest_include_file "${ctest_file_base}_include.cmake")
set(ctest_tests_file "${ctest_file_base}_tests.cmake")

add_custom_command(
  TARGET Apache.Geode.IntegrationTests2 POST_BUILD
  BYPRODUCTS "${ctest_tests_file}"
  COMMAND pwsh -File 
          "$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/xUnitTestAddTests.ps1>"
          -AssemblyPath "$<SHELL_PATH:$<TARGET_FILE:Apache.Geode.IntegrationTests2>>"
          -XUnitConsolePath "$<SHELL_PATH:${CMAKE_BINARY_DIR}/packages/xunit.runner.console.2.4.0/tools/net452/xunit.console.exe>"
          -CTestFile "$<SHELL_PATH:${ctest_tests_file}>"
  WORKING_DIRECTORY "$<TARGET_FILE_DIR:Apache.Geode.IntegrationTests2>"
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
