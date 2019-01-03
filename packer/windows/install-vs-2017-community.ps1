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

$vs_community_bootstrapper_uri = "https://download.visualstudio.microsoft.com/download/pr/5df30b3f-9db2-4195-bce3-c5518277da5d/18edc9dd7697111f993c5c06f18b51e5/vs_community.exe"
$vs_community_bootstrapper = "C:\vs_community.exe"

$args = @('--installPath "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community"'
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
		  '--add microsoft.visualstudio.component.windows10sdk.17134'
		  '--add microsoft.visualstudio.component.windows10sdk.16299.desktop'
		  '--add microsoft.visualstudio.component.webdeploy'
		  '--add microsoft.component.pythontools'
		  '--add component.cpython2.x64'
		  '--add microsoft.net.component.3.5.developertools'
		  '--add microsoft.visualstudio.component.typescript.3.0'
		  '--quiet'
		  )

Invoke-WebRequest -Uri $vs_community_bootstrapper_uri -OutFile $vs_community_bootstrapper

Start-Process -Filepath $vs_community_bootstrapper -ArgumentList $args -Wait

Exit 0
