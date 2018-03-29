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

add_library(${PROJECT_NAME} STATIC
  CredentialGenerator.cpp
  CredentialGenerator.hpp
  DummyCredentialGenerator.hpp
  DummyCredentialGenerator2.hpp
  DummyCredentialGenerator3.hpp
  LdapUserCredentialGenerator.hpp
  NoopCredentialGenerator.hpp
  PkcsAuthInit.cpp
  PkcsAuthInit.hpp
  PkcsCredentialGenerator.hpp
  Security.cpp
  Security.hpp
  typedefs.hpp
  XmlAuthzCredentialGenerator.hpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
	$<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    fwk
    testobject
    crypto
    ssl
  PRIVATE
    ACE
	_WarningsAsError
)

# For Visual Studio organization
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER test)
