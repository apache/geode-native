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

project(boost VERSION 1.78.0 LANGUAGES NONE)

set(SHA256 94ced8b72956591c4775ae2207a9763d3600b30d9d7446562c552f0a14a63be7)

if (WIN32)
  set(BOOTSTRAP_COMMAND bootstrap.bat)
else()
  set(BOOTSTRAP_COMMAND ./bootstrap.sh)
endif()
set(B2_COMMAND ./b2)
set(B2_FLAGS
  -d0
  --prefix=<INSTALL_DIR>/$<CONFIG>
  --with-system
  --with-log
  --with-iostreams
  --layout=system
  address-model=${BUILD_BITS}
  link=static
  threading=multi
  variant=$<IF:$<CONFIG:Debug>,debug,release>
  debug-symbols=$<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,on,off>
)

include(ProcessorCount)
ProcessorCount(NPROCS)
if (${NPROCS})
  list(APPEND B2_FLAGS -j${NPROCS})
endif()

if ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME})
  # disabled until combined supports arm list(APPEND B2_FLAGS architecture=combined)
  # manually set arch until combine supports arm
  foreach(arch ${CMAKE_OSX_ARCHITECTURES})
    string(APPEND CMAKE_CXX_FLAGS " -arch ${arch}")
    string(APPEND CMAKE_STATIC_LINKER_FLAGS " -arch ${arch}")
  endforeach()
  string(APPEND CMAKE_CXX_FLAGS " ${CMAKE_CXX_SYSROOT_FLAG} ${CMAKE_OSX_SYSROOT}")
  string(APPEND CMAKE_CXX_FLAGS " ${CMAKE_CXX_OSX_DEPLOYMENT_TARGET_FLAG}${CMAKE_OSX_DEPLOYMENT_TARGET}")
elseif(WIN32)
  list(APPEND B2_FLAGS define=_WIN32_WINNT=${USE_WINAPI_VERSION})
endif()

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  list(APPEND BOOTSTRAP_FLAGS --with-toolset=sun)
  list(APPEND B2_FLAGS define=BOOST_OPTIONAL_DETAIL_NO_RVALUE_REFERENCES)
elseif(MSVC)
  if(CMAKE_GENERATOR_TOOLSET MATCHES "version=([0-9]+\.[0-9]+\.[0-9]+)")
    set(BOOTSTRAP_COMMAND ${CMAKE_COMMAND} -E env VCToolsVersion=${CMAKE_MATCH_1} ${BOOTSTRAP_COMMAND})
    set(B2_COMMAND ${CMAKE_COMMAND} -E env VCToolsVersion=${CMAKE_MATCH_1} ${B2_COMMAND})
  endif()
  list(APPEND BOOTSTRAP_FLAGS vc${MSVC_TOOLSET_VERSION})
  if (CMAKE_VS_PLATFORM_TOOLSET_VERSION)
	list(APPEND B2_FLAGS toolset=msvc-${CMAKE_VS_PLATFORM_TOOLSET_VERSION})
  else()
    list(APPEND B2_FLAGS toolset=msvc-${MSVC_TOOLSET_VERSION})
  endif()
  list(APPEND B2_FLAGS define=BOOST_LOG_NO_CXX11_CODECVT_FACETS)
  set(CMAKE_STATIC_LIBRARY_PREFIX lib)
elseif ("Clang" STREQUAL ${CMAKE_CXX_COMPILER_ID})
  list(APPEND BOOTSTRAP_FLAGS --with-toolset=clang)
endif()

list(APPEND B2_FLAGS "cxxflags=${CMAKE_CXX_FLAGS} ${CMAKE_CXX11_STANDARD_COMPILE_OPTION} ${CMAKE_CXX_COMPILE_OPTIONS_PIC}")

string(REPLACE "." "_" _VERSION_UNDERSCORE ${PROJECT_VERSION})

set(EXTERN ${PROJECT_NAME}-extern)
include(ExternalProject)
ExternalProject_Add(${EXTERN}
   URL "https://boostorg.jfrog.io/artifactory/main/release/${PROJECT_VERSION}/source/boost_${_VERSION_UNDERSCORE}.tar.gz"
       "https://sourceforge.net/projects/boost/files/boost/${PROJECT_VERSION}/boost_${_VERSION_UNDERSCORE}.tar.gz/download"
   URL_HASH SHA256=${SHA256}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND ${BOOTSTRAP_COMMAND} ${BOOTSTRAP_FLAGS}
   BUILD_COMMAND ${B2_COMMAND} ${B2_FLAGS}
   INSTALL_COMMAND ${B2_COMMAND} ${B2_FLAGS} install
   PREFIX .
   DOWNLOAD_DIR .
   SOURCE_DIR ./src
   STAMP_DIR ./stamp
)

ExternalProject_Get_Property(${EXTERN} SOURCE_DIR)
ExternalProject_Get_Property(${EXTERN} INSTALL_DIR)
set(INSTALL_DIR "${INSTALL_DIR}/$<CONFIG>")

function(ADD_BOOST_LIBRARY)
  set(options HEADER_ONLY)
  set(oneValueArgs)
  set(multiValueArgs LINK_LIBRARIES DEFINITIONS INCLUDE_DIRECTORIES DEPENDENCIES)
  cmake_parse_arguments(PARSE_ARGV 1 args "${options}" "${oneValueArgs}" "${multiValueArgs}")

  set(args_NAME ${ARGV0})
  set(targetName boost_${args_NAME})

  set(linkLibrary)
  if (NOT args_HEADER_ONLY)
    set(linkLibrary "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_${args_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()

  add_library(${targetName} INTERFACE)

  target_compile_definitions(${targetName} INTERFACE ${args_DEFINITIONS})

  target_include_directories(${targetName} SYSTEM INTERFACE ${args_INCLUDE_DIRECTORIES})

  target_link_libraries(${targetName} INTERFACE
    ${linkLibrary}
    ${args_LINK_LIBRARIES}
  )

  if(DEFINED args_DEPENDENCIES)
    add_dependencies(${targetName} ${args_DEPENDENCIES})
  endif()

  add_library(Boost::${args_NAME} ALIAS ${targetName})
endfunction()


find_package(Threads REQUIRED)


add_boost_library(boost HEADER_ONLY
  INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${INSTALL_DIR}/include>
  DEFINITIONS
    BOOST_ALL_NO_LIB
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=${USE_WINAPI_VERSION}>
  DEPENDENCIES
    ${EXTERN}
)
add_boost_library(system LINK_LIBRARIES Boost::boost)
add_boost_library(atomic LINK_LIBRARIES Boost::boost)
add_boost_library(iostreams LINK_LIBRARIES Boost::boost)
add_boost_library(thread LINK_LIBRARIES Threads::Threads Boost::atomic Boost::boost)
add_boost_library(filesystem LINK_LIBRARIES Boost::system Boost::boost)
add_boost_library(log 
  LINK_LIBRARIES
    Boost::thread
    Boost::filesystem
    Boost::boost
  DEFINITIONS
    $<$<CXX_COMPILER_ID:MSVC>:BOOST_LOG_NO_CXX11_CODECVT_FACETS>
)
add_boost_library(log_setup LINK_LIBRARIES Boost::log)
add_boost_library(chrono LINK_LIBRARIES Boost::chrono)
add_boost_library(stacktrace HEADER_ONLY 
  LINK_LIBRARIES
    Boost::boost
  DEFINITIONS
    BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
)
add_boost_library(asio HEADER_ONLY
  LINK_LIBRARIES
    Boost::boost
  DEFINITIONS
    BOOST_ASIO_HAS_MOVE
)
add_boost_library(process HEADER_ONLY LINK_LIBRARIES Boost::system)
add_boost_library(regex LINK_LIBRARIES Boost::boost)

if(MSVC)
  target_compile_options(boost_process INTERFACE
  	/wd4267
    /wd4244
  )
endif()
