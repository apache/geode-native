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

$GFSH_PATH = ""
if (Get-Command gfsh -ErrorAction SilentlyContinue)
{
    $GFSH_PATH = "gfsh"
}
else
{
    if (-not (Test-Path env:GEODE_HOME))
    {
        Write-Host "Could not find gfsh.  Please set the GEODE_HOME path. e.g. "
        Write-Host "(Powershell) `$env:GEODE_HOME = <path to Geode>"
        Write-Host " OR"
        Write-Host "(Command-line) set %GEODE_HOME% = <path to Geode>"
        exit
    }
    else
    {
        $GFSH_PATH = "$env:GEODE_HOME\bin\gfsh.bat"
    }
}

if ($GFSH_PATH -ne "")
{
   Invoke-Expression "$GFSH_PATH -e 'connect --locator=localhost[10335] --user=server --password=server' -e 'shutdown --include-locators=true'"
   Invoke-Expression "$GFSH_PATH -e 'connect --locator=localhost[10334]' -e 'shutdown --include-locators=true'"
   Start-Sleep -Seconds 5
   Remove-Item -LiteralPath "locator" -Force -Recurse
   Remove-Item -LiteralPath "server" -Force -Recurse
   Remove-Item -LiteralPath "auth_locator" -Force -Recurse
   Remove-Item -LiteralPath "auth_server" -Force -Recurse
}
 