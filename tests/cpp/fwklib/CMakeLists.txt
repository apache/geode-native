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
project(framework LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC 
  ClientTask.hpp
  FrameworkTest.cpp
  FrameworkTest.hpp
  FwkBB.hpp
  FwkBBClient.cpp
  FwkBBClient.hpp
  FwkBBServer.cpp
  FwkBBServer.hpp
  FwkException.hpp
  FwkExport.hpp
  FwkLog.cpp
  FwkLog.hpp
  FwkObjects.cpp
  FwkObjects.hpp
  FwkStrCvt.cpp
  FwkStrCvt.hpp
  GsRandom.cpp
  GsRandom.hpp
  IpcHandler.cpp
  IpcHandler.hpp
  PaceMeter.hpp
  PerfFwk.cpp
  PerfFwk.hpp
  PoolHelper.hpp
  QueryHelper.hpp
  RegionHelper.hpp
  Service.cpp
  Service.hpp
  TaskClient.cpp
  TaskClient.hpp
  TcpIpc.cpp
  TcpIpc.hpp
  TestClient.cpp
  TestClient.hpp
  TimeBomb.cpp
  TimeBomb.hpp
  TimeLimit.hpp
  Timer.hpp
  TimeSync.cpp
  TimeSync.hpp
  UDPIpc.cpp
  UDPIpc.hpp
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    # Add PIC flags to link later with shared library
    ${CMAKE_CXX_COMPILE_OPTIONS_PIC}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    # headers for this library are referenced relative to the parent
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  PRIVATE
  	$<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC 
    apache-geode
    xerces-c
  PRIVATE
    ACE
		internal
	  _WarningsAsError
)

# For Visual Studio organization
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER cpp/test/integration)
