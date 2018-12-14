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

Set-PSDebug -Trace 2

$addComponentIds = @(
  '--add microsoft.net.component.4.targetingpack'
  '--add microsoft.net.component.4.5.1.targetingpack'
  '--add microsoft.visualstudio.component.debugger.justintime'
  '--add microsoft.visualstudio.component.web'
  '--add microsoft.visualstudio.component.vc.coreide'
  '--add microsoft.visualstudio.component.vc.redist.14.latest'
  '--add microsoft.visualstudio.component.graphics.win81'
  '--add microsoft.visualstudio.component.vc.cmake.project'
  '--add microsoft.visualstudio.component.vc.testadapterforgoogletest'
  '--add microsoft.component.vc.runtime.ucrtsdk'
  '--add microsoft.visualstudio.component.windows81sdk'
  '--add microsoft.visualstudio.component.vc.cli.support'
  '--add microsoft.visualstudio.component.webdeploy'
  '--add microsoft.component.pythontools'
  '--add component.cpython2.x64'
  '--add microsoft.net.component.3.5.developertools'
)

choco install visualstudio2017community -p --params $addComponentIds -y
# choco install visualstudio2017community -y
# choco install visualstudio2017community --package-parameters "--allWorkloads --includeRecommended --includeOptional --passive --locale en-US" -y

Exit 0