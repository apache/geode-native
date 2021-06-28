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

project(ACE VERSION 7.0.1 LANGUAGES NONE)

set(SHA256 a28339750620c70cd29a8a7088a4bc6ebaf1ff7ba667498a0279ac97f0e32e01)

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
elseif ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME})
  set(ACE_PLATFORM macosx)
  set(ACE_CONFIG macosx)
  foreach(arch ${CMAKE_OSX_ARCHITECTURES})
    string(APPEND CMAKE_CXX_FLAGS " -arch ${arch}")
    string(APPEND CMAKE_STATIC_LINKER_FLAGS " -arch ${arch}")
  endforeach()
  string(APPEND CMAKE_CXX_FLAGS " ${CMAKE_CXX_SYSROOT_FLAG} ${CMAKE_OSX_SYSROOT}")
  string(APPEND CMAKE_CXX_FLAGS " ${CMAKE_CXX_OSX_DEPLOYMENT_TARGET_FLAG}${CMAKE_OSX_DEPLOYMENT_TARGET}")
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME} )
  set( ACE_PLATFORM win32-msvc )
  set( ACE_CONFIG win32 )
endif()

if (NOT DEFINED ACE_PLATFORM)
  message( FATAL_ERROR "ACE_PLATFORM unset for ${CMAKE_SYSTEM_NAME}" )
endif()

set(ENV_COMMAND ${CMAKE_COMMAND} -E env ACE_ROOT=<SOURCE_DIR>)

if (${WIN32})
  if (64 EQUAL ${BUILD_BITS})
    set( _PLATFORM x64 )
  else()
    set( _PLATFORM win32 )
  endif()
  if (MSVC_TOOLSET_VERSION EQUAL 140)
    set(MPC_TYPE "vc14")
  elseif(MSVC_TOOLSET_VERSION EQUAL 141)
    set(MPC_TYPE "vs2017")
  elseif(MSVC_TOOLSET_VERSION GREATER_EQUAL 142)
    set(MPC_TYPE "vs2019")
  endif()

  set ( _COMMAND_PREFIX ${CMAKE_COMMAND} -E chdir ace )

  set ( _MSBUILD_FLAGS /m /p:Platform=${_PLATFORM} /p:Configuration=$<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>)
  set ( _MSBUILD_FLAGS_STATIC ${_MSBUILD_FLAGS} /t:ace_${MPC_TYPE}_static )

  include(ProcessorCount)
  ProcessorCount(_NPROCS)

  find_package(Perl REQUIRED)

  set ( MPC ${PERL_EXECUTABLE} ../bin/mwc.pl )
  set ( MPC_FLAGS -type ${MPC_TYPE} -recurse -hierarchy -workers ${_NPROCS} -apply_project )

  #TODO add custom targets to build in parallel?
  #TODO MPC has flag to replace variables with absolute paths, think we need that for devstudio builds
  set ( _CONFIGURE_COMMAND ${MPC} -static ${MPC_FLAGS}
                           -name_modifier "*_${MPC_TYPE}_static"
                           -value_template MultiProcessorCompilation=true
                           -value_template WindowsTargetPlatformVersion=${CMAKE_SYSTEM_VERSION}
                           -value_template staticflags+=__ACE_INLINE__
                           -value_template staticflags+=ACE_BUILD_DLL
                           -value_template staticflags+=ACE_AS_STATIC_LIBS
                           -value_template runtime_library=Multithreaded$<$<CONFIG:Debug>:Debug>Dll
  )
  set ( _BUILD_COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ACE_${MPC_TYPE}_static.sln ${_MSBUILD_FLAGS} ${_MSBUILD_FLAGS_STATIC}
  )
  set ( _INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib <INSTALL_DIR>/lib
                 COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/ace <INSTALL_DIR>/include/ace
  )

  set(CMAKE_STATIC_LIBRARY_SUFFIX s$<${MSVC}:$<$<CONFIG:Debug>:d>>.lib)

else()
  # TODO Configure trips up without MAKE
  set( _MAKE $(MAKE) SHELL=/bin/bash debug=$<CONFIG:Debug> optimize=$<NOT:$<CONFIG:Debug>> buildbits=${BUILD_BITS} c++11=1 static_libs_only=1 ${_MAKE_ARGS})
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
   SOURCE_SUBDIR "ace"
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND "${_CONFIGURE_COMMAND}"
   BUILD_COMMAND ${ENV_COMMAND} ${_BUILD_COMMAND}
   INSTALL_COMMAND ${ENV_COMMAND} ${_INSTALL_COMMAND}
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
  COMMAND echo "LDFLAGS += ${CMAKE_STATIC_LINKER_FLAGS}" >> ${SOURCE_DIR}/include/makeinclude/platform_macros.GNU
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

add_library(ACE::ACE ALIAS ACE_ACE)
