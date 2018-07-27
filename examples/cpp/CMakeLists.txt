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

project(examples LANGUAGES NONE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt COPYONLY)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
  BUILDING.md
  DESTINATION examples/cpp)

function(add_example)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCE)
  cmake_parse_arguments(ADD_EXAMPLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  foreach(SOURCE ${ADD_EXAMPLE_SOURCE})
    list(APPEND EXAMPLE_SOURCES_PATHS ${ADD_EXAMPLE_NAME}/${SOURCE})
  endforeach()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.cpp_example.in
    ${CMAKE_CURRENT_BINARY_DIR}/${ADD_EXAMPLE_NAME}/CMakeLists.txt
    @ONLY)

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${ADD_EXAMPLE_NAME}/CMakeLists.txt
    ${EXAMPLE_SOURCES_PATHS}
    ${ADD_EXAMPLE_NAME}/startserver.sh
    ${ADD_EXAMPLE_NAME}/stopserver.sh
    DESTINATION examples/cpp/${ADD_EXAMPLE_NAME})
endfunction()

add_example(NAME dataserializable
	SOURCE main.cpp Order.cpp Order.hpp)

add_example(NAME pdxserializable
	SOURCE main.cpp Order.cpp Order.hpp)

add_example(NAME customserializer
	SOURCE main.cpp Order.cpp Order.hpp OrderSerializer.cpp OrderSerializer.hpp)

add_example(NAME put-get-remove
	SOURCE main.cpp)
