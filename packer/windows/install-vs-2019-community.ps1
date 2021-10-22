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

$ErrorActionPreference = "Stop"

write-host "Installing Visual Studio 2019 Community..."

$args = @('--add Microsoft.VisualStudio.Component.CoreEditor',
          '--add Microsoft.VisualStudio.Workload.CoreEditor',
          '--add Microsoft.Net.Component.4.6.1.TargetingPack',
          '--add Microsoft.VisualStudio.Component.Roslyn.Compiler',
          '--add Microsoft.Net.Component.4.8.SDK',
          '--add Microsoft.Component.MSBuild',
          '--add Microsoft.VisualStudio.Component.TextTemplating',
          '--add Microsoft.Net.Component.4.5.2.TargetingPack',
          '--add Microsoft.VisualStudio.Component.IntelliCode',
          '--add Microsoft.VisualStudio.Component.VC.CoreIde',
          '--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
          '--add Microsoft.VisualStudio.Component.VC.Redist.14.Latest',
          '--add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core',
          '--add Microsoft.VisualStudio.Component.VC.ATL',
          '--add Microsoft.VisualStudio.Component.Windows10SDK.16299',
          '--add Microsoft.VisualStudio.Component.VC.14.20.x86.x64',
          '--add Microsoft.VisualStudio.Workload.NativeDesktop',
          '--add Microsoft.VisualStudio.Component.VC.14.20.CLI.Support',
          '--add Microsoft.Net.Component.4.6.1.SDK',
		  '--quiet')

choco install visualstudio2019community -confirm --package-parameters "$args"

write-host "Installed Visual Studio 2019 Community."

# Avoids reboot error code
Exit 0
