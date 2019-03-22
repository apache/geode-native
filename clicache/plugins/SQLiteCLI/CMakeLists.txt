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
project(Apache.Geode.Plugins.SQLite CSharp)

add_library( ${PROJECT_NAME} SHARED
    AssemblyInfo.cs
    SqLiteImpl.cs
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Apache.Geode
    PdxClassLibrary
)

get_target_property(SQLite_netFx_INTERFACE_LINK_LIBRARIES SQLite::netFx INTERFACE_LINK_LIBRARIES)

set_target_properties( ${PROJECT_NAME} PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_GLOBAL_TreatWarningsAsErrors True
  VS_GLOBAL_PROJECT_TYPES "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCE_System.Data.SQLite "${SQLite_netFx_INTERFACE_LINK_LIBRARIES}"
  VS_DOTNET_REFERENCES "System;System.Data;System.Xml"
  FOLDER cli/plugins
)

add_dependencies(${PROJECT_NAME} SQLite::netFx)
