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

cmake_minimum_required(VERSION 3.10)
project(DHImpl LANGUAGES CXX)

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(DHImpl SHARED
  DHImpl.cpp
  DHImpl.hpp
  ../openssl-compat/openssl-compat.h
)

set_target_properties(DHImpl PROPERTIES
  FOLDER cpp/test/integration)

include(GenerateExportHeader)
generate_export_header(DHImpl)

target_include_directories(DHImpl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(DHImpl
  PUBLIC
    apache-geode
    OpenSSL::Crypto
    c++11
  PRIVATE
    _WarningsAsError
)

add_clangformat(DHImpl)
