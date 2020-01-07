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

project( ACE VERSION 6.5.5 LANGUAGES NONE )

set( SHA256 dc70f1422e5d880b7d2cb59483873acfe6918bb1456c7e36e8b510d9e7769f3b )

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  set( ACE_PLATFORM sunos5_sunc++ )
  set( ACE_CONFIG sunos${CMAKE_SYSTEM_VERSION} )
elseif ("Linux" STREQUAL ${CMAKE_SYSTEM_NAME} )
  if ("Clang" STREQUAL ${CMAKE_CXX_COMPILER_ID})
    set( ACE_PLATFORM linux_clang )
  else()
    set( ACE_PLATFORM linux )
  endif()
  set( ACE_CONFIG linux )
  ## TODO fix visibility problem with g++
  set( _MAKE_ARGS ${_MAKE_ARGS} no_hidden_visibility=1 )
elseif ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME} )
  set( ACE_PLATFORM macosx_sierra )
  set( ACE_CONFIG macosx-sierra )
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME} )
  set( ACE_PLATFORM win32-msvc )
  set( ACE_CONFIG win32 )
endif()

if (NOT DEFINED ACE_PLATFORM)
  message( FATAL_ERROR "ACE_PLATFORM unset for ${CMAKE_SYSTEM_NAME}" )
endif()

find_package(OpenSSL REQUIRED)
set( OPENSSL_ROOT ${OPENSSL_INCLUDE_DIR}/.. )

if (${WIN32})
  if (64 EQUAL ${BUILD_BITS})
    set( _PLATFORM x64 )
  else()
    set( _PLATFORM win32 )
  endif()
  if (MSVC_VERSION EQUAL 1900)
    set(_TYPE "vc14")
  elseif((MSVC_VERSION GREATER_EQUAL 1910) AND (MSVC_VERSION LESS_EQUAL 1919))
    set(_TYPE "vs2017")
  else()
    set(_TYPE "vs2019")
  endif()

  set ( _COMMAND_PREFIX ${CMAKE_COMMAND} -E chdir ace )

  set ( _MSBUILD_FLAGS /m /p:Platform=${_PLATFORM} /p:Configuration=$<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>)
  set ( _MSBUILD_FLAGS_STATIC ${_MSBUILD_FLAGS} /t:ace_${_TYPE}_static /t:SSL_${_TYPE}_static )

  include(ProcessorCount)
  ProcessorCount(_NPROCS)

  find_package(Perl REQUIRED)

  set ( MPC ${PERL_EXECUTABLE} ..\\bin\\mwc.pl )
  set ( MPC_FLAGS -type ${_TYPE} -expand_vars -features ssl=1 -recurse -hierarchy -workers ${_NPROCS} -value_template MultiProcessorCompilation=true -apply_project )

  #TODO add custom targets to build in parallel?
  #TODO MPC has flag to replace variables with absolute paths, think we need that for devstudio builds
  set ( _CONFIGURE_COMMAND ${_COMMAND_PREFIX}
                             ${CMAKE_COMMAND} -E env ACE_ROOT=<SOURCE_DIR> SSL_ROOT=${OPENSSL_ROOT}
							 ${MPC} ${MPC_FLAGS} -name_modifier "*_${_TYPE}_static" -static
							 -value_template staticflags+=__ACE_INLINE__ 
							 -value_template staticflags+=ACE_BUILD_DLL 
							 -value_template staticflags+=ACE_AS_STATIC_LIBS 
							 -value_template runtime_library=Multithreaded$<$<CONFIG:Debug>:Debug>Dll 
  )
  set ( _BUILD_COMMAND ${_COMMAND_PREFIX} ${CMAKE_VS_MSBUILD_COMMAND} ACE_${_TYPE}_static.sln ${_MSBUILD_FLAGS} ${_MSBUILD_FLAGS_STATIC}
  )
  set ( _INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory lib <INSTALL_DIR>/lib 
                 COMMAND ${CMAKE_COMMAND} -E copy_directory ace <INSTALL_DIR>/include/ace
  )

  set(CMAKE_STATIC_LIBRARY_SUFFIX s$<${MSVC}:$<$<CONFIG:Debug>:d>>.lib)

else()
  # TODO Configure trips up without MAKE
  # TODO look into using cmake -E chdir
  # TODO use cmake -E env

  set( _MAKE cd ace && ACE_ROOT=<SOURCE_DIR> SSL_ROOT=${OPENSSL_ROOT} $(MAKE) SHELL=/bin/bash debug=$<CONFIG:Debug> optimize=$<NOT:$<CONFIG:Debug>> buildbits=${BUILD_BITS} c++11=1 static_libs_only=1 ssl=1 ${_MAKE_ARGS})
  set ( _CONFIGURE_COMMAND "" )
  set ( _BUILD_COMMAND ${_MAKE} all )
  set ( _INSTALL_COMMAND ${_MAKE} install )
endif()

string(REPLACE "." "_" _VERSION_UNDERSCORE ${PROJECT_VERSION})

set( EXTERN ${PROJECT_NAME}-extern )
include(ExternalProject)
ExternalProject_Add( ${EXTERN}
   URL "https://github.com/DOCGroup/ACE_TAO/releases/download/ACE%2BTAO-${_VERSION_UNDERSCORE}/ACE-${PROJECT_VERSION}.tar.gz"
   URL_HASH SHA256=${SHA256}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND "${_CONFIGURE_COMMAND}"
   BUILD_COMMAND "${_BUILD_COMMAND}"
   INSTALL_COMMAND "${_INSTALL_COMMAND}"
   DEPENDS ${DEPENDS}
)

ExternalProject_Get_Property( ${EXTERN} SOURCE_DIR )
set( SOURCE_DIR ${SOURCE_DIR} )
ExternalProject_Get_Property( ${EXTERN} INSTALL_DIR )
set( INSTALL_DIR ${INSTALL_DIR} )
set( DEPENDENCIES_DIR ${INSTALL_DIR} PARENT_SCOPE)

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
set( PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/patches )
ExternalProject_Add_Step( ${EXTERN} patches
  ALWAYS 0
  DEPENDEES download
  DEPENDERS patch
  DEPENDS ${PATCH_FILE}
  BYPRODUCTS ${SOURCE_DIR}/include/makeinclude/platform_sunos5_sunc++.GNU ${SOURCE_DIR}/ace/config-lite.h ${SOURCE_DIR}/ace/Stats.cpp
  WORKING_DIRECTORY ${SOURCE_DIR}
  COMMAND ${Patch_EXECUTABLE} -u -N -p1 < ${PATCH_FILE}
)
endif()

configure_file(config.h.in config.h)
ExternalProject_Add_Step( ${EXTERN} config.h
  DEPENDEES patch
  DEPENDERS configure
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/config.h
  BYPRODUCTS ${SOURCE_DIR}/ace/config.h
  COMMAND ${CMAKE_COMMAND} -E copy config.h ${SOURCE_DIR}/ace/config.h
)

if (UNIX)
ExternalProject_Add_Step( ${EXTERN} platform
  DEPENDEES patch
  DEPENDERS configure
  BYPRODUCTS ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
  COMMAND echo "INSTALL_PREFIX = ${INSTALL_DIR}" > ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
  COMMAND echo "CC = ${CMAKE_C_COMPILER}" >> ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
  COMMAND echo "CXX = ${CMAKE_CXX_COMPILER}" >> ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
  COMMAND echo "CCFLAGS += ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_COMPILE_OPTIONS_PIC}" >> ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
  COMMAND echo "include ${SOURCE_DIR}/include/makeinclude/platform_${ACE_PLATFORM}.GNU" >> ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
)
endif()

find_package(Threads)

add_library(ACE_ACE INTERFACE)
target_include_directories(ACE_ACE SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_compile_definitions(ACE_ACE INTERFACE
  __ACE_INLINE__
  ACE_AS_STATIC_LIBS
  __STDC_FORMAT_MACROS # ACE slurps in inttypes.h before cinttypes and clobbers these macros
)
target_link_libraries(ACE_ACE INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ACE${CMAKE_STATIC_LIBRARY_SUFFIX}
  Threads::Threads
)

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  target_link_libraries(ACE_ACE INTERFACE
    rt
    m
    socket
    nsl
  )
  target_compile_definitions(ACE_ACE INTERFACE
    _POSIX_PTHREAD_SEMANTICS
  )

elseif ("Linux" STREQUAL ${CMAKE_SYSTEM_NAME} )
  target_link_libraries(ACE_ACE INTERFACE
    dl
    rt
  )
elseif ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME} )
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME} )
endif()

add_dependencies(ACE_ACE ${EXTERN})

add_library(ACE_ACE_SSL INTERFACE)
target_link_libraries(ACE_ACE_SSL INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ACE_SSL${CMAKE_STATIC_LIBRARY_SUFFIX}
  ACE_ACE
  OpenSSL::SSL
  OpenSSL::Crypto
)

add_library(ACE::ACE ALIAS ACE_ACE)
add_library(ACE::ACE_SSL ALIAS ACE_ACE_SSL)
