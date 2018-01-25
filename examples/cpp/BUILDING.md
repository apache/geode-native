# Building

## Prerequisites (All Platforms)
* [CMake 3.5](https://cmake.org/) or newer
* C++11 compiler *(see platform specific requirements)*
* [Apache Geode](http://geode.apache.org/releases/) binaries installed or available to link against

   Running requires access to an installation of Geode. By default the value of `GEODE_HOME` is used as the path for the startserver.sh script, if gfsh is not in the path.

### Platform-Specific Prerequisites
* [Mac OS X](#mac-os-x)
* [Linux](#linux)
* [Solaris](#solaris)
* [Windows](#windows)

## Steps to build

**Mac OS X / \*nix**

    $ cd <example>
    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build . -- <optional parallelism parameters>

**Windows**

    $ cd <example>
    $ md build
    $ cd build
    $ cmake ..
    $ cmake --build . -- <optional parallelism parameters>

## Steps to run

**Mac OS X / \*nix**

    $ cd <example>
    $ sh ./startserver.sh
    $ build/<example name>
    $ sh ./stopserver.sh

**Windows**

    $ <TBD>

## Generator
CMake uses a "generator" to produce configuration files for use by a variety of build tools, e.g., UNIX makefiles, Visual Studio projects. By default a system-specific generator is used by CMake during configuration. (Please see [the CMake documentation](https://cmake.org/documentation/) for further information.) However, in many cases there is a better choice.

### Mac OS X Generator
The recommended generator on Mac OS X is `Xcode`:

    $ cmake -G "Xcode" ..

### Windows Generator
The recommended generator on Windows is `Visual Studio 14 2015 Win64`:

    $ cmake -G "Visual Studio 14 2015 Win64" ..


### Building requirements
Please see the documentation for building Geode Native.