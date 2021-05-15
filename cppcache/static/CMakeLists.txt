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

project(apache-geode-static LANGUAGES CXX)

add_library(apache-geode-static STATIC ${SOURCES_ALL})

generate_export_file( "apache-geode-static" )


set_source_files_properties(${CONFIGURE_OUT_FILES} PROPERTIES GENERATED TRUE)

target_compile_definitions(apache-geode-static PUBLIC
  APACHE_GEODE_STATIC_DEFINE
)

target_link_libraries(apache-geode-static
  PUBLIC
    _apache-geode
  PRIVATE
    _WarningsAsError
    _CppCodeCoverage)

target_include_directories(apache-geode-static
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_definitions(apache-geode-static PUBLIC GEODE_WITH_LOG_FMT)

add_dependencies(_apache-geode version-header)

add_dependencies(client-libraries apache-geode-static)

set_target_properties(apache-geode-static PROPERTIES
  FOLDER cpp
)

if (MSVC)
  cmake_minimum_required(VERSION 3.13)
  set_target_properties(apache-geode-static PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE
    INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO FALSE
  )
endif()

set_target_properties(apache-geode-static PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)

# BEGIN Visual Studio organization
source_group("Header Files" REGULAR_EXPRESSION "\.(hpp|inl)$")
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})
# END Visual Studio organization

if (USE_PCH)
  set(_COTIRE_PREFIX_HEADER_IGNORE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND _COTIRE_PREFIX_HEADER_IGNORE_PATH
      /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/9.0.0/include/unwind.h
      ${CMAKE_BINARY_DIR}/dependencies/boost/include/boost/stacktrace
      ${CMAKE_BINARY_DIR}/dependencies/boost/include/boost/stacktrace.hpp)
  endif()

  set_target_properties(apache-geode-static PROPERTIES
    COTIRE_ENABLE_PRECOMPILED_HEADER TRUE
    COTIRE_ADD_UNITY_BUILD TRUE
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${_COTIRE_PREFIX_HEADER_IGNORE_PATH}"
    COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_BINARY_DIR}/dependencies;${CMAKE_CURRENT_SOURCE_DIR}/../include")

  set_source_files_properties(cppcache/version.h PROPERTIES GENERATED TRUE)

  cotire(apache-geode-static)
endif()

add_clangformat(apache-geode-static)
