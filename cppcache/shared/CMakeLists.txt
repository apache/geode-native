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

project(apache-geode LANGUAGES CXX)

add_library(apache-geode SHARED ${SOURCES_ALL} ${RESOURCES})

if (MSVC)
  target_compile_options(apache-geode
    PRIVATE
      /bigobj # C1128 - large number of templates causes too many section.
   )
  target_link_options(apache-geode
    PRIVATE
      /DELAYLOAD:libssl-1_1-x64.dll
      /DELAYLOAD:libcrypto-1_1-x64.dll
  )
endif()

if (has_linker_exclude_libs)
  target_link_options(apache-geode PRIVATE -Wl,--exclude-libs,ALL)
endif()

generate_export_file( "apache-geode" )

set_source_files_properties(${CONFIGURE_OUT_FILES} PROPERTIES GENERATED TRUE)

target_link_libraries(apache-geode
  PRIVATE
    _apache-geode
    _WarningsAsError
    _CppCodeCoverage
  PUBLIC
    c++11)

target_include_directories(apache-geode
  PUBLIC
    $<TARGET_PROPERTY:_apache-geode,INTERFACE_INCLUDE_DIRECTORIES>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_definitions(apache-geode PUBLIC GEODE_WITH_LOG_FMT)

set_target_properties(apache-geode PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  OUTPUT_NAME ${PRODUCT_LIB_NAME}
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  FOLDER cpp
  )

add_dependencies(client-libraries apache-geode)

install(TARGETS apache-geode
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

IF(MSVC)
  INSTALL ( 
    FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> 
    DESTINATION bin 
    CONFIGURATIONS Debug RelWithDebInfo 
  )
ENDIF(MSVC)

# BEGIN Visual Studio organization
source_group("Header Files" REGULAR_EXPRESSION "\.(hpp|inl)$")
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})
# END Visual Studio organization


if (USE_PCH)
  set(_COTIRE_PREFIX_HEADER_IGNORE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND _COTIRE_PREFIX_HEADER_IGNORE_PATH
      /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/9.0.0/include/unwind.h
      ${CMAKE_BINARY_DIR}/dependencies/boost/include/boost/stacktrace
      ${CMAKE_BINARY_DIR}/dependencies/boost/include/boost/stacktrace.hpp)
  endif()

  set_target_properties(apache-geode PROPERTIES
    COTIRE_ENABLE_PRECOMPILED_HEADER TRUE
    COTIRE_ADD_UNITY_BUILD TRUE
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${_COTIRE_PREFIX_HEADER_IGNORE_PATH}"
    COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_BINARY_DIR}/dependencies;${CMAKE_CURRENT_SOURCE_DIR}/../include")

  set_source_files_properties(cppcache/version.h PROPERTIES GENERATED TRUE)

  cotire(apache-geode)
endif()
