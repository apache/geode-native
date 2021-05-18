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

get_target_property(JAVAOBJECT_JAR_PATH javaobject JAR_FILE)

configure_file(TestConfig.cpp.in TestConfig.cpp)

add_library(integration-framework STATIC
  Cluster.cpp
  Cluster.h
  Framework.cpp
  Framework.h
  Gfsh.cpp
  Gfsh.h
  GfshExecute.cpp
  GfshExecute.h
  NamedType.h
  TestConfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/TestConfig.cpp
  )

add_dependencies(integration-framework javaobject)

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
    Boost::process
    Boost::asio
  PRIVATE
    _WarningsAsError
    internal
)

add_clangformat(integration-framework)
