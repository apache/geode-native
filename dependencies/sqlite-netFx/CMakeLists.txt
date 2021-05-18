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

project( sqlite-netFx VERSION 1.0.113.0 LANGUAGES NONE )

set( SHA256 deceee8c28c77c1edc9e3edd0a4a7805df6a4a1fb9d2df2a6b70dc0a96491560 )

include(ExternalProject)
ExternalProject_Add( ${PROJECT_NAME}
   URL "https://system.data.sqlite.org/blobs/${PROJECT_VERSION}/${PROJECT_NAME}451-binary-x64-2013-${PROJECT_VERSION}.zip"
   URL_HASH SHA256=${SHA256}
   UPDATE_COMMAND ""
   BUILD_COMMAND ""
   CONFIGURE_COMMAND ""
   INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/System.Data.SQLite.dll <INSTALL_DIR>
)
ExternalProject_Get_Property( ${PROJECT_NAME} INSTALL_DIR )

add_library(SQLite_netFx INTERFACE)
target_link_libraries(SQLite_netFx INTERFACE
  ${INSTALL_DIR}/System.Data.SQLite.dll
)
add_dependencies(SQLite_netFx ${PROJECT_NAME})
add_library(SQLite::netFx ALIAS SQLite_netFx)
