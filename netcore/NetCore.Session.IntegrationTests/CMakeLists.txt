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
project(netcore-session-integrationtest LANGUAGES NONE)

if(DOTNET AND INCLUDE_DOTNET_CORE)
  add_custom_target(netcore-session-integrationtests ALL 
    COMMAND ${DOTNET} build NetCore.Session.IntegrationTests.csproj --configuration $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:apache-geode-c> ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/netcoreapp3.1/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS apache-geode-c netcore-session VERBATIM)
endif()

enable_testing()

add_test(NAME startclusters COMMAND ${Geode_gfsh_EXECUTABLE}
         -e "start locator --name=locator --port=10334 --http-service-port=6060 --J=-Dgemfire.jmx-manager-port=1099"
         -e "start server --name=server --server-port=0"
         -e "create region --name=exampleRegion --type=PARTITION"
         -e "create region --name=geodeSessionState --type=PARTITION"
         -e "disconnect"
         -e "start locator --name=auth_locator ${AUTH_LOCATOR_OPTS} --port=10335 --http-service-port=7070 --J=-Dgemfire.jmx-manager-port=2099"
         -e "connect --locator=localhost[10335] --user=server --password=server"
         -e "start server --name=auth_server ${AUTH_OPTS} --server-port=0"
         -e "create region --name=authExampleRegion --type=PARTITION"
         -e "create region --name=authGeodeSessionState --type=PARTITION")
add_test(NAME stopclusters 
  COMMAND ${Geode_gfsh_EXECUTABLE}
     -e "connect --locator=localhost[10335] --user=server --password=server" 
     -e "stop server --name=auth_server"
     -e "stop locator --name=auth_locator"
     -e "connect --locator=localhost[10334]" 
     -e "stop server --name=server"
     -e "stop locator --name=locator")
add_test(NAME remove_dirs
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/remove_dirs.cmake)

add_test(NAME netcore-session-test 
  COMMAND dotnet test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(startclusters PROPERTIES FIXTURES_SETUP geode)
set_tests_properties(netcore-session-test PROPERTIES FIXTURES_REQUIRED geode)
set_tests_properties(stopclusters PROPERTIES FIXTURES_CLEANUP geode)
set_tests_properties(remove_dirs PROPERTIES DEPENDS stopclusters)
