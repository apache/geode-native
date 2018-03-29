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

project( gtest LANGUAGES NONE )

set( ${PROJECT_NAME}_VERSION 1.8.0 )
set( ${PROJECT_NAME}_SHA265 f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf )
set( ${PROJECT_NAME}_URL "https://github.com/google/googletest/archive/release-${${PROJECT_NAME}_VERSION}.zip" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )

include(ExternalProject)

if(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  set(SUN_COMPILER_FLAGS "-DCMAKE_CXX_FLAGS=-std=c++11 -m64")
endif()

ExternalProject_Add( ${${PROJECT_NAME}_EXTERN}
   URL ${${PROJECT_NAME}_URL}
   URL_HASH SHA256=${${PROJECT_NAME}_SHA265}
   UPDATE_COMMAND ""
   INSTALL_COMMAND ""
   CMAKE_ARGS "${SUN_COMPILER_FLAGS}" -Dgtest_force_shared_crt:BOOL=ON
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
set( ${PROJECT_NAME}_gtest_SOURCE_DIR ${SOURCE_DIR}/googletest )
set( ${PROJECT_NAME}_gmock_SOURCE_DIR ${SOURCE_DIR}/googlemock )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )
set( ${PROJECT_NAME}_INSTALL_DIR ${INSTALL_DIR} )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} BINARY_DIR )
set( ${PROJECT_NAME}_gtest_BINARY_DIR ${BINARY_DIR}/googlemock/gtest/${_DEBUG_OR_RELEASE} )
set( ${PROJECT_NAME}_gmock_BINARY_DIR ${BINARY_DIR}/googlemock/${_DEBUG_OR_RELEASE} )
set( DEPENDENCIES_${PROJECT_NAME}_DIR ${${PROJECT_NAME}_BINARY_DIR} PARENT_SCOPE)

set( ${PROJECT_NAME}_STATIC_LIB
${${PROJECT_NAME}_gtest_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
${${PROJECT_NAME}_gtest_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
${${PROJECT_NAME}_gmock_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX}
${${PROJECT_NAME}_gmock_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock_main${CMAKE_STATIC_LIBRARY_SUFFIX}
PARENT_SCOPE)

add_library(${PROJECT_NAME}_gtest INTERFACE)
target_include_directories(${PROJECT_NAME}_gtest INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_gtest_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${${PROJECT_NAME}_gmock_SOURCE_DIR}/include>
)
target_link_libraries(${PROJECT_NAME}_gtest INTERFACE
  ${${PROJECT_NAME}_gtest_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_dependencies(${PROJECT_NAME}_gtest ${${PROJECT_NAME}_EXTERN})

add_library(${PROJECT_NAME}_gtest_main INTERFACE)
target_include_directories(${PROJECT_NAME}_gtest_main INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_gtest_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${${PROJECT_NAME}_gmock_SOURCE_DIR}/include>
)
target_link_libraries(${PROJECT_NAME}_gtest_main INTERFACE
  ${${PROJECT_NAME}_gtest_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_dependencies(${PROJECT_NAME}_gtest_main ${${PROJECT_NAME}_EXTERN})

add_library(GTest::GTest ALIAS ${PROJECT_NAME}_gtest)
add_library(GTest::Main ALIAS ${PROJECT_NAME}_gtest_main)
