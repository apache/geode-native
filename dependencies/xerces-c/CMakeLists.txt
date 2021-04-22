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

project( xerces-c VERSION 3.2.3 LANGUAGES NONE )

set( SHA256 fb96fc49b1fb892d1e64e53a6ada8accf6f0e6d30ce0937956ec68d39bd72c7e)
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )

include(GNUInstallDirs)
include(ExternalProject)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(Xerces_TRANSCODER "-Dtranscoder=iconv")
endif()

# Xerces-C++ is very warningful
string(APPEND CMAKE_C_FLAGS " -w")
string(APPEND CMAKE_CXX_FLAGS " -w")

ExternalProject_Add( ${PROJECT_NAME}-extern
  URL "https://www.apache.org/dyn/closer.lua/xerces/c/3/sources/xerces-c-${PROJECT_VERSION}.tar.gz?action=download"
  URL_HASH SHA256=${SHA256}
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DBUILD_SHARED_LIBS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -Dnetwork:BOOL=OFF
    ${Xerces_TRANSCODER}
  CMAKE_CACHE_ARGS
    -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT:STRING=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
)

ExternalProject_Get_Property( ${PROJECT_NAME}-extern INSTALL_DIR )

if (${WIN32})
  set(LIBRARY_NAME xerces-c_3$<$<CONFIG:Debug>:D>)
else()
  set(LIBRARY_NAME xerces-c)
endif()

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
  ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${LIBRARY_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
)

if ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME})
  target_link_libraries(${PROJECT_NAME} INTERFACE
    "-framework CoreServices"
    curl
  )
endif()

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-extern)
add_library(XercesC::XercesC ALIAS xerces-c)

