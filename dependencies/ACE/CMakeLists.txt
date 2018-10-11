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

project( ACE LANGUAGES NONE )

set( ${PROJECT_NAME}_VERSION 6.4.7 )
set( ${PROJECT_NAME}_SHA265 b2b59a5f49b164121a8d1c109ff04fc23fcd67cfad040772e8c045c8a34cdabe )
string(REPLACE "." "_" _VERSION_UNDERSCORE ${${PROJECT_NAME}_VERSION})
set( ${PROJECT_NAME}_URL "https://github.com/DOCGroup/ACE_TAO/releases/download/ACE%2BTAO-${_VERSION_UNDERSCORE}/ACE.tar.gz" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )
set( ${PROJECT_NAME}_DEPENDS openssl-extern )

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

include(ExternalProject)

set( OPENSSL_ROOT ${DEPENDENCIES_openssl_DIR} )


if (${WIN32})
  if (64 EQUAL ${BUILD_BITS})
    set( _PLATFORM x64 )
  else()
    set( _PLATFORM win32 )
  endif()

  set ( _COMMAND_PREFIX ${CMAKE_COMMAND} -E chdir ace )

  set ( _MSBUILD_FLAGS /m /p:Platform=${_PLATFORM} /p:Configuration=$<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>)
  set ( _MSBUILD_FLAGS_STATIC ${_MSBUILD_FLAGS} /t:ace_vc${MSVC_VERSION}_static /t:SSL_vc${MSVC_VERSION}_static ) 

  include(ProcessorCount)
  ProcessorCount(_NPROCS)
  set ( MPC ${PERL} ..\\bin\\mwc.pl )
  set ( MPC_FLAGS -type vc${MSVC_VERSION} -expand_vars -features ssl=1 -recurse -hierarchy -workers ${_NPROCS} -value_template MultiProcessorCompilation=true -apply_project )

  #TODO add custom targets to build in parallel?
  #TODO MPC has flag to replace variables with absolute paths, think we need that for devstudio builds
  set ( _CONFIGURE_COMMAND ${_COMMAND_PREFIX}
                             ${CMAKE_COMMAND} -E env ACE_ROOT=<SOURCE_DIR> SSL_ROOT=${OPENSSL_ROOT}
							 ${MPC} ${MPC_FLAGS} -name_modifier "*_vc${MSVC_VERSION}_static" -static 
							 -value_template staticflags+=__ACE_INLINE__ 
							 -value_template staticflags+=ACE_BUILD_DLL 
							 -value_template staticflags+=ACE_AS_STATIC_LIBS 
							 -value_template runtime_library=Multithreaded$<$<CONFIG:Debug>:Debug>Dll 
  )
  set ( _BUILD_COMMAND ${_COMMAND_PREFIX} ${MSBUILD} ACE_vc${MSVC_VERSION}_static.sln ${_MSBUILD_FLAGS} ${_MSBUILD_FLAGS_STATIC}
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

ExternalProject_Add( ${${PROJECT_NAME}_EXTERN}
   URL ${${PROJECT_NAME}_URL}
   URL_HASH SHA256=${${PROJECT_NAME}_SHA265}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND "${_CONFIGURE_COMMAND}"
   BUILD_COMMAND "${_BUILD_COMMAND}"
   INSTALL_COMMAND "${_INSTALL_COMMAND}"
   DEPENDS ${${PROJECT_NAME}_DEPENDS}
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
set( ${PROJECT_NAME}_SOURCE_DIR ${SOURCE_DIR} )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )
set( ${PROJECT_NAME}_INSTALL_DIR ${INSTALL_DIR} )
set( DEPENDENCIES_${PROJECT_NAME}_DIR ${${PROJECT_NAME}_INSTALL_DIR} PARENT_SCOPE)

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
set( PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/patches )
ExternalProject_Add_Step( ${${PROJECT_NAME}_EXTERN} patches
    ALWAYS 0
    DEPENDEES download
    DEPENDERS patch
    DEPENDS ${PATCH_FILE}
    BYPRODUCTS ${${PROJECT_NAME}_SOURCE_DIR}/include/makeinclude/platform_sunos5_sunc++.GNU ${${PROJECT_NAME}_SOURCE_DIR}/ace/config-lite.h ${${PROJECT_NAME}_SOURCE_DIR}/ace/Stats.cpp
    WORKING_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR}
    COMMAND ${PATCH} -u -N -p1 < ${PATCH_FILE}
)
endif()

configure_file(config.h.in config.h)
ExternalProject_Add_Step( ${${PROJECT_NAME}_EXTERN} config.h
    DEPENDEES patch
    DEPENDERS configure
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/config.h
    BYPRODUCTS ${${PROJECT_NAME}_SOURCE_DIR}/ace/config.h
    COMMAND ${CMAKE_COMMAND} -E copy config.h ${${PROJECT_NAME}_SOURCE_DIR}/ace/config.h
)

if (UNIX)
ExternalProject_Add_Step( ${${PROJECT_NAME}_EXTERN} platform
    DEPENDEES patch
    DEPENDERS configure
    BYPRODUCTS ${${PROJECT_NAME}_SOURCE_DIR}/include/makeinclude/platform_macros.GNU
    COMMAND echo "INSTALL_PREFIX = ${${PROJECT_NAME}_INSTALL_DIR}" > ${${PROJECT_NAME}_SOURCE_DIR}/include/makeinclude/platform_macros.GNU
    COMMAND echo "CCFLAGS += ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_COMPILE_OPTIONS_PIC}" >> ${${PROJECT_NAME}_SOURCE_DIR}/include/makeinclude/platform_macros.GNU
    COMMAND echo "include ${${PROJECT_NAME}_SOURCE_DIR}/include/makeinclude/platform_${ACE_PLATFORM}.GNU" >> ${${PROJECT_NAME}_SOURCE_DIR}/include/makeinclude/platform_macros.GNU
)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/include>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
	-D__ACE_INLINE__
	-DACE_AS_STATIC_LIBS
)
target_link_libraries(${PROJECT_NAME} INTERFACE
    ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ACE${CMAKE_STATIC_LIBRARY_SUFFIX}
)

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  target_link_libraries(${PROJECT_NAME} INTERFACE
    rt
    pthread
    m
    socket
    nsl
  )
  target_compile_definitions( ${PROJECT_NAME} INTERFACE
    _POSIX_PTHREAD_SEMANTICS
  )

elseif ("Linux" STREQUAL ${CMAKE_SYSTEM_NAME} )
  target_link_libraries(${PROJECT_NAME} INTERFACE
    pthread
    dl
    rt
  )
elseif ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME} )
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME} )
endif()

add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXTERN})

add_library(ACE_SSL INTERFACE)
target_link_libraries(ACE_SSL INTERFACE
    ${PROJECT_NAME}
    ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ACE_SSL${CMAKE_STATIC_LIBRARY_SUFFIX}
)
