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
project(Apache.Geode.Test CXX)

set(vstest_dir ${CMAKE_BINARY_DIR}/packages/Microsoft.TestPlatform.16.8.3/tools/net451/Common7/IDE/Extensions/TestPlatform)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)

if(NOT "${STRONG_NAME_PUBLIC_KEY}" STREQUAL "")
  set(STRONG_NAME_PUBLIC_KEY_ATTRIBUTE ", PublicKey=${STRONG_NAME_PUBLIC_KEY}")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/AssemblyInfo.cpp)
list(APPEND CONFIGURE_IN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cpp.in)
list(APPEND CONFIGURE_OUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/AssemblyInfo.cpp)

add_library(Apache.Geode.Test MODULE
  AssemblyInfo.cpp.in
  native_conditional_unqiue_ptrTests.cpp
  native_shared_ptrTests.cpp
  native_unique_ptrTests.cpp
  TimeUtilsTests.cpp
  ${CONFIGURE_OUT_FILES}
  packages.config
)

set_target_properties(Apache.Geode.Test PROPERTIES
  COMMON_LANGUAGE_RUNTIME "" 
  VS_GLOBAL_CLRSupport "true"
  VS_GLOBAL_KEYWORD "ManagedCProj"
  VS_GLOBAL_IsTestProject True
  VS_GLOBAL_TestProjectType UnitTest
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
  VS_GLOBAL_ROOTNAMESPACE Apache.Geode.Test
  VS_DOTNET_REFERENCES "System;System.Xml"
  VS_DOTNET_REFERENCE_Microsoft.VisualStudio.QualityTools.UnitTestFramework "${vstest_dir}/Microsoft.VisualStudio.QualityTools.UnitTestFramework.dll"
  FOLDER cli/test/unit
)

target_include_directories(Apache.Geode.Test
  PRIVATE
  	$<TARGET_PROPERTY:apache-geode,SOURCE_DIR>
  	$<TARGET_PROPERTY:apache-geode,INCLUDE_DIRECTORIES>
  	$<TARGET_PROPERTY:Apache.Geode,SOURCE_DIR>
)

target_link_libraries(Apache.Geode.Test
  PUBLIC
    # Apache.Geode #- Causes include of .lib
  PRIVATE
    c++cli
	c++11
	_WarningsAsError
)

# Makes project only reference, no .lib.
add_dependencies(Apache.Geode.Test Apache.Geode)
add_dependencies(Apache.Geode.Test nuget-restore)

string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SHARED_LINKER_FLAGS_STRONG_KEY}")

# For Visual Studio organization
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})

enable_testing()

add_test(NAME Apache.Geode.Test
  COMMAND ${vstest_dir}/vstest.console.exe
          $<SHELL_PATH:$<TARGET_FILE:Apache.Geode.Test>>
          --nologo --parallel
  WORKING_DIRECTORY "$<TARGET_FILE_DIR:Apache.Geode.Test>"
)
