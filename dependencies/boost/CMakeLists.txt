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

project( boost VERSION 1.72.0 LANGUAGES NONE )

set( SHA256 c66e88d5786f2ca4dbebb14e06b566fb642a1a6947ad8cc9091f9f445134143f )

set(_B2_FLAGS
  -d0
  --prefix=<INSTALL_DIR>/$<CONFIG>
  --with-system
  --with-log
  --layout=system
  address-model=${BUILD_BITS}
  link=static
  threading=multi
  variant=$<IF:$<CONFIG:Debug>,debug,release>
  debug-symbols=$<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,on,off>
)

include(ProcessorCount)
ProcessorCount(_NPROCS)
if (${_NPROCS})
  set ( _B2_FLAGS ${_B2_FLAGS} -j${_NPROCS} )
endif()

if (${WIN32})
  set ( _BOOTSTRAP_COMMAND .\\bootstrap.bat )
  set ( _B2_COMMAND .\\b2 )
  set ( _B2_FLAGS ${_B2_FLAGS} toolset=msvc-14.1 )
else()
  set ( _BOOTSTRAP_COMMAND ./bootstrap.sh )
  set ( _B2_COMMAND ./b2 )
  set ( _B2_FLAGS ${_B2_FLAGS} "cxxflags=${CMAKE_CXX_FLAGS} ${CMAKE_CXX11_STANDARD_COMPILE_OPTION} ${CMAKE_CXX_COMPILE_OPTIONS_PIC}" )
endif()

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  set ( _BOOTSTRAP_COMMAND ${_BOOTSTRAP_COMMAND} --with-toolset=sun )
  set ( _B2_FLAGS ${_B2_FLAGS} define=BOOST_OPTIONAL_DETAIL_NO_RVALUE_REFERENCES)
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME})
  set ( CMAKE_STATIC_LIBRARY_PREFIX lib )
elseif ("Clang" STREQUAL ${CMAKE_CXX_COMPILER_ID})
  set ( _BOOTSTRAP_COMMAND ${_BOOTSTRAP_COMMAND} --with-toolset=clang )
endif()

string(REPLACE "." "_" _VERSION_UNDERSCORE ${PROJECT_VERSION})
set( EXTERN ${PROJECT_NAME}-extern )

include(ExternalProject)
ExternalProject_Add(${EXTERN}
   URL "https://boostorg.jfrog.io/artifactory/main/release/${PROJECT_VERSION}/source/boost_${_VERSION_UNDERSCORE}.tar.gz"
       "https://sourceforge.net/projects/boost/files/boost/${PROJECT_VERSION}/boost_${_VERSION_UNDERSCORE}.tar.gz/download"
   URL_HASH SHA256=${SHA256}
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

ExternalProject_Get_Property( ${EXTERN} SOURCE_DIR )
ExternalProject_Get_Property( ${EXTERN} INSTALL_DIR )
set(INSTALL_DIR "${INSTALL_DIR}/$<CONFIG>")

function(ADD_BOOST_LIBRARY)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LIBRARIES DEPENDENCIES)
  cmake_parse_arguments(PARSE_ARGV 1 args "${options}" "${oneValueArgs}" "${multiValueArgs}")

  set(args_NAME ${ARGV0})

  if (NOT DEFINED args_LIBRARIES)
    set(args_LIBRARIES ${args_NAME})
  endif()

  set(linkLibraries)
  foreach(library ${args_LIBRARIES})
    list(APPEND linkLibraries "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_${args_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
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
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
  BOOST_ALL_NO_LIB
)
target_link_libraries(${PROJECT_NAME} INTERFACE
)
add_library(Boost::boost ALIAS boost)
add_dependencies(${PROJECT_NAME} ${EXTERN})

find_package(Threads REQUIRED)

add_boost_library(system DEPENDENCIES Boost::boost)
add_boost_library(atomic DEPENDENCIES Boost::boost)
add_boost_library(thread DEPENDENCIES Threads::Threads Boost::atomic Boost::boost)
add_boost_library(filesystem DEPENDENCIES Boost::system Boost::boost)
add_boost_library(log DEPENDENCIES Boost::thread Boost::filesystem Boost::boost)
add_boost_library(log_setup DEPENDENCIES Boost::log)
add_boost_library(chrono DEPENDENCIES Boost::chrono)
add_boost_library(stacktrace LIBRARIES "" DEPENDENCIES Boost::boost)
add_boost_library(asio LIBRARIES "" DEPENDENCIES Boost::boost)
add_boost_library(process LIBRARIES "" DEPENDENCIES Boost::system)

target_compile_definitions(boost_stacktrace INTERFACE
  BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
)

target_compile_definitions(boost_asio INTERFACE
    BOOST_ASIO_HAS_MOVE
)

if(WIN32)
  target_compile_definitions(boost INTERFACE
    # Required for Boost.WinAPI
    _WIN32_WINNT=0x06020000
  )
endif()

if(MSVC)
  target_compile_options(boost_process INTERFACE
  	/wd4267
    /wd4244
	)
endif()
