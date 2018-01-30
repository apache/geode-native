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

cmake_minimum_required(VERSION 3.4)
project(Apache.Geode.Test CXX)

file(GLOB SOURCES "*.cpp")

if(NOT "${STRONG_NAME_PUBLIC_KEY}" STREQUAL "")
  set(STRONG_NAME_PUBLIC_KEY_ATTRIBUTE ", PublicKey=${STRONG_NAME_PUBLIC_KEY}")
endif()
list(APPEND CONFIGURE_IN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cpp.in)
list(APPEND CONFIGURE_OUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/AssemblyInfo.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/AssemblyInfo.cpp)

set_source_files_properties(${CONFIGURE_OUT_FILES} PROPERTIES GENERATED TRUE)

add_library(${PROJECT_NAME} MODULE
  ${SOURCES}
  ${CONFIGURE_IN_FILES}
  ${CONFIGURE_OUT_FILES}
  ${RESOURCES}
)
#add_dependencies(unit-tests ${PROJECT_NAME})

add_definitions(-DUNICODE -D_UNICODE)
target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    -DUNICODE -D_UNICODE
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Apache.Geode
  PUBLIC
    c++11
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  COMPILE_FLAGS "/clr"
  VS_GLOBAL_CLRSupport "true"
  VS_GLOBAL_KEYWORD "ManagedCProj"
  VS_GLOBAL_TestProjectType UnitTest
  VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2"
  VS_DOTNET_REFERENCES "System;System.Xml;Microsoft.VisualStudio.QualityTools.UnitTestFramework"
)

string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SHARED_LINKER_FLAGS_STRONG_KEY}")

include_directories(${CMAKE_SOURCE_DIR}/clicache/src)
include_directories(${CMAKE_SOURCE_DIR}/cppcache/src)
include_directories(${CMAKE_SOURCE_DIR}/cppcache/include)
#PCH issues
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# For Visual Studio organization
source_group("Header Files" REGULAR_EXPRESSION "\.(hpp|inl)$")
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})
