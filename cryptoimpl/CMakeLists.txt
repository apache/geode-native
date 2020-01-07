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

project(cryptoImpl LANGUAGES CXX)

add_library(cryptoImpl SHARED
  ${CMAKE_CURRENT_BINARY_DIR}/cryptoimpl_export.h
  Ssl.hpp
  SSLImpl.hpp
  SSLImpl.cpp
)

include(GenerateExportHeader)
generate_export_header(cryptoImpl)

target_include_directories(cryptoImpl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

set_target_properties(cryptoImpl PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  FOLDER cpp/test/integration)

target_link_libraries(cryptoImpl
  PRIVATE
    openssl-compat
    ACE::ACE_SSL
    _WarningsAsError
  PUBLIC
    c++11
)

add_clangformat(cryptoImpl)

if ("Windows" STREQUAL ${CMAKE_SYSTEM_NAME} )
  install(TARGETS cryptoImpl
    RUNTIME DESTINATION bin)
else()
  install (TARGETS cryptoImpl DESTINATION lib)
endif()

