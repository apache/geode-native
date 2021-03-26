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
project( Apache.Geode.DockerAcceptanceTests CSharp )

set(CMAKE_CSharp_FLAGS "/langversion:5")

get_target_property(JAVAOBJECT_JAR_PATH javaobject JAR_FILE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cs.in ${CMAKE_CURRENT_BINARY_DIR}/Config.cs)

add_library( ${PROJECT_NAME} SHARED
  Config.cs.in
  ${CMAKE_CURRENT_BINARY_DIR}/Config.cs
  TestBase.cs
  cache.xml
  xunit.runner.json
  packages.config
  SNITests.cs
)

set_source_files_properties(
  cache.xml
  server.xml
  xunit.runner.json
  geode.properties
  PROPERTIES
    VS_COPY_TO_OUT_DIR Always
    VS_TOOL_OVERRIDE None
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Apache.Geode
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_GLOBAL_TreatWarningsAsErrors True
  VS_GLOBAL_TestProjectType UnitTest
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCES "System;System.Xml;System.Web;System.Configuration"
  VS_DOTNET_REFERENCE_xunit.core "${CMAKE_BINARY_DIR}/packages/xunit.extensibility.core.2.4.0/lib/netstandard1.1/xunit.core.dll"
  VS_DOTNET_REFERENCE_xunit.assert "${CMAKE_BINARY_DIR}/packages/xunit.assert.2.4.0/lib/netstandard1.1/xunit.assert.dll"
  VS_DOTNET_REFERENCE_xunit.abstractions "${CMAKE_BINARY_DIR}/packages/xunit.abstractions.2.0.2/lib/net35/xunit.abstractions.dll"
  VS_DOTNET_REFERENCE_xunit.execution.desktop "${CMAKE_BINARY_DIR}/packages/xunit.extensibility.execution.2.4.0/lib/net452/xunit.execution.desktop.dll"
  VS_DOTNET_REFERENCE_xunit.runner.visualstudio.testadapter "${CMAKE_BINARY_DIR}/packages/xunit.runner.visualstudio.2.4.0/build/_common/xunit.runner.visualstudio.testadapter.dll"
  FOLDER cli/test/acceptance
)

if(NOT "${STRONG_NAME_KEY}" STREQUAL "")
  set_target_properties( ${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_SignAssembly "true"
    VS_GLOBAL_AssemblyOriginatorKeyFile ${STRONG_NAME_KEY}
  )
endif()

add_dependencies(${PROJECT_NAME} nuget-restore)
