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
