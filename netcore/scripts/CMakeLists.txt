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
project(netcore-scripts LANGUAGES NONE)

if(DOTNET AND INCLUDE_DOTNET_CORE)
  set(GEODE_NATIVE_NETCORE_UTILITY_CLASSPATH ${CMAKE_CURRENT_BINARY_DIR}/../utility/netcore-utility.jar)
  file(TO_NATIVE_PATH ${GEODE_NATIVE_NETCORE_UTILITY_CLASSPATH} GEODE_NATIVE_NETCORE_UTILITY_CLASSPATH)
  message(STATUS "CLASSPATH: ${GEODE_NATIVE_NETCORE_UTILITY_CLASSPATH}")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/startserver.ps1.in ${CMAKE_CURRENT_BINARY_DIR}/startserver.ps1)
endif()

