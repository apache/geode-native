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

project(apache-geode-c-integration-test)

add_executable(${PROJECT_NAME}
  CAuthInitialize.cpp
  CCacheCreationTest.cpp
  ExampleTest.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    GTEST_ELLIPSIS_NEEDS_POD_
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
   ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    apache-geode-c
    apache-geode-c-integration-utilities
    integration-framework
    testobject
    ACE::ACE
    GTest::gtest
    GTest::gtest_main
    Boost::boost
    Boost::system
    Boost::log
    Boost::filesystem
    Boost::chrono
  PRIVATE
    _WarningsAsError
    internal
)

if(WIN32)
  foreach (_target apache-geode-c apache-geode testobject)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${_target}>"
        "$<$<CONFIG:Debug>:$<TARGET_PDB_FILE:${_target}>>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
  endforeach()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  FOLDER c_bindings/test/integration
)

add_clangformat(${PROJECT_NAME})

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME} DISCOVERY_TIMEOUT 60)
