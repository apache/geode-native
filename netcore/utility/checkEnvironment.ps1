if (-not (Test-Path env:GEODE_HOME))
{
	Write-Host "GEODE_HOME environment variable not set"
	exit 1
}

if (-not (Test-Path env:GEODE_NATIVE_BUILD_DIR))
{
	Write-Host "GEODE_NATIVE_BUILD_DIR environment variable not set"
	exit 1
}

exit 0
