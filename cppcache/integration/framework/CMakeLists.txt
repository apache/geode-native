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

configure_file(TestConfig.cpp.in TestConfig.cpp)

add_library(integration-framework STATIC
  Gfsh.cpp
  Framework.cpp
  Cluster.cpp
  GfshExecute.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/TestConfig.cpp
  )

target_compile_definitions(integration-framework
  PUBLIC
    BOOST_ASIO_HAS_MOVE
)

target_include_directories(integration-framework
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}

)

target_link_libraries(integration-framework
  PUBLIC
    apache-geode
    GTest::gtest
    Boost::boost
    Boost::system
    Boost::log
    Boost::filesystem
  PRIVATE
    _WarningsAsError
    internal
)

if(WIN32)
  target_compile_definitions(integration-framework
    PUBLIC
      # Required for Boost.WinAPI
      _WIN32_WINNT=0x06020000
  )
endif()
