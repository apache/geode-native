# C++ Toolkit Version
Currently we target VC toolset 14.15 which is forward compatible to 14.15+.
* Build Tools Component: `Microsoft.VisualStudio.Component.VC.Tools.14.15`
* Entrypoint: `vcvarsall.bat ... -vcvars_ver=14.15`

We should investigate if we need to go back a few more minors to remain compatible
with previous releases.

# C++ SDK Version
We haven't really figured out if or how to lock down the SDK version. Currently we use
version 10.16299.
* Build Tools Component: `Microsoft.VisualStudio.Component.Windows10SDK.16299.Desktop`
* Entrypoint: `vcvarsall.bat  ... 10.0.16299.0 ...`

ACE library seems to be stuck on 10.16299. If we upgrade in the future we may need to
unstick ACE to avoid mixing of SDKs. It is unclear is mixing SDKs is an issue.

# .NET Framework
We build against version 4.5.2 but require 3.5 for old NUnit 2.6.4 at runtime.
* Base Image: `mcr.microsoft.com/dotnet/framework/runtime:3.5`
* Build Tools Component: `Microsoft.Net.Component.4.5.2.TargetingPack`
