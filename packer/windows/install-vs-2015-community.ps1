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
# TODO AdminDeploy.xml
# vs_community.exe /AdminFile C:\Users\Administrator\AdminDeployment.xml /Log setup.log /Passive
Set-PSDebug -Trace 0

Import-Module Packer

$log = "vs_community.log"

Install-Package https://download.microsoft.com/download/e/4/c/e4c393a9-8fff-441b-ad3a-3f4040317a1f/vs_community.exe `
    -Hash ED8D88D0AB88832754302BFC2A374E803B3A21C1590B428092944272F9EA30FE `
    -ArgumentList @("/AdminFile", "C:\\vs-2015-admin.xml", "/quiet", "/log", $log) `
    -Log $log -Verbose
