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

project(cppcache LANGUAGES CXX)

include(CheckIncludeFile)
include(CheckFunctionExists)

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
  check_include_file("pthread.h" HAVE_PTHREAD_H)
  check_function_exists("pthread_setname_np" HAVE_pthread_setname_np)
endif()

set(COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "${COMMON_SOURCE_DIR}/*.cpp")

file(GLOB_RECURSE PRIVATE_HEADERS "${COMMON_SOURCE_DIR}/*.hpp")
set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

file(GLOB_RECURSE PUBLIC_HEADERS "${COMMON_INCLUDE_DIR}/*")
set_source_files_properties(${PUBLIC_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

set(RESOURCES ${COMMON_SOURCE_DIR}/apache-geode.rc)

find_package(Git 1.7 REQUIRED)

string(REPLACE ";" "," PRODUCT_VERSION_RC "${PRODUCT_VERSION_LIST}")
list(APPEND CONFIGURE_IN_FILES ${COMMON_SOURCE_DIR}/version.h.in)
list(APPEND CONFIGURE_OUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# adds named target for version.h to avoid parallel generation under windows
add_custom_target(version-header DEPENDS version.h)
set_target_properties(version-header PROPERTIES
	FOLDER cpp
)

if ("SunOS" STREQUAL ${CMAKE_SYSTEM_NAME})
  set( _SOLARIS 1 )
elseif ("Linux" STREQUAL ${CMAKE_SYSTEM_NAME} )
  set( _LINUX 1 )
elseif ("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME} )
  set( _MACOSX 1 )
elseif ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME} )
  set( _WIN32 1 )
endif()

list(APPEND CONFIGURE_IN_FILES ${COMMON_SOURCE_DIR}/config.h.in)
list(APPEND CONFIGURE_OUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${COMMON_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set_source_files_properties(${CONFIGURE_OUT_FILES} PROPERTIES GENERATED TRUE)

set(SOURCES_ALL ${SOURCES} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS} ${CONFIGURE_IN_FILES} ${CONFIGURE_OUT_FILES})

add_library(_apache-geode INTERFACE)

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  target_link_libraries(_apache-geode INTERFACE
    demangle
    kstat
  )
elseif (WIN32)
  target_link_libraries(_apache-geode INTERFACE
    Dbghelp
  )
endif()

target_link_libraries(_apache-geode INTERFACE
  ACE::ACE
  ACE::ACE_SSL
  Boost::boost
  Boost::filesystem
  Boost::thread
  Boost::stacktrace
  XercesC::XercesC
)

if (USE_PCH)
  if (MSVC)
    # TODO figure out why PCH causes these warnings
    # Ignore warnings resulting from using PCH
    target_compile_options(_apache-geode INTERFACE
      /wd4251 /wd4103 /wd4275 /wd4250
    )
  endif()
endif()

target_include_directories(_apache-geode INTERFACE
  $<BUILD_INTERFACE:${COMMON_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

add_dependencies(_apache-geode version-header)

install(DIRECTORY ${COMMON_INCLUDE_DIR} DESTINATION .)
add_subdirectory(shared)
add_subdirectory(static)
add_subdirectory(test)
add_subdirectory(internal)
add_subdirectory(integration)
add_subdirectory(integration-test)

if (BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()
