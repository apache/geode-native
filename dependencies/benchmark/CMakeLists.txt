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

project( benchmark VERSION 1.6.0 LANGUAGES NONE )

set( SHA256 3da225763533aa179af8438e994842be5ca72e4a7fed4d7976dc66c8c4502f58 )
set( DEPENDS GTest::gtest )


if(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
endif()

set( EXTERN ${PROJECT_NAME}-extern )
include(ExternalProject)
ExternalProject_Add( ${EXTERN}
  URL "https://github.com/google/benchmark/archive/v${PROJECT_VERSION}.zip"
  URL_HASH SHA256=${SHA256}
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>/$<CONFIG>
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -DGTEST_ROOT=$<TARGET_PROPERTY:GTest::gtest,INTERFACE_INCLUDE_DIRECTORIES>/..
    -DBENCHMARK_ENABLE_TESTING=OFF
  CMAKE_CACHE_ARGS
    -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT:STRING=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
  DEPENDS ${DEPENDS}
)

ExternalProject_Get_Property( ${EXTERN} SOURCE_DIR )
ExternalProject_Get_Property( ${EXTERN} INSTALL_DIR )
set(INSTALL_DIR "${INSTALL_DIR}/$<CONFIG>")

if (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  set(PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/patches)
  ExternalProject_Add_Step(${EXTERN} patches
    ALWAYS 0
    DEPENDEES download
    DEPENDERS patch
    DEPENDS ${PATCH_FILE}
    BYPRODUCTS ${SOURCE_DIR}/CMakeLists.txt
    WORKING_DIRECTORY ${SOURCE_DIR}
    COMMAND ${Patch_EXECUTABLE} -u -N -p1 < ${PATCH_FILE}
  )
endif()

find_package(Threads REQUIRED)

add_library(benchmark_benchmark INTERFACE)
target_include_directories(benchmark_benchmark SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(benchmark_benchmark INTERFACE
  ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}benchmark${CMAKE_STATIC_LIBRARY_SUFFIX}
  Threads::Threads
)
if (WIN32)
  target_link_libraries(benchmark_benchmark INTERFACE
    Shlwapi
  )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_link_libraries(benchmark_benchmark INTERFACE
    rt
  )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
  target_link_libraries(benchmark_benchmark INTERFACE
    kstat
  )
endif()
add_dependencies(benchmark_benchmark ${EXTERN})

add_library(benchmark::benchmark ALIAS benchmark_benchmark)
