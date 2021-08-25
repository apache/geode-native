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

project(netcore LANGUAGES NONE)

option(INCLUDE_DOTNET_CORE "Build .NET Core client." ON)

find_program(DOTNET dotnet)

if(DOTNET AND INCLUDE_DOTNET_CORE)
  add_custom_target(netcore ALL COMMAND ${DOTNET} build --configuration $<CONFIG> WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/NetCore DEPENDS apache-geode-c VERBATIM)
  add_custom_target(netcore-test ALL COMMAND ${DOTNET} build --configuration $<CONFIG> WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/NetCore.Test DEPENDS netcore VERBATIM)

#
# Move object files and binaries to build directory.  We do this for the sake
# of tidiness in the source tree.  This method seems a little bit overwrought,
# but reasoning is as follows:
# i.  At the time of this writing, cmake does not properly support .net core
#     builds.
# ii. dotnet build command does not, as far as we can tell, support building
#     outside of the source tree.
# ii. cmake has a rename_directory command, but it does not work if the
#     destination directory is empty.  We were left with a choice of
#     delete-then-rename or copy-then-delete, and chose the latter.
#
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore/bin ${CMAKE_CURRENT_BINARY_DIR}/NetCore/bin)
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore/bin)

  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore/obj ${CMAKE_CURRENT_BINARY_DIR}/NetCore/obj)
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore/obj)
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore.Test/bin ${CMAKE_CURRENT_BINARY_DIR}/NetCore.Test/bin)
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore.Test/bin)
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore.Test/obj ${CMAKE_CURRENT_BINARY_DIR}/NetCore.Test/obj)
  add_custom_command(TARGET netcore POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/NetCore.Test/obj)

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/NetCore/bin/$<CONFIG>/netcoreapp3.1/Apache.Geode.NetCore.dll DESTINATION ./lib)

  add_subdirectory(utility)
endif()
