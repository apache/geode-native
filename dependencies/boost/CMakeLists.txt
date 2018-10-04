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

set( ${PROJECT_NAME}_VERSION 1.68.0 )
set( ${PROJECT_NAME}_SHA265 da3411ea45622579d419bfda66f45cd0f8c32a181d84adfa936f5688388995cf )
string(REPLACE "." "_" _VERSION_UNDERSCORE ${${PROJECT_NAME}_VERSION})
set( ${PROJECT_NAME}_URL "https://dl.bintray.com/boostorg/release/${${PROJECT_NAME}_VERSION}/source/boost_${_VERSION_UNDERSCORE}.tar.gz" )
set( ${PROJECT_NAME}_EXTERN ${PROJECT_NAME}-extern )

include(ProcessorCount)
ProcessorCount(_NPROCS)

include(ExternalProject)

set(_B2_FLAGS
  -d0
  --prefix=<INSTALL_DIR>
  --with-system
  --with-log
  --layout=system
  address-model=${BUILD_BITS}
  link=static
  threading=multi
  variant=$<LOWER_CASE:$<CONFIG>>
)

if (${_NPROCS})
  set ( _B2_FLAGS ${_B2_FLAGS} -j${_NPROCS} )
endif()

if (${WIN32})
  set ( _BOOTSTRAP_COMMAND .\\bootstrap.bat )
  set ( _B2_COMMAND .\\b2 )
  set ( _B2_FLAGS ${_B2_FLAGS} toolset=msvc-14.0 )
else()
  set ( _BOOTSTRAP_COMMAND ./bootstrap.sh )
  set ( _B2_COMMAND ./b2 )
  set ( _B2_FLAGS ${_B2_FLAGS} "cxxflags=${CMAKE_CXX_FLAGS} ${CMAKE_CXX11_STANDARD_COMPILE_OPTION}" )
endif()

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  set ( _BOOTSTRAP_COMMAND ${_BOOTSTRAP_COMMAND} --with-toolset=sun )
  set ( _B2_FLAGS ${_B2_FLAGS} define=BOOST_OPTIONAL_DETAIL_NO_RVALUE_REFERENCES)
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME})
  set ( CMAKE_STATIC_LIBRARY_PREFIX lib )
elseif ("Clang" STREQUAL ${CMAKE_CXX_COMPILER_ID})
  set ( _BOOTSTRAP_COMMAND ${_BOOTSTRAP_COMMAND} --with-toolset=clang )
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

if (NOT WIN32)
# https://github.com/boostorg/process/issues/51
  set( PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/patches)
  ExternalProject_Add_Step( ${${PROJECT_NAME}_EXTERN} patches
      ALWAYS 0
      DEPENDEES download
      DEPENDERS patch
      DEPENDS ${PATCH_FILE}
      WORKING_DIRECTORY ${${PROJECT_NAME}_INSTALL_DIR}
      COMMAND ${PATCH} -u -N -p1 < ${PATCH_FILE}
  )
endif()


ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} SOURCE_DIR )
set( ${PROJECT_NAME}_SOURCE_DIR ${SOURCE_DIR} )
ExternalProject_Get_Property( ${${PROJECT_NAME}_EXTERN} INSTALL_DIR )
set( ${PROJECT_NAME}_INSTALL_DIR ${INSTALL_DIR} )


function(ADD_BOOST_LIBRARY)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LIBRARIES DEPENDENCIES)
  cmake_parse_arguments(PARSE_ARGV 1 args "${options}" "${oneValueArgs}" "${multiValueArgs}")

  set(args_NAME ${ARGV0})

  if (NOT DEFINED args_LIBRARIES OR args_LIBRARIES STREQUAL "")
    set(args_LIBRARIES ${args_NAME})
  endif()

  set(linkLibraries)
  foreach(library ${args_LIBRARIES})
    list(APPEND linkLibraries "${${PROJECT_NAME}_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_${args_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endforeach()

  add_library(boost_${args_NAME} INTERFACE)
  target_link_libraries(boost_${args_NAME} INTERFACE
    ${linkLibraries}
    ${args_DEPENDENCIES}
  )
  add_library(Boost::${args_NAME} ALIAS boost_${args_NAME})
endfunction()


add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/include>
  $<BUILD_INTERFACE:${${PROJECT_NAME}_INSTALL_DIR}/src>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
  BOOST_ALL_NO_LIB
)
target_link_libraries(${PROJECT_NAME} INTERFACE
)
add_library(Boost::boost ALIAS boost)
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXTERN})

add_boost_library(system DEPENDENCIES Boost::boost)
add_boost_library(atomic DEPENDENCIES Boost::boost)
add_boost_library(thread DEPENDENCIES Boost::atomic Boost::boost)
add_boost_library(filesystem DEPENDENCIES Boost::boost)
add_boost_library(log DEPENDENCIES Boost::thread Boost::filesystem Boost::boost)
add_boost_library(log_setup DEPENDENCIES Boost::log)
