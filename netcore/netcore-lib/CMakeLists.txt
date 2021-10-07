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
project(netcore-lib LANGUAGES NONE)

add_custom_target(netcore-lib ALL COMMAND ${DOTNET} build netcore-lib.csproj -p:platform=x64 --configuration $<CONFIG>
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS apache-geode-c VERBATIM)

if (WIN32)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/netcoreapp3.1/Apache.Geode.NetCore.dll DESTINATION ./bin)
else ()
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/netcoreapp3.1/Apache.Geode.NetCore.dll DESTINATION ./lib)
endif()

