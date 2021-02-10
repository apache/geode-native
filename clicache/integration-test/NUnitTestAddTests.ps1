﻿Param
(
    [Parameter(Mandatory)]
    [string]$AssemblyPath,
    [Parameter(Mandatory)]
    [string]$NUnitConsolePath,
    [Parameter(Mandatory)]
    [string]$WorkingDirectory,
    [Parameter(Mandatory)]
    [string]$CTestFile,
    [Parameter(Mandatory)]
    [string]$SourcePath,
    [Parameter(Mandatory)]
    [string]$ServerLogLevel,
    [Parameter(Mandatory)]
    [string]$ServerSecurityLogLevel,
    [Parameter(Mandatory)]
    [string]$ServerClasspath,
    [Parameter(Mandatory)]
    [string]$GeodePath,
    [Parameter(Mandatory)]
    [string]$JavaPath,
    [Parameter(Mandatory = $FALSE)]
    [string]$EnvPath
)

#$ErrorActionPreference = "Stop

Remove-Item -Path $CTestFile -Force -ErrorAction Ignore 

$nunit = [reflection.assembly]::LoadFrom("nunit.Framework.dll")

$assembly = [reflection.assembly]::LoadFrom($AssemblyPath)

foreach ($type in $assembly.GetTypes())
{
    $ignoreFixture = $FALSE;

    foreach ($customAttribute in $type.CustomAttributes)
    {
        if ($customAttribute.AttributeType.Equals([NUnit.Framework.IgnoreAttribute]))
        {
            $ignoreFixture = $TRUE
        }
    }

    foreach ($method in $type.GetMethods())
    {
        foreach ($customAttribute in $method.CustomAttributes)
        {
            if ($customAttribute.AttributeType.Equals([NUnit.Framework.TestAttribute]))
            {
                $testName = "$($method.ReflectedType.NameSpace).$($method.ReflectedType.Name).$($method.Name)"
                Add-Content -Path $CTestFile -Value "add_test( $testName [==[$NUnitConsolePath]==] /run:$testName [==[$AssemblyPath]==])"
                Add-Content -Path $CTestFile -Value "set_tests_properties( $testName PROPERTIES `
                        WORKING_DIRECTORY [==[$WorkingDirectory]==] `
                        ENVIRONMENT [==[TESTSRC=$SourcePath;GFE_LOGLEVEL=$ServerLogLevel;GFE_SECLOGLEVEL=$ServerSecurityLogLevel;GF_JAVA=$JavaPath;GFE_DIR=$GeodePath;GF_CLASSPATH=$ServerClasspath;PATH=$EnvPath]==] `
                        )"
                if ($ignoreFixture -or $customAttribute.AttributeType.Equals([NUnit.Framework.IgnoreAttribute]))
                {
                    Add-Content -Path $CTestFile -Value "set_tests_properties( $testName PROPERTIES DISABLED True )"
                }
            }
        }
    }
}

