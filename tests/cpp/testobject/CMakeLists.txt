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
project(testobject LANGUAGES CXX)

add_library(testobject SHARED 
  ArrayOfByte.hpp
  BatchObject.cpp
  BatchObject.hpp
  DeltaFastAssetAccount.cpp
  DeltaFastAssetAccount.hpp
  DeltaPSTObject.cpp
  DeltaPSTObject.hpp
  DeltaTestImpl.cpp
  DeltaTestImpl.hpp
  DeltaTestObj.hpp
  EqStruct.cpp
  EqStruct.hpp
  FastAsset.cpp
  FastAsset.hpp
  FastAssetAccount.cpp
  FastAssetAccount.hpp
  InvalidPdxUsage.cpp
  InvalidPdxUsage.hpp
  NestedPdxObject.cpp
  NestedPdxObject.hpp
  NonPdxType.cpp
  NonPdxType.hpp
  NoopAuthInit.cpp
  NoopAuthInit.hpp
  PdxClassV1.cpp
  PdxClassV1.hpp
  PdxClassV2.cpp
  PdxClassV2.hpp
  PdxType.cpp
  PdxType.hpp
  PdxVersioned1.cpp
  PdxVersioned1.hpp
  PdxVersioned2.cpp
  PdxVersioned2.hpp
  Portfolio.cpp
  Portfolio.hpp
  PortfolioPdx.cpp
  PortfolioPdx.hpp
  Position.cpp
  Position.hpp
  PositionPdx.cpp
  PositionPdx.hpp
  PSTObject.cpp
  PSTObject.hpp
  TestFactories.cpp
  TestObject1.cpp
  TestObject1.hpp
  TimestampedObject.hpp
  VariousPdxTypes.cpp
  VariousPdxTypes.hpp
)

include(GenerateExportHeader)
generate_export_header(testobject)

target_include_directories(testobject
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_definitions(testobject
  PRIVATE
    BUILD_TESTOBJECT
)

target_include_directories(testobject
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

target_link_libraries(testobject
  PUBLIC
    apache-geode
    framework
  PRIVATE
    ACE::ACE
    Boost::boost
    _WarningsAsError
    spdlog::spdlog
)

add_clangformat(testobject)

# For Visual Studio organization
set_target_properties(testobject PROPERTIES FOLDER test-common)
