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

function(add_example)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCE)
  cmake_parse_arguments(ADD_EXAMPLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.cpp_example.in
    ${CMAKE_CURRENT_BINARY_DIR}/${ADD_EXAMPLE_NAME}/CMakeLists.txt
    @ONLY)
endfunction()

add_example(NAME authinitialize
	SOURCE main.cpp exampleAuthInitialize.hpp exampleAuthInitialize.cpp)

add_example(NAME continuousquery
    SOURCE main.cpp Order.cpp Order.hpp)

add_example(NAME dataserializable
    SOURCE main.cpp Order.cpp Order.hpp)

add_example(NAME functionexecution
        SOURCE main.cpp)

add_example(NAME pdxserializable
    SOURCE main.cpp Order.cpp Order.hpp)

add_example(NAME pdxserializer
    SOURCE main.cpp Order.cpp Order.hpp OrderSerializer.cpp OrderSerializer.hpp)

add_example(NAME putgetremove
    SOURCE main.cpp)

add_example(NAME remotequery
    SOURCE main.cpp Order.cpp Order.hpp)

add_subdirectory(sslputget)

add_example(NAME transaction
    SOURCE main.cpp)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION examples/
  USE_SOURCE_PERMISSIONS
  PATTERN "./*.in" EXCLUDE
  PATTERN "CMakeFiles" EXCLUDE
  PATTERN "Makefile" EXCLUDE
  PATTERN "cmake_install.cmake" EXCLUDE)

