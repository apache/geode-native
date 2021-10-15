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
          '--add Microsoft.NetCore.Component.Runtime.5.0',
          '--add Microsoft.NetCore.Component.Runtime.3.1',
          '--add Microsoft.NetCore.Component.SDK',
          '--add Microsoft.VisualStudio.Component.NuGet',
          '--add Microsoft.Net.Component.4.6.1.TargetingPack',
          '--add Microsoft.VisualStudio.Component.Roslyn.Compiler',
          '--add Microsoft.VisualStudio.Component.Roslyn.LanguageServices',
          '--add Microsoft.VisualStudio.Component.FSharp',
          '--add Microsoft.ComponentGroup.ClickOnce.Publish',
          '--add Microsoft.NetCore.Component.DevelopmentTools',
          '--add Microsoft.VisualStudio.Component.FSharp.WebTemplates',
          '--add Microsoft.VisualStudio.ComponentGroup.WebToolsExtensions',
          '--add Microsoft.VisualStudio.Component.DockerTools',
          '--add Microsoft.NetCore.Component.Web',
          '--add Microsoft.Net.Component.4.8.SDK',
          '--add Microsoft.Net.Component.4.7.2.TargetingPack',
          '--add Microsoft.Net.ComponentGroup.DevelopmentPrerequisites',
          '--add Microsoft.VisualStudio.Component.TypeScript.4.3',
          '--add Microsoft.VisualStudio.Component.JavaScript.TypeScript',
          '--add Microsoft.VisualStudio.Component.JavaScript.Diagnostics',
          '--add Microsoft.Component.MSBuild',
          '--add Microsoft.VisualStudio.Component.TextTemplating',
          '--add Component.Microsoft.VisualStudio.RazorExtension',
          '--add Microsoft.VisualStudio.Component.IISExpress',
          '--add Microsoft.VisualStudio.Component.SQL.ADAL',
          '--add Microsoft.VisualStudio.Component.SQL.LocalDB.Runtime',
          '--add Microsoft.VisualStudio.Component.Common.Azure.Tools',
          '--add Microsoft.VisualStudio.Component.SQL.CLR',
          '--add Microsoft.VisualStudio.Component.MSODBC.SQL',
          '--add Microsoft.VisualStudio.Component.MSSQL.CMDLnUtils',
          '--add Microsoft.VisualStudio.Component.ManagedDesktop.Core',
          '--add Microsoft.Net.Component.4.5.2.TargetingPack',
          '--add Microsoft.Net.Component.4.5.TargetingPack',
          '--add Microsoft.VisualStudio.Component.SQL.SSDT',
          '--add Microsoft.VisualStudio.Component.SQL.DataSources',
          '--add Component.Microsoft.Web.LibraryManager',
          '--add Component.Microsoft.WebTools.BrowserLink.WebLivePreview',
          '--add Microsoft.VisualStudio.ComponentGroup.Web',
          '--add Microsoft.VisualStudio.Component.Web',
          '--add Microsoft.VisualStudio.ComponentGroup.Web.Client',
          '--add Microsoft.Net.Component.4.TargetingPack',
          '--add Microsoft.Net.Component.4.5.1.TargetingPack',
          '--add Microsoft.Net.Component.4.6.TargetingPack',
          '--add Microsoft.Net.ComponentGroup.TargetingPacks.Common',
          '--add Component.Microsoft.VisualStudio.Web.AzureFunctions',
          '--add Microsoft.VisualStudio.ComponentGroup.AzureFunctions',
          '--add Microsoft.VisualStudio.Component.Azure.Compute.Emulator',
          '--add Microsoft.VisualStudio.Component.Azure.Storage.Emulator',
          '--add Microsoft.VisualStudio.Component.Azure.ClientLibs',
          '--add Microsoft.VisualStudio.Component.Azure.AuthoringTools',
          '--add Microsoft.VisualStudio.Component.CloudExplorer',
          '--add Microsoft.VisualStudio.ComponentGroup.Web.CloudTools',
          '--add Microsoft.VisualStudio.Component.DiagnosticTools',
          '--add Microsoft.VisualStudio.Component.EntityFramework',
          '--add Microsoft.VisualStudio.Component.AspNet45',
          '--add Microsoft.VisualStudio.Component.AppInsights.Tools',
          '--add Microsoft.VisualStudio.Component.WebDeploy',
          '--add Microsoft.VisualStudio.Component.Debugger.JustInTime',
          '--add Component.Microsoft.VisualStudio.LiveShare',
          '--add Microsoft.VisualStudio.Component.WslDebugging',
          '--add Microsoft.VisualStudio.Component.IntelliCode',
          '--add Microsoft.VisualStudio.Workload.NetWeb',
          '--add Microsoft.Component.PythonTools',
          '--add Component.CPython3.x64',
          '--add Microsoft.VisualStudio.Component.VC.CoreIde',
          '--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
          '--add Microsoft.VisualStudio.Component.Graphics.Tools',
          '--add Microsoft.VisualStudio.Component.VC.DiagnosticTools',
          '--add Microsoft.VisualStudio.Component.Windows10SDK.19041',
          '--add Microsoft.VisualStudio.Workload.Python',
          '--add Microsoft.VisualStudio.Component.ManagedDesktop.Prerequisites',
          '--add Microsoft.ComponentGroup.Blend',
          '--add Microsoft.VisualStudio.Component.DotNetModelBuilder',
          '--add Microsoft.VisualStudio.Workload.ManagedDesktop',
          '--add Microsoft.VisualStudio.Component.VC.Redist.14.Latest',
          '--add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core',
          '--add Microsoft.VisualStudio.ComponentGroup.WebToolsExtensions.CMake',
          '--add Microsoft.VisualStudio.Component.VC.CMake.Project',
          '--add Microsoft.VisualStudio.Component.VC.ATL',
          '--add Microsoft.VisualStudio.Component.VC.TestAdapterForBoostTest',
          '--add Microsoft.VisualStudio.Component.VC.TestAdapterForGoogleTest',
          '--add Microsoft.VisualStudio.Component.VC.CLI.Support',
          '--add Microsoft.VisualStudio.Component.VC.ASAN',
          '--add Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset',
          '--add Microsoft.VisualStudio.Component.VC.Llvm.Clang',
          '--add Microsoft.VisualStudio.Workload.NativeDesktop',
          '--add Microsoft.VisualStudio.Workload.NetCoreTools',
		  '--quiet')

choco install visualstudio2019community -confirm --package-parameters "$args"

write-host "Installed Visual Studio 2019 Community."

# Avoids reboot error code
Exit 0
