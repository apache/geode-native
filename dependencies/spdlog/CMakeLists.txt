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

project( spdlog VERSION 1.8.5 LANGUAGES NONE )

set( SHA256 6e66c8ed4c014b0fb00c74d34eea95b5d34f6e4b51b746b1ea863dc3c2e854fd )

include(GNUInstallDirs)
include(ExternalProject)

ExternalProject_Add( ${PROJECT_NAME}-extern
  URL "https://github.com/gabime/spdlog/archive/v${PROJECT_VERSION}.zip"
  URL_HASH SHA256=${SHA256}
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=$<CONFIG>
    -DSPDLOG_BUILD_BENCH=OFF
    -DSPDLOG_BUILD_EXAMPLES=OFF
    -DSPDLOG_BUILD_TESTS=OFF
)

ExternalProject_Get_Property( ${PROJECT_NAME}-extern INSTALL_DIR )

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
  spdlog
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-extern)
add_library(spdlog::spdlog ALIAS spdlog)

