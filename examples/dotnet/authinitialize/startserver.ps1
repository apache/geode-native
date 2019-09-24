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
    }
    else
    {
        $GFSH_PATH = "$env:GEODE_HOME\bin\gfsh.bat"
    }
}

if ($GFSH_PATH -ne "")
{
    # Set this variable to include your java object that implements the Authenticator class
    $RESOLVEDPATH = Resolve-Path -Path "$PSScriptRoot/../../utilities/example.jar"

    $COMMON_OPTS = "--J=-Dgemfire.security-username=server"
    $COMMON_OPTS = "$COMMON_OPTS --J=-Dgemfire.security-password=server"
    $COMMON_OPTS = "$COMMON_OPTS --classpath=$RESOLVEDPATH"

    $LOCATOR_OPTS = "$COMMON_OPTS --J=-Dgemfire.security-manager=javaobject.SimpleSecurityManager"

    Invoke-Expression "$GFSH_PATH  -e 'start locator --name=locator $LOCATOR_OPTS' -e 'connect --locator=localhost[10334] --user=server --password=server' -e 'start server --name=server $COMMON_OPTS'  -e 'create region --name=region --type=PARTITION'"
}
