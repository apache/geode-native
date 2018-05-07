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

project( benchmark LANGUAGES NONE )

set( ${PROJECT_NAME}_VERSION 1.4.0 )
set( ${PROJECT_NAME}_SHA265 7f5f3608c9228fa023151a4b54e91f4ada4b7b49c26facede6c5b8b83ddbedad )
set( ${PROJECT_NAME}_URL "https://github.com/google/benchmark/archive/v${${PROJECT_NAME}_VERSION}.zip" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )
set( ${PROJECT_NAME}_DEPENDS gtest_gtest )

include(ExternalProject)

if (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  set(SUN_COMPILER_FLAGS "-DCMAKE_CXX_FLAGS=-m64")
endif()

ExternalProject_Add( ${${PROJECT_NAME}_EXTERN}
   URL ${${PROJECT_NAME}_URL}
   URL_HASH SHA256=${${PROJECT_NAME}_SHA265}
   UPDATE_COMMAND ""
   CMAKE_ARGS "${SUN_COMPILER_FLAGS}"
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	  -DCMAKE_BUILD_TYPE=$<CONFIG>
      -DGTEST_ROOT=$<TARGET_PROPERTY:GTest::GTest,INTERFACE_INCLUDE_DIRECTORIES>/..
   DEPENDS ${${PROJECT_NAME}_DEPENDS}
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )

if (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  set(PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/patches)
  ExternalProject_Add_Step(${${PROJECT_NAME}_EXTERN} patches
    ALWAYS 0
    DEPENDEES download
    DEPENDERS patch
    DEPENDS ${PATCH_FILE}
    BYPRODUCTS ${SOURCE_DIR}/CMakeLists.txt
    WORKING_DIRECTORY ${SOURCE_DIR}
    COMMAND ${PATCH} -u -N -p1 < ${PATCH_FILE}
  )
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(${PROJECT_NAME} INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}benchmark${CMAKE_STATIC_LIBRARY_SUFFIX}
)
if (WIN32)
target_link_libraries(${PROJECT_NAME} INTERFACE
  Shlwapi
)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  target_link_libraries(${PROJECT_NAME} INTERFACE
    kstat
  )
endif()
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXTERN})

add_library(benchmark::benchmark ALIAS ${PROJECT_NAME})
