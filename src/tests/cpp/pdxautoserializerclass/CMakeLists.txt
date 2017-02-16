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
cmake_minimum_required(VERSION 3.4)
project(nativeclient.tests.pdxautoserializerclass)

file(GLOB_RECURSE SOURCES "*.cpp")

if (APPLE)
    set(DYNAMIC_LIBRARY_PATH DYLD_LIBRARY_PATH=$<TARGET_LINKER_FILE_DIR:apache-geode>)
elseif(UNIX)
    set(DYNAMIC_LIBRARY_PATH LD_LIBRARY_PATH=$<TARGET_LINKER_FILE_DIR:apache-geode>)
endif()

add_library(pdxobject SHARED ${SOURCES})
set_target_properties(pdxobject PROPERTIES FOLDER test)

add_definitions(-DBUILD_TESTOBJECT)

target_link_libraries(pdxobject
  PUBLIC 
    apache-geode
    c++11
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

