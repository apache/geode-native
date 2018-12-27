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

project( openssl LANGUAGES NONE )

set( VERSION 1.1.1a )
set( SHA265 fc20130f8b7cbd2fb918b2f14e2f429e109c31ddd0fb38fc5d71d9ffed3f9f41 )

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  # No debug for Solaris without patching Configure
  if ("i386" STREQUAL ${CMAKE_SYSTEM_PROCESSOR})
    if (64 EQUAL ${BUILD_BITS})
      set( openssl_PLATFORM solaris64-x86_64-cc )
    else()
      set( openssl_PLATFORM solaris-x86-cc )
    endif()
  elseif ("sparc" STREQUAL ${CMAKE_SYSTEM_PROCESSOR})
    if (64 EQUAL ${BUILD_BITS})
      set( openssl_PLATFORM solaris64-sparcv9-cc )
    else()
      set( openssl_PLATFORM solaris-sparcv9-cc )
    endif()
  endif()
elseif ("Linux" STREQUAL ${CMAKE_SYSTEM_NAME})
  if (64 EQUAL ${BUILD_BITS})
    set( openssl_PLATFORM $<$<CONFIG:Debug>:debug->linux-x86_64 )
  else()
    set( openssl_PLATFORM $<$<CONFIG:Debug>:debug->linux-elf )
    set( openssl_CONFIGURE_FLAGS ${openssl_CONFIGURE_FLAGS} -m32 )
  endif()
  if ("Clang" STREQUAL ${CMAKE_CXX_COMPILER_ID})
    set( openssl_PLATFORM ${openssl_PLATFORM}-clang )
  endif()
elseif ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME})
  # No debug for OS X without patching Configure
  set( openssl_PLATFORM darwin64-x86_64-cc )
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME})
  if (64 EQUAL ${BUILD_BITS})
    set( openssl_PLATFORM $<$<CONFIG:Debug>:debug->VC-WIN64A no-asm )
  else()
    set( openssl_PLATFORM $<$<CONFIG:Debug>:debug->VC-WIN32 no-asm)
  endif()
endif()

if (NOT DEFINED openssl_PLATFORM)
  message( FATAL_ERROR "openssl_PLATFORM unset for ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}" )
endif()

if (${WIN32})
  find_package(Perl REQUIRED)
  # Keeps separate release/debug objects in build script
  set ( _CONFIGURE_COMMAND ${PERL_EXECUTABLE} Configure ${openssl_PLATFORM} --prefix=<INSTALL_DIR>/$<CONFIG> --openssldir=<INSTALL_DIR>/$<CONFIG> ${openssl_CONFIGURE_FLAGS} )
  set ( _BUILD_COMMAND nmake )
  set ( _INSTALL_COMMAND nmake install_sw )
else()
   # TODO Configure trips up without MAKE
  set ( _CONFIGURE_COMMAND MAKE=$(MAKE) ./Configure threads zlib shared --prefix=<INSTALL_DIR>/$<CONFIG> ${openssl_CONFIGURE_FLAGS} ${openssl_PLATFORM} )
  set ( _BUILD_COMMAND $(MAKE) all )
  set ( _INSTALL_COMMAND $(MAKE) install_sw )
endif()

set( EXTERN ${PROJECT_NAME}-extern )
include(ExternalProject)
ExternalProject_Add( ${EXTERN}
   URL "https://www.openssl.org/source/openssl-${VERSION}.tar.gz"
   URL_HASH SHA256=${SHA265}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND "${_CONFIGURE_COMMAND}"
   BUILD_COMMAND "${_BUILD_COMMAND}"
   INSTALL_COMMAND "${_INSTALL_COMMAND}"
# TODO   TEST_COMMAND $(MAKE) test
)

ExternalProject_Get_Property( ${EXTERN} INSTALL_DIR )
set(INSTALL_DIR "${INSTALL_DIR}/$<CONFIG>")

if (${WIN32})
  set( CMAKE_SHARED_LIBRARY_PREFIX lib )
else()
  set( CMAKE_LINK_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

add_library(OpenSSL_SSL INTERFACE)
target_include_directories(OpenSSL_SSL SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(OpenSSL_SSL INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}ssl${CMAKE_LINK_LIBRARY_SUFFIX}
)
add_dependencies(OpenSSL_SSL ${EXTERN})

add_library(OpenSSL_Crypto INTERFACE)
target_include_directories(OpenSSL_Crypto SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(OpenSSL_Crypto INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}crypto${CMAKE_LINK_LIBRARY_SUFFIX}
)
add_dependencies(OpenSSL_Crypto ${EXTERN})

set_target_properties(OpenSSL_SSL OpenSSL_Crypto PROPERTIES
  INTERFACE_LIBRARY_DIR ${INSTALL_DIR}/lib
  INTERFACE_RUNTIME_DIR ${INSTALL_DIR}/bin
)

add_library(OpenSSL::SSL ALIAS OpenSSL_SSL)
add_library(OpenSSL::Crypto ALIAS OpenSSL_Crypto)
