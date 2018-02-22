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

cmake_minimum_required(VERSION 3.8.2)

project(PutGetRemove CSharp)

message(STATUS "Assembly is ${GEODE_ASSEMBLY}")

if ("${GEODE_ASSEMBLY}" STREQUAL "")
	message(FATAL_ERROR "GEODE_ASSEMBLY not set, examples will not build without it")
endif()

add_executable(${PROJECT_NAME} 
    Program.cs)

message(STATUS "output dir is ${RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "project binary dir: ${PROJECT_BINARY_DIR}")

message(STATUS "current source dir is ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Attempting to link to ${CMAKE_CURRENT_SOURCE_DIR}/../../../build/clicache/src/Debug/Apache.Geode.dll")

target_link_libraries(${PROJECT_NAME}
	${GEODE_ASSEMBLY})

set_property(TARGET PutGetRemove PROPERTY VS_DOTNET_REFERENCES
    "System"
    "${GEODE_ASSEMBLY}")
    
