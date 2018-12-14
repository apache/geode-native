$package = 'doxygen.install'
$url  = 'http://doxygen.nl/files/doxygen-1.8.14-setup.exe'
$sha256 = '7B1A361E0C94ADB35EB75EEE069223E7AE19A4444C5B87F65539F7951BF96025'

Import-Module C:\ProgramData\chocolatey\helpers\chocolateyInstaller.psm1
Install-ChocolateyPackage $package 'exe' '/VERYSILENT' $url -Checksum $sha256 -ChecksumType 'sha256'

$oldpath = (Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).path
$newpath = "$oldpath;C:\Program Files\doxygen\bin"
Set-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH -Value $newPath
$ENV:PATH=$newpath