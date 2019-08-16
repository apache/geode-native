$package = 'doxygen.install'
$url  = 'http://doxygen.nl/files/doxygen-1.8.16-setup.exe'
$sha256 = 'c0d4bb19e87921b4aad2d0962bac1f6664bfb9d0f103658908af76565386c940'

Import-Module C:\ProgramData\chocolatey\helpers\chocolateyInstaller.psm1
Install-ChocolateyPackage $package 'exe' '/VERYSILENT' $url -Checksum $sha256 -ChecksumType 'sha256'

$oldpath = (Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).path
$newpath = "$oldpath;C:\Program Files\doxygen\bin"
Set-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH -Value $newPath
$ENV:PATH=$newpath
