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

cmake_minimum_required(VERSION 3.4)
project(apache-geode)

add_library(${PROJECT_NAME} SHARED ${SOURCES_ALL} ${RESOURCES})

set_source_files_properties(${CONFIGURE_OUT_FILES} PROPERTIES GENERATED TRUE)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    _apache-geode
  PUBLIC
    c++11)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<TARGET_PROPERTY:_apache-geode,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(apache-geode PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  OUTPUT_NAME ${PRODUCT_LIB_NAME})

add_dependencies(client-libraries ${PROJECT_NAME})

install(TARGETS apache-geode
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# For Visual Studio organization
source_group("Header Files" REGULAR_EXPRESSION "\.(hpp|inl)$")
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})
