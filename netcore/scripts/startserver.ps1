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

if ($env:BUILD_HOME -eq "" -or $env:BUILD_HOME -eq $null)
{
    Write-Host "Please set BUILD_HOME path to the root directory of a geode-native examples build."
    Write-Host "startserver.ps1 needs to find 'example.jar' under BUILD_HOME/utilities to work properly."
    exit
}

$AUTH_OPTS="--J=-Dgemfire.security-username=server"
$AUTH_OPTS="$AUTH_OPTS --J=-Dgemfire.security-password=server"
$AUTH_OPTS="$AUTH_OPTS --classpath=$env:BUILD_HOME/utilities/example.jar"

$AUTH_LOCATOR_OPTS="$AUTH_OPTS --J=-Dgemfire.security-manager=javaobject.SimpleSecurityManager" 

if ($GFSH_PATH -ne "")
{
   $expression = "$GFSH_PATH " + `
        "-e 'start locator --name=locator --port=10334 --http-service-port=6060 --J=-Dgemfire.jmx-manager-port=1099' " + `
        "-e 'start server --name=server --server-port=0' " + `
        "-e 'create region --name=exampleRegion --type=PARTITION'";
   Invoke-Expression $expression

   $expression = "$GFSH_PATH " + `
        "-e 'start locator --name=auth_locator $AUTH_LOCATOR_OPTS --port=10335 --http-service-port=7070 --J=-Dgemfire.jmx-manager-port=2099' " + `
        "-e 'connect --locator=localhost[10335] --user=server --password=server' " + `
        "-e 'start server --name=auth_server $AUTH_OPTS --server-port=0' " + `
        "-e 'create region --name=authExampleRegion --type=PARTITION'";
   Invoke-Expression $expression
}
 