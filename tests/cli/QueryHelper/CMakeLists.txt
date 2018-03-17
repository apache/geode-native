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

add_library(${PROJECT_NAME} SHARED 
  AssemblyInfo.cpp
  QueryStringsM.cpp
  QueryStringsM.hpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_CLRSupport "true"
  VS_GLOBAL_KEYWORD "ManagedCProj"
  VS_GLOBAL_PROJECT_TYPES "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
  VS_GLOBAL_ROOTNAMESPACE "Apache.Geode.Client.Tests"
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2"
  VS_DOTNET_REFERENCES "System;System.Xml"
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
	# disabled warnings
	/wd4947
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:Apache.Geode,SOURCE_DIR>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:testThinClientRemoteQueryRS,SOURCE_DIR>>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC 
    Apache.Geode
  PRIVATE
    c++cli
    c++11
)

string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SHARED_LINKER_FLAGS_STRONG_KEY}")

# For Visual Studio organization
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER test/cliTests)
