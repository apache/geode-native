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

project( gtest VERSION 1.11.0 LANGUAGES NONE )

set( SHA256 353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a )

include(ExternalProject)
ExternalProject_Add( ${PROJECT_NAME}-extern
  URL "https://github.com/google/googletest/archive/release-${PROJECT_VERSION}.zip"
  URL_HASH SHA256=${SHA256}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -Dgtest_force_shared_crt:BOOL=ON
  CMAKE_CACHE_ARGS
    -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT:STRING=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
)

ExternalProject_Get_Property( ${PROJECT_NAME}-extern INSTALL_DIR )

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

include(GNUInstallDirs)

function(ADD_GTEST_LIBRARY)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs DEPENDENCIES)
  cmake_parse_arguments(PARSE_ARGV 1 args "${options}" "${oneValueArgs}" "${multiValueArgs}")

  set(args_NAME ${ARGV0})

  set(target "GTest_${args_NAME}")

  add_library(${target} INTERFACE)

  target_include_directories(${target} SYSTEM INTERFACE
    $<BUILD_INTERFACE:${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(${target} INTERFACE
    ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${args_NAME}$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${args_DEPENDENCIES}
  )

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} INTERFACE
      -Wno-used-but-marked-unused # GTest uses unused attribute incorrectly
    )
  endif()

  add_library(GTest::${args_NAME} ALIAS ${target})

  add_dependencies(${target} ${PROJECT_NAME}-extern)

endfunction()

find_package(Threads REQUIRED)

add_gtest_library(gtest DEPENDENCIES Threads::Threads)
add_gtest_library(gtest_main DEPENDENCIES GTest::gtest)
add_gtest_library(gmock DEPENDENCIES GTest::gtest)
add_gtest_library(gmock_main DEPENDENCIES GTest::gmock)
