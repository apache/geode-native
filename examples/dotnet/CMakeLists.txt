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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt @ONLY)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
  DESTINATION examples/dotnet)

function(add_example)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCE)
  cmake_parse_arguments(ADD_EXAMPLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  foreach(SOURCE ${ADD_EXAMPLE_SOURCE})
    list(APPEND EXAMPLE_SOURCES_PATHS ${ADD_EXAMPLE_NAME}/${SOURCE})
  endforeach()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.dotnet_example.in
    ${CMAKE_CURRENT_BINARY_DIR}/${ADD_EXAMPLE_NAME}/CMakeLists.txt
    @ONLY)

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${ADD_EXAMPLE_NAME}/CMakeLists.txt
    ${EXAMPLE_SOURCES_PATHS}
    ${ADD_EXAMPLE_NAME}/README.md
    ${ADD_EXAMPLE_NAME}/startserver.ps1
    ${ADD_EXAMPLE_NAME}/stopserver.ps1
    DESTINATION examples/dotnet/${ADD_EXAMPLE_NAME})
endfunction()

add_example(NAME authinitialize
	SOURCE ExampleAuthInitialize.cs Program.cs)

add_example(NAME classaskey
	SOURCE PhotosKey.cs PhotosValue.cs Program.cs)

add_example(NAME continuousquery
	SOURCE Order.cs MyCqListener.cs Program.cs)

add_example(NAME dataserializable
	SOURCE Order.cs Program.cs)

add_example(NAME functionexecution
	SOURCE Program.cs)

add_example(NAME pdxautoserializer
	SOURCE Order.cs Program.cs)

add_example(NAME pdxserializable
	SOURCE Order.cs Program.cs)

add_example(NAME putgetremove
	SOURCE Program.cs)

add_example(NAME remotequery 
	SOURCE Order.cs Program.cs)

add_subdirectory(sslputget)

add_example(NAME transaction
	SOURCE Program.cs)
