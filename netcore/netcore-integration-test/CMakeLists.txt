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
project(netcore-test LANGUAGES NONE)

add_custom_target(netcore-integration-test ALL 
  COMMAND ${DOTNET} build netcore-integration-test.csproj -p:platform=x64 --configuration $<CONFIG>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:apache-geode-c> ${CMAKE_CURRENT_SOURCE_DIR}/bin/x64/$<CONFIG>/netcoreapp3.1/
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS apache-geode-c netcore-lib netcore-utility VERBATIM)

enable_testing()

set(AUTH_OPTS "--J=-Dgemfire.security-username=server --J=-Dgemfire.security-password=server --classpath=$<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/../utility/netcore-utility.jar>")
set(AUTH_LOCATOR_OPTS "${AUTH_OPTS} --J=-Dgemfire.security-manager=javaobject.SimpleSecurityManager")

add_test(NAME netcore-test 
  COMMAND dotnet test -c $<CONFIG> -p:platform=x64
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

