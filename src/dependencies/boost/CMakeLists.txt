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

project( boost )

set( ${PROJECT_NAME}_VERSION 1.64.0 )
set( ${PROJECT_NAME}_SHA265 0445c22a5ef3bd69f5dfb48354978421a85ab395254a26b1ffb0aa1bfd63a108 )
string(REPLACE "." "_" _VERSION_UNDERSCORE ${${PROJECT_NAME}_VERSION})
set( ${PROJECT_NAME}_URL "https://dl.bintray.com/boostorg/release/${${PROJECT_NAME}_VERSION}/source/boost_${_VERSION_UNDERSCORE}.tar.gz" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )

include(ProcessorCount)
ProcessorCount(_NPROCS)

include(ExternalProject)

set( _B2_FLAGS -d0 --prefix=<INSTALL_DIR> --with-system address-model=${BUILD_BITS} link=static )

if (${_NPROCS})
  set ( _B2_FLAGS ${_B2_FLAGS} -j${_NPROCS} )
endif()

if (${WIN32})
  set ( _BOOTSTRAP_COMMAND .\\bootstrap.bat )
  set ( _B2_COMMAND .\\b2 )
else()
  set ( _BOOTSTRAP_COMMAND ./bootstrap.sh )
  set ( _B2_COMMAND ./b2 )
  set ( _B2_FLAGS ${_B2_FLAGS} cxxflags=-std=c++11 )
endif()

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  set ( _BOOTSTRAP_COMMAND ${_BOOTSTRAP_COMMAND} --with-toolset=sun )
endif()

ExternalProject_Add( ${${PROJECT_NAME}_EXTERN}
   URL ${${PROJECT_NAME}_URL}
   URL_HASH SHA256=${${PROJECT_NAME}_SHA265}
   UPDATE_COMMAND ""
   BUILD_IN_SOURCE 1
   CONFIGURE_COMMAND ${_BOOTSTRAP_COMMAND}
   BUILD_COMMAND ${_B2_COMMAND} ${_B2_FLAGS}
   INSTALL_COMMAND ${_B2_COMMAND} ${_B2_FLAGS} install
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
set( ${PROJECT_NAME}_SOURCE_DIR ${SOURCE_DIR} )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )
set( ${PROJECT_NAME}_INSTALL_DIR ${INSTALL_DIR} )

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/include>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
)
target_link_libraries(${PROJECT_NAME} INTERFACE
)

add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXTERN})
