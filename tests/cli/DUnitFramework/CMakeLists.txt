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

cmake_minimum_required( VERSION 3.15 )
project(Apache.Geode.DUnitFramework CSharp)

add_library(Apache.Geode.DUnitFramework SHARED
  AssemblyInfo.cs
  ClientBase.cs
  ClientGroup.cs
  CMakeLists.txt
  DUnitTestClass.cs
  Exceptions.cs
  IClientServerComm.cs
  Log.cs
  ServerCommunication.cs
  ServerConnection.cs
  TimeBomb.cs
  UnitProcess.cs
  UnitThread.cs
  Util.cs
  XmlNodeReaderWriter.cs
)

target_link_libraries(Apache.Geode.DUnitFramework
  PUBLIC
    Apache.Geode
)

set_target_properties(Apache.Geode.DUnitFramework PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE Apache.Geode.DUnitFramework
  DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2"
  VS_GLOBAL_TreatWarningsAsErrors True
  VS_GLOBAL_PROJECT_TYPES "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCES "System;System.Xml;System.Runtime.Remoting;System.Management"
  VS_PACKAGE_REFERENCES "NUnit_2.6.4"
  FOLDER cli/test/integration
)

if(NOT "${STRONG_NAME_KEY}" STREQUAL "")
  set_target_properties(Apache.Geode.DUnitFramework PROPERTIES
    VS_GLOBAL_SignAssembly "true"
    VS_GLOBAL_AssemblyOriginatorKeyFile ${STRONG_NAME_KEY}
  )
endif()
