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

project( boost LANGUAGES NONE )

set( ${PROJECT_NAME}_VERSION 1.65.1 )
set( ${PROJECT_NAME}_SHA265 a13de2c8fbad635e6ba9c8f8714a0e6b4264b60a29b964b940a22554705b6b60 )
string(REPLACE "." "_" _VERSION_UNDERSCORE ${${PROJECT_NAME}_VERSION})
set( ${PROJECT_NAME}_URL "https://dl.bintray.com/boostorg/release/${${PROJECT_NAME}_VERSION}/source/boost_${_VERSION_UNDERSCORE}.tar.gz" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )

include(ProcessorCount)
ProcessorCount(_NPROCS)

include(ExternalProject)

set( _B2_FLAGS -d0 --prefix=<INSTALL_DIR> --with-system --with-log address-model=${BUILD_BITS} link=static threading=multi)

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
   PREFIX .
   DOWNLOAD_DIR .
   SOURCE_DIR ./src
   STAMP_DIR ./stamp
)

ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
set( ${PROJECT_NAME}_SOURCE_DIR ${SOURCE_DIR} )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )
set( ${PROJECT_NAME}_INSTALL_DIR ${INSTALL_DIR} )

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  #
  # Solaris patch is to workaround issue of dladdr function signature
  # mismatching the signature boost stacktrace expects. Following boost ticket
  # tracks this issue:
  #
  #     https://svn.boost.org/trac10/ticket/13300#ticket
  #
  set( PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/solaris.patch )
  ExternalProject_Add_Step( ${${PROJECT_NAME}_EXTERN} patches
      ALWAYS 0
      DEPENDEES download
      DEPENDERS patch
      DEPENDS ${PATCH_FILE}
      WORKING_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR}
      COMMAND ${PATCH} -u -N -p1 < ${PATCH_FILE}
  )
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/include>
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/src>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
)
target_link_libraries(${PROJECT_NAME} INTERFACE
)
add_library(Boost::boost ALIAS boost)
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXTERN})

add_library(boost_system INTERFACE)
target_link_libraries(boost_system INTERFACE
  ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_system${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_library(Boost::system ALIAS boost_system)

add_library(boost_thread INTERFACE)
target_link_libraries(boost_thread INTERFACE
  Boost::boost
  ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_thread${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_library(Boost::thread ALIAS boost_thread)

add_library(boost_log INTERFACE)
target_link_libraries(boost_log INTERFACE
  Boost::boost
  Boost::thread
  ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_log${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_log_setup${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_library(Boost::log ALIAS boost_log)
