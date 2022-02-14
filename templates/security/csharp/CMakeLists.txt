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


project(Apache.Geode.Templates.Cache.Security CSharp)

add_library(Apache.Geode.Templates.Cache.Security SHARED
  AssemblyInfo.cs
  UserPasswordAuthInit.cs
)

target_link_libraries(Apache.Geode.Templates.Cache.Security
  PUBLIC
    Apache.Geode
)

set_target_properties(Apache.Geode.Templates.Cache.Security PROPERTIES
  VS_GLOBAL_ROOTNAMESPACE Apache.Geode.Templates.Cache.Security
  VS_GLOBAL_TreatWarningsAsErrors True
  VS_GLOBAL_PROJECT_TYPES "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
  VS_DOTNET_REFERENCES "System;System.Xml;System.Data"
  FOLDER cli/test/integration
)

