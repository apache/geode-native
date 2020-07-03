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

Set-Variable log -Option Constant -Scope Local -Value c:\ssh-init.log
Set-Variable scriptPath -Option Constant -Scope Local -Value (Join-Path $PSScriptRoot -ChildPath $MyInvocation.MyCommand.Name)
Set-Variable authorizedKeysPath -Option Constant -Scope Local -Value (Join-Path $env:ProgramData -ChildPath "ssh\administrators_authorized_keys")
Set-Variable sskKeysPath -Option Constant -Scope Local -Value (Join-Path $env:ProgramData -ChildPath "ssh\ssh-keys")

function Write-Log {
    Write-Host "$args"
    Add-Content -Path $log -Value "$args"
}

if ($Schedule) {
  Write-Log "Scheduling SSH Authorized Keys Initialization..."
  schtasks.exe /Create /F /TN "SSH Authorized Keys Initialization" /RU SYSTEM /SC ONSTART /TR "powershell.exe -File '$scriptPath'"
  Write-Log "Scheduled SSH Authorized Keys Initialization."
  Exit 0
}

Write-Log "Initializing SSH Authorized Keys..."

Invoke-WebRequest -Headers @{'Metadata-Flavor'='Google'} -Uri 'http://metadata.google.internal/computeMetadata/v1/instance/attributes/ssh-keys' -OutFile "$sskKeysPath"
Get-Content -Path "$sskKeysPath" -OutVariable sshKeys

Write-Log "Got ssh-keys: $sshKeys"

Remove-Item -Path "$authorizedKeysPath" -Force
$sshKeys = $sshKeys.Split([Environment]::NewLine, [System.StringSplitOptions]::RemoveEmptyEntries)
foreach ($sshKey in $sshKeys) {
    $sshKey = $sshKey.Split(':')
    $user = $sshKey[0]
    $key = $sshKey[1]

    Write-Log "Adding admin user $user with key $key."

    New-LocalUser -AccountNeverExpires -Name $user -NoPassword
    Add-LocalGroupMember -Group "Administrators" -Member $user

    Add-Content -Path "$authorizedKeysPath" -Value "$key"
    icacls "${authorizedKeysPath}" /inheritance:r /grant "SYSTEM:(F)" /grant "BUILTIN\Administrators:(F)"
}

Write-Log "Initialized SSH Authorized Keys."
