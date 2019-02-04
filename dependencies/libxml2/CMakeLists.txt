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

project( libxml2 VERSION 2.9.9 LANGUAGES NONE )

set( SHA256 94fb70890143e3c6549f265cee93ec064c80a84c42ad0f23e85ee1fd6540a871 )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64" )

if (${WIN32})
  set (_MAKE_FLAGS ${_MAKE_FLAGS} "CPP=cl.exe /MP /EP" "CC=cl.exe /MP" )
  set (_MAKE_FLAGS ${_MAKE_FLAGS}
                          DEBUG=$<STREQUAL:$<CONFIG>,Debug>
                          CRUNTIME=/MD$<$<CONFIG:Debug>:d>
                          XML_SO=libxml2$<$<CONFIG:Debug>:d>.dll
                          XML_IMP=libxml2$<$<CONFIG:Debug>:d>.lib
                          XML_A=libxml2_a$<$<CONFIG:Debug>:d>.lib
                          XML_A_DLL=libxml2_a_dll$<$<CONFIG:Debug>:d>.lib
                          XML_INTDIR=int$<$<CONFIG:Debug>:d>.msvc
                          XML_INTDIR_A=int$<$<CONFIG:Debug>:d>.a.msvc
                          XML_INTDIR_A_DLL=int$<$<CONFIG:Debug>:d>.a.dll.msvc
                          UTILS_INTDIR=int$<$<CONFIG:Debug>:d>.utils.msvc )

  # http://hostagebrain.blogspot.com/2015/06/building-libxml2-without-dependencies.html
  set ( _CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir win32 cscript configure.js iconv=no compiler=msvc cruntime=/MD$<$<CONFIG:Debug>:d> debug=$<$<CONFIG:Debug>:yes>$<$<NOT:$<CONFIG:Debug>>:no> prefix=..\\..\\.. static=yes )
  set ( _BUILD_COMMAND ${CMAKE_COMMAND} -E chdir win32 nmake -f Makefile.msvc all ${_MAKE_FLAGS} )
  set ( _INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir win32 nmake -f Makefile.msvc install ${_MAKE_FLAGS} )
else()
  set ( _CONFIGURE_COMMAND ./configure
          --prefix=<INSTALL_DIR>
          $<$<CONFIG:Debug>:--with-debug>
          --libdir=<INSTALL_DIR>/lib
          --with-pic
          --enable-static
          --without-iconv
          --without-python
          --without-lzma
          CC=${CMAKE_C_COMPILER}
          CFLAGS=${CMAKE_C_FLAGS}
          "MAKE=$(MAKE)")
  set ( _BUILD_COMMAND $(MAKE) all )
  set ( _INSTALL_COMMAND $(MAKE) install )
endif()

set( EXTERN ${PROJECT_NAME}-extern )
include(ExternalProject)
ExternalProject_Add( ${EXTERN}
   URL "http://xmlsoft.org/sources/libxml2-${PROJECT_VERSION}.tar.gz"
   URL_HASH SHA256=${SHA256}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND "${_CONFIGURE_COMMAND}"
   BUILD_COMMAND "${_BUILD_COMMAND}"
   INSTALL_COMMAND "${_INSTALL_COMMAND}"
)

ExternalProject_Get_Property( ${EXTERN} INSTALL_DIR )
set( INSTALL_DIR ${INSTALL_DIR} )

if (${WIN32})
  set(CMAKE_STATIC_LIBRARY_PREFIX lib)
  set(CMAKE_STATIC_LIBRARY_SUFFIX _a$<$<CONFIG:Debug>:d>.lib)
endif()

add_library(LibXml2_LibXml2 INTERFACE)
target_include_directories(LibXml2_LibXml2 SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include/libxml2>
)
target_link_libraries(LibXml2_LibXml2 INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}xml2${CMAKE_STATIC_LIBRARY_SUFFIX}
)
if (${UNIX})
  target_link_libraries(LibXml2_LibXml2 INTERFACE
    z
  )
endif()
add_dependencies(LibXml2_LibXml2 ${EXTERN})

add_library(LibXml2::LibXml2 ALIAS LibXml2_LibXml2)
