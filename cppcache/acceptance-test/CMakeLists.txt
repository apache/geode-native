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

add_executable(cpp-acceptance-test
  SNITest.cpp
)

target_compile_definitions(cpp-acceptance-test
  PUBLIC
    GTEST_ELLIPSIS_NEEDS_POD_
)

target_include_directories(cpp-acceptance-test
  PUBLIC
   ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(cpp-acceptance-test
  PUBLIC
    apache-geode
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
)

if(WIN32)
  foreach (_target apache-geode testobject)
    add_custom_command(TARGET cpp-acceptance-test
	  DEPENDS  ${_target}
	  COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${_target}>"
        "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$<TARGET_PDB_FILE:${_target}>>"
        "$<TARGET_FILE_DIR:cpp-acceptance-test>")
  endforeach()
endif()

set_target_properties(cpp-acceptance-test PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  FOLDER cpp/test/acceptance
)

add_clangformat(cpp-acceptance-test)

enable_testing()
include(GoogleTest)
gtest_discover_tests(cpp-acceptance-test
  DISCOVERY_TIMEOUT 60
)

add_custom_target(run-cpp-acceptance-test
  DEPENDS $<TARGET_FILE:cpp-acceptance-test>
  COMMAND ctest -C $<CONFIG>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)
add_dependencies(run-acceptance-tests run-cpp-acceptance-test)
set_target_properties(run-cpp-acceptance-test PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
  FOLDER cpp/test/acceptance
)
