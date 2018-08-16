# Geode Native .NET Examples

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Install [CMake](https://cmake.org/download/)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Examples
* [Apache.Geode.Examples.AuthInitialize](Apache.Geode.Examples.Cache/README.md)
* [Apache.Geode.Examples.Cache](Apache.Geode.Examples.Cache/README.md)

## Building the Examples

1. Copy the `examples` directory from the native client installation folder to a folder in your workspace.

1. Use cmake to generate the .sln and .vcxproj files for the examples:

    ```
    cd workspace\examples\dotnet
    mkdir build
    cd build
    cmake .. -G"Visual Studio 14 2015 Win64" -DGeodeNative_ROOT="<NC-install-root-dir>"
    ```
The result is a Visual Studio solution for the .NET examples.
1. Open the solution file, `examples.sln`, in [Visual Studio 2015](https://www.visualstudio.com/) or newer and build all projects.


## Running the Examples
To run the examples, 

1.  Decide which example to try first. Follow directions in the README file in the source directory for that example to start the server and create a region. 

1. Assuming you are still in Visual Studio, select the desired example as your StartUp project and execute it.

The example's README file describes the expected output.

