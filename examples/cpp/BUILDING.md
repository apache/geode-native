# Building

## Prerequisites (All Platforms)
* [CMake 3.8](https://cmake.org/) or newer
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

    $<TBD>

## Generator
CMake uses a "generator" to produce configuration files for use by a variety of build tools, e.g., UNIX makefiles, Visual Studio projects. By default a system-specific generator is used by CMake during configuration. (Please see [the CMake documentation](https://cmake.org/documentation/) for further information.) However, in many cases there is a better choice.

### Mac OS X Generator
The recommended generator on Mac OS X is `Xcode`:

	$ cmake -G "Xcode" ..

### Windows Generator
The recommended generator on Windows is `Visual Studio 14 2015 Win64`:

	$ cmake -G "Visual Studio 14 2015 Win64" ..


# Platform-Specific Prerequisites

## Mac OS X
* Mac OS X 10.12 (Sierra) or newer
* XCode 8.2 or newer

### Required Tools
* [XCode](https://developer.apple.com/xcode/download/)
* Xcode command line developer tools

    `$ xcode-select --install`

### Optional Tools
* [CMake GUI](https://cmake.org/)
* [Eclipse CDT 8.8](https://eclipse.org/cdt/) or newer

## Windows
* Windows 8.1 64-bit
* Windows 10 64-bit
* Windows Server 2012 R2 64-bit
* Windows Server 2016 64-bit

### Required Tools
* [Visual Studio 2015](https://www.visualstudio.com) or newer
* .NET 4.5.2 or later
* patch.exe (available as part of GnuWin)

## Linux
* RHEL/CentOS 6
* RHEL/CentOS 7
* SLES 11
* SLES 12

### Required Tools
* [GCC 5](https://gcc.gnu.org) or newer

### Optional Tools
* [Eclipse CDT 8.8](https://eclipse.org/cdt/) or newer

## Solaris
* Solaris 11 SPARC
* Solaris 11 x86

### Required Tools
* [Solaris Studio 12.6](http://www.oracle.com/technetwork/server-storage/developerstudio/downloads/index.html) or newer
