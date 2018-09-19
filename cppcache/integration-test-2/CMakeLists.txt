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

configure_file(framework/config.h.in config.h)

add_executable(integration-test-2
  ExampleTest.cpp
  framework/Gfsh.cpp
  framework/Gfsh.h
  framework/Framework.cpp
  framework/Framework.h
  framework/Cluster.cpp
  framework/Cluster.h
  framework/GfshExecute.cpp
  framework/GfshExecute.h
  RegionPutGetAllTest.cpp
  PdxInstanceTest.cpp
  RegisterKeysTest.cpp
  StructTest.cpp
  EnableChunkHandlerThreadTest.cpp
  DataSerializableTest.cpp
)

target_compile_definitions(integration-test-2
  PUBLIC
    BOOST_ASIO_HAS_MOVE
    GTEST_ELLIPSIS_NEEDS_POD_
)

target_include_directories(integration-test-2
  PUBLIC
   ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(integration-test-2
  PUBLIC
    apache-geode
    testobject
    ACE
    GTest::GTest
    GTest::Main
    Boost::boost
    Boost::system
    Boost::log
    Boost::filesystem
 PRIVATE
    _WarningsAsError
 internal
)

if(WIN32)
  target_compile_definitions(integration-test-2
    PUBLIC
      # Required for Boost.WinAPI
      _WIN32_WINNT=0x06020000
  )

  foreach (_target apache-geode testobject)
    add_custom_command(TARGET integration-test-2 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${_target}>"
        "$<$<CONFIG:Debug>:$<TARGET_PDB_FILE:${_target}>>"
        "$<TARGET_FILE_DIR:integration-test-2>")
  endforeach()
endif()

enable_testing()
include(GoogleTest)
gtest_discover_tests(integration-test-2)