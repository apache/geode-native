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
project(security LANGUAGES CXX)

add_library(security STATIC
  CredentialGenerator.cpp
  CredentialGenerator.hpp
  DummyCredentialGenerator.hpp
  DummyCredentialGenerator2.hpp
  DummyCredentialGenerator3.hpp
  LdapUserCredentialGenerator.hpp
  NoopCredentialGenerator.hpp
  PkcsCredentialGenerator.hpp
  typedefs.hpp
  XmlAuthzCredentialGenerator.hpp
)

include(GenerateExportHeader)
generate_export_header(security)

target_include_directories(security
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

target_link_libraries(security
  PUBLIC
    framework
    testobject
    OpenSSL::Crypto
    OpenSSL::SSL
  PRIVATE
    ACE::ACE
    _WarningsAsError
)

add_clangformat(security)

# For Visual Studio organization
set_target_properties(security PROPERTIES FOLDER cli/test/integration)
