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

project( rat VERSION 0.13 LANGUAGES NONE )
# used to check licenses in source

set( ARTIFACT_NAME apache-rat-${PROJECT_VERSION} )
set( ARTIFACT_FILE apache-rat-${PROJECT_VERSION}-bin.tar.gz )
set( ARTIFACT_URL "https://www.apache.org/dyn/closer.lua/creadur/${ARTIFACT_NAME}/${ARTIFACT_FILE}?action=download" )

file( DOWNLOAD ${ARTIFACT_URL} ${CMAKE_CURRENT_BINARY_DIR}/${ARTIFACT_FILE} )
execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar xzf ${ARTIFACT_FILE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set( Rat_JAR ${CMAKE_CURRENT_BINARY_DIR}/${ARTIFACT_NAME}/${ARTIFACT_NAME}.jar CACHE STRING "Full path to Apache Rat jar." FORCE)
