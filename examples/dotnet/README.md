# Geode Native .NET Examples

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Install [CMake](https://cmake.org/download/)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Examples
* [Apache.Geode.Examples.AuthInitialize](Apache.Geode.Examples.Cache/README.md)
* [Apache.Geode.Examples.Cache](Apache.Geode.Examples.Cache/README.md)

## Using
1) Use cmake to generate the .sln and .vcxproj files for the Examples
```
    cd examples\dotnet
    mkdir build
    cd build
    cmake .. -G"Visual Studio 14 2015 Win64" -DGEODE_ASSEMBLY=C:\path\to\Apache.Geode.dll -Thost=x64
```
2) Open `Apache.Geode.Examples.sln` in [Visual Studio 2015](https://www.visualstudio.com/)
or newer.
