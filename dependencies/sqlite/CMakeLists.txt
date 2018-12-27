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

project( sqlite VERSION 3260000 LANGUAGES NONE )

set( SHA265 de5dcab133aa339a4cf9e97c40aa6062570086d6085d8f9ad7bc6ddf8a52096e )


set( EXTERN ${PROJECT_NAME}-extern )
include(ExternalProject)
ExternalProject_Add( ${EXTERN}
  URL "https://www.sqlite.org/2018/sqlite-amalgamation-${PROJECT_VERSION}.zip"
  URL_HASH SHA256=${SHA265}
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_Property( ${EXTERN} SOURCE_DIR )
ExternalProject_Get_Property( ${EXTERN} INSTALL_DIR )

ExternalProject_Add_Step( ${EXTERN} CMakeLists.txt
  DEPENDEES patch
  DEPENDERS configure
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in
  BYPRODUCTS ${SOURCE_DIR}/CMakeLists.txt
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in ${SOURCE_DIR}/CMakeLists.txt
)

if (WIN32)
  set(CMAKE_SHARED_LIBRARY_SUFFIX .lib)
endif()

add_library(SQLite_sqlite3 INTERFACE)
target_include_directories(SQLite_sqlite3 SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)
target_link_libraries(SQLite_sqlite3 INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}sqlite3$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
)
add_dependencies(SQLite_sqlite3 ${EXTERN})

set_target_properties(SQLite_sqlite3 PROPERTIES INTERFACE_LIBRARY_DIR ${INSTALL_DIR}/lib)
set_target_properties(SQLite_sqlite3 PROPERTIES INTERFACE_RUNTIME_DIR ${INSTALL_DIR}/bin)

add_library(SQLite::sqlite3 ALIAS SQLite_sqlite3)
