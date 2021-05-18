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

Param
(
    [Parameter(Mandatory)]
    [string]$AssemblyPath,
    [Parameter(Mandatory)]
    [string]$XUnitConsolePath,
    [Parameter(Mandatory)]
    [string]$CTestFile
)

#$ErrorActionPreference = "Stop

Remove-Item -Path $CTestFile -Force -ErrorAction Ignore 

$xunit = [reflection.assembly]::LoadFrom("xunit.core.dll")

$assembly = [reflection.assembly]::LoadFrom($AssemblyPath)

foreach ($type in $assembly.GetTypes())
{
    foreach ($method in $type.GetMethods())
    {
        foreach ($customAttribute in $method.CustomAttributes)
        {
            if ($customAttribute.AttributeType.Equals([Xunit.FactAttribute]))
            {
                $testName = "$($method.ReflectedType.NameSpace).$($method.ReflectedType.Name).$($method.Name)"
                Add-Content -Path $CTestFile -Value "add_test( $testName [==[$XUnitConsolePath]==] [==[$AssemblyPath]==] -nologo -method $testName )"
                foreach ($namedArgument in $customAttribute.NamedArguments)
                {
                    if ($namedArgument.MemberName -eq "Skip")
                    {
                        Add-Content -Path $CTestFile -Value "set_tests_properties( $testName PROPERTIES DISABLED True )"
                    }
                }
            }
        }
    }
}
