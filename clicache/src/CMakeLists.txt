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
project(Apache.Geode)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

file(GLOB_RECURSE PRIVATE_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.hpp")
set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

set(RESOURCES Apache.Geode.rc)

if(NOT "${STRONG_NAME_PUBLIC_KEY}" STREQUAL "")
  set(STRONG_NAME_PUBLIC_KEY_ATTRIBUTE ", PublicKey=${STRONG_NAME_PUBLIC_KEY}")
endif()
list(APPEND CONFIGURE_IN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/impl/AssemblyInfo.cpp.in)
list(APPEND CONFIGURE_OUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/impl/AssemblyInfo.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/impl/AssemblyInfo.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/impl/AssemblyInfo.cpp)

#set_source_files_properties(${CONFIGURE_IN_FILES} PROPERTIES LANGUAGE NONE)
set_source_files_properties(${CONFIGURE_OUT_FILES} PROPERTIES GENERATED TRUE)

add_library(${PROJECT_NAME} SHARED
  ${SOURCES}
  ${PRIVATE_HEADERS} ${CONFIGURE_IN_FILES} ${CONFIGURE_OUT_FILES} ${RESOURCES})
add_dependencies(client-libraries ${PROJECT_NAME})

#TODO get external project library names
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    apache-geode-static
    psapi
  PUBLIC
    c++11
)

string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clr /bigobj /wd4947 /wd4251 /wd4635 /doc /we4488")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SHARED_LINKER_FLAGS_STRONG_KEY}")

include_directories(${CMAKE_SOURCE_DIR}/cppcache/src)

set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME ${PRODUCT_DLL_NAME}
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2"
  VS_DOTNET_REFERENCES "System;System.Xml"
)

include(PrecompiledHeader)
add_precompiled_header(${PROJECT_NAME} geode_includes.hpp FORCEINCLUDE)

add_subdirectory(templates)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
)

# For Visual Studio organization
source_group("Header Files" REGULAR_EXPRESSION "\.(hpp|inl)$")
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})
