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
project(QueryWrapper LANGUAGES CXX)

add_library(QueryWrapper SHARED 
  AssemblyInfo.cpp
  QueryStringsM.cpp
  QueryStringsM.hpp
)

set_target_properties(QueryWrapper PROPERTIES
  VS_GLOBAL_CLRSupport "true"
  VS_GLOBAL_KEYWORD "ManagedCProj"
  VS_GLOBAL_PROJECT_TYPES "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
  VS_GLOBAL_ROOTNAMESPACE "Apache.Geode.Client.Tests"
  VS_DOTNET_REFERENCES "System;System.Xml"
)

target_compile_options(QueryWrapper
  PRIVATE
	# disabled warnings
	/wd4947
)

target_include_directories(QueryWrapper
  PRIVATE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:Apache.Geode,SOURCE_DIR>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:testThinClientRemoteQueryRS,SOURCE_DIR>>
)

target_link_libraries(QueryWrapper
  PUBLIC 
    # Apache.Geode #- Causes include of .lib
  PRIVATE
    c++cli
    c++11
	_WarningsAsError
)

# Makes project only reference, no .lib.
add_dependencies(QueryWrapper Apache.Geode)

string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

# For Visual Studio organization
set_target_properties(QueryWrapper PROPERTIES FOLDER cli/test/integration)
