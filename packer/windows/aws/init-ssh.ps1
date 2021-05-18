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

[CmdletBinding(DefaultParameterSetName = 'Default')]
param (
    # Schedules the script to run on the next boot.
    # If this argument is not provided, script is executed immediately.
    [parameter(Mandatory = $false, ParameterSetName = "Schedule")]
    [switch] $Schedule = $false
)

Set-Variable modulePath -Option Constant -Scope Local -Value (Join-Path $env:ProgramData -ChildPath "Amazon\EC2-Windows\Launch\Module\Ec2Launch.psd1")
Set-Variable scriptPath -Option Constant -Scope Local -Value (Join-Path $PSScriptRoot -ChildPath $MyInvocation.MyCommand.Name)
Set-Variable authorizedKeysPath -Option Constant -Scope Local -Value (Join-Path $env:ProgramData -ChildPath "ssh\administrators_authorized_keys")

Import-Module $modulePath

Initialize-Log -Filename "Ec2Launch.log" -AllowLogToConsole

if ($Schedule) {
  Write-Log "Scheduling SSH Authorized Keys Initialization..."
  Register-ScriptScheduler -ScriptPath $scriptPath -ScheduleName "SSH Authorized Keys Initialization"
  Write-Log "Scheduling SSH Authorized Keys Initialization."

  Complete-Log
  Exit 0
}

Write-Log "Initializing SSH Authorized Keys..."

Invoke-WebRequest -Uri "http://169.254.169.254/latest/meta-data/public-keys/0/openssh-key" -OutFile $authorizedKeysPath
icacls $authorizedKeysPath /inheritance:r /grant "SYSTEM:(F)" /grant "BUILTIN\Administrators:(F)"

Write-Log "Initialized SSH Authorized Keys."

Complete-Log
Exit 0
