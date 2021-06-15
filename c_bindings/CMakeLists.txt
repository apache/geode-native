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

project(apache-geode-c)

add_library(${PROJECT_NAME} SHARED "src/auth_initialize.cpp"
"src/cache.cpp"
"src/client.cpp"
"src/pool.cpp"
"src/region.cpp"
"src/cache/factory.cpp"
"src/pool/factory.cpp"
"src/pool/manager.cpp"
"src/region/factory.cpp")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

target_link_libraries(${PROJECT_NAME} PRIVATE apache-geode-static)

target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:apache-geode-static,SOURCE_DIR>/../src)

include(GenerateExportHeader)

generate_export_header(${PROJECT_NAME}
  BASE_NAME APACHE_GEODE_C
  EXPORT_FILE_NAME apache-geode-c_export.h
  CUSTOM_CONTENT_FROM_VARIABLE EXPORT_HEADER_CUSTOM_CONTENT
)

include_directories("include" "src" $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-exported_symbols_list \"${CMAKE_CURRENT_SOURCE_DIR}/symbols\"")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-Wl,--version-script=\"${CMAKE_CURRENT_SOURCE_DIR}/symbols.version\"")
endif()

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(DIRECTORY "include/geode/" DESTINATION "c/include/geode")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/apache-geode-c_export.h DESTINATION c/include/geode/internal)

add_subdirectory(integration)

add_clangformat(apache-geode-c)
