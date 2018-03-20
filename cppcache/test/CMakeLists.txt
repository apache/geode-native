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

cmake_minimum_required( VERSION 3.3 )
project(apache-geode_unittests)

file(GLOB_RECURSE SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE "/MD$<$<CONFIG:Debug>:d>")
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    apache-geode-static
    gtest
    gtest_main
    boost
    c++11
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>
    #$<TARGET_PROPERTY:apache-geode,BINARY_DIR>
)

add_dependencies(unit-tests ${PROJECT_NAME})


enable_testing()

# Function to resolve both config and generate stage variables.
#macro(generate_config INPUT TEMP OUTPUT)
#  configure_file(${INPUT} ${TEMP})
#  file(GENERATE OUTPUT ${OUTPUT} INPUT ${TEMP} CONDITION 1)
#endmacro()

#if (WIN32)
#  set (PROJECT_NAME_SCRIPT_SUFFIX .bat)
#  generate_config(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.bat.in ${CMAKE_CURRENT_BINARY_DIR}/.${PROJECT_NAME}.bat.in $<TARGET_FILE:${PROJECT_NAME}>${PROJECT_NAME_SCRIPT_SUFFIX})
#
#  set(PROJECT_NAME_COMMAND $<TARGET_FILE:${PROJECT_NAME}>${PROJECT_NAME_SCRIPT_SUFFIX})
#else()
#  set (PROJECT_NAME_SCRIPT_SUFFIX .sh)
#  generate_config(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.sh.in ${CMAKE_CURRENT_BINARY_DIR}/.${PROJECT_NAME}.sh.in $<TARGET_FILE:${PROJECT_NAME}>${PROJECT_NAME_SCRIPT_SUFFIX})
#
#  # TODO fix file permissions on .sh
#  set(PROJECT_NAME_COMMAND bash $<TARGET_FILE:${PROJECT_NAME}>${PROJECT_NAME_SCRIPT_SUFFIX})
#endif()

add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)


add_custom_target(run-cppcache-unit-tests
  COMMAND ctest -C $<CONFIGURATION> ${CTEST_PROJECT_NAME_VERBOSITY}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL)
set_target_properties(run-cppcache-unit-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
)
add_dependencies(run-unit-tests run-cppcache-unit-tests)
