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

project( openssl C )

set( ${PROJECT_NAME}_VERSION 1.1.0 )
set( ${PROJECT_NAME}_SHA265 f5c69ff9ac1472c80b868efc1c1c0d8dcfc746d29ebe563de2365dd56dbd8c82 )
set( ${PROJECT_NAME}_URL "https://www.openssl.org/source/openssl-${${PROJECT_NAME}_VERSION}.tar.gz" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )

set ( ${PROJECT_NAME}_CFLAGS "${${PROJECT_NAME}_CFLAGS} ${CFLAGS_ARCH}" )

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
  # Keeps separate release/debug objects in build script
  set ( _CONFIGURE_COMMAND ${PERL} Configure ${openssl_PLATFORM} --prefix=<INSTALL_DIR>/${_DEBUG_OR_RELEASE} --openssldir=<INSTALL_DIR>/${_DEBUG_OR_RELEASE} ${openssl_CONFIGURE_FLAGS} )
  set ( _BUILD_COMMAND nmake )
  set ( _INSTALL_COMMAND nmake install )
else()
   # TODO Configure trips up without MAKE
  set ( _CONFIGURE_COMMAND MAKE=$(MAKE) ./Configure threads zlib shared --prefix=<INSTALL_DIR>/${_DEBUG_OR_RELEASE} ${openssl_CONFIGURE_FLAGS} ${openssl_PLATFORM} )
  set ( _BUILD_COMMAND $(MAKE) all )
  set ( _INSTALL_COMMAND $(MAKE) install_sw )
endif()

include(ExternalProject)
ExternalProject_Add( ${${PROJECT_NAME}_EXTERN}
   URL ${${PROJECT_NAME}_URL}
   URL_HASH SHA256=${${PROJECT_NAME}_SHA265}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND "${_CONFIGURE_COMMAND}"
   BUILD_COMMAND "${_BUILD_COMMAND}"
   INSTALL_COMMAND "${_INSTALL_COMMAND}"
# TODO   TEST_COMMAND $(MAKE) test
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
set( ${PROJECT_NAME}_SOURCE_DIR ${SOURCE_DIR} )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )
set( ${PROJECT_NAME}_INSTALL_DIR ${INSTALL_DIR}/${_DEBUG_OR_RELEASE} )
set( DEPENDENCIES_${PROJECT_NAME}_DIR ${${PROJECT_NAME}_INSTALL_DIR} PARENT_SCOPE)

if (${WIN32})
  set( CRYPTO_NAME libcrypto )
  set( SSL_NAME libssl )
else()
  set( CRYPTO_NAME crypto )
  set( SSL_NAME ssl )
  set( CMAKE_LINK_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

add_library(ssl INTERFACE)
target_include_directories(ssl INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/include>
)
target_link_libraries(ssl INTERFACE
  ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${SSL_NAME}${CMAKE_LINK_LIBRARY_SUFFIX}
)
add_dependencies(ssl ${${PROJECT_NAME}_EXTERN})

add_library(crypto INTERFACE)
target_include_directories(crypto INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/include>
)
target_link_libraries(crypto INTERFACE
  ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${CRYPTO_NAME}${CMAKE_LINK_LIBRARY_SUFFIX}
)
add_dependencies(crypto ${${PROJECT_NAME}_EXTERN})

set_target_properties(ssl crypto PROPERTIES INTERFACE_LIBRARY_DIR ${${PROJECT_NAME}_INSTALL_DIR}/lib)
set_target_properties(ssl crypto PROPERTIES INTERFACE_RUNTIME_DIR ${${PROJECT_NAME}_INSTALL_DIR}/bin)

