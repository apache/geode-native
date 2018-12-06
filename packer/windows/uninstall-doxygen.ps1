$package = 'doxygen.install'
$uninstallRegKey = 'HKLM:SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\doxygen_is1'

Import-Module C:\ProgramData\chocolatey\helpers\chocolateyInstaller.psm1
$uninstallPath = (Get-ItemProperty $uninstallRegKey UninstallString).UninstallString
Uninstall-ChocolateyPackage $package 'exe' '/VERYSILENT' $uninstallPath