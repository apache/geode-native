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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
endif()

ExternalProject_Add( ${${PROJECT_NAME}_EXTERN}
   URL ${${PROJECT_NAME}_URL}
   URL_HASH SHA256=${${PROJECT_NAME}_SHA265}
   UPDATE_COMMAND ""
   INSTALL_COMMAND ""
   CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -Dgtest_force_shared_crt:BOOL=ON -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=$<CONFIG>
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )

add_library(${PROJECT_NAME}_gtest INTERFACE)
target_include_directories(${PROJECT_NAME}_gtest SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(${PROJECT_NAME}_gtest INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_dependencies(${PROJECT_NAME}_gtest ${${PROJECT_NAME}_EXTERN})

add_library(${PROJECT_NAME}_gtest_main INTERFACE)
target_include_directories(${PROJECT_NAME}_gtest_main SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(${PROJECT_NAME}_gtest_main INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_dependencies(${PROJECT_NAME}_gtest_main ${${PROJECT_NAME}_EXTERN})

add_library(GTest::GTest ALIAS ${PROJECT_NAME}_gtest)
add_library(GTest::Main ALIAS ${PROJECT_NAME}_gtest_main)
