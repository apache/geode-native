# Building

## Prerequisites (All Platforms)
* [CMake 3.8](https://cmake.org/) or newer
* C++11 compiler *(see platform specific requirements)*
* [Doxygen 8.11](http://www.stack.nl/~dimitri/doxygen/download.html) *(for building source documentation)*
* [Apache Geode](http://geode.apache.org/releases/) binaries installed or available to link against

   Building requires access to an installation of Geode. By default the value of `GEODE_ROOT` or `GEODE` is used during CMake configuration if either of those shell variables is exported. To explicitly specify the location in which Geode is installed, add `-DGEODE_ROOT=/path/to/geode` to the _initial_ `cmake` execution command.

### Platform-Specific Prerequisites
* [Mac OS X](#mac-os-x)
* [Linux](#linux)
* [Solaris](#solaris)
* [Windows](#windows)

## Steps to build

    $ cd <clone>
    $ mkdir build
    $ cd build
    $ cmake .. <platform-specific generator parameters (see below)>
    $ cmake --build . -- <platform-specific parallelism parameters (see below)>

### Generator
CMake uses a "generator" to produce configuration files for use by a variety of build tools, e.g., UNIX makefiles, Visual Studio projects. By default a system-specific generator is used by CMake during configuration. (Please see [the CMake documentation](https://cmake.org/documentation/) for further information.) However, in many cases there is a better choice.

#### Mac OS X
The recommended generator on Mac OS X is `Xcode`:

	$ cmake .. -G "Xcode"

#### Windows
When running cmake commands on Windows, be sure to use [Visual Studio Native Tools Command Prompt](https://msdn.microsoft.com/en-us/library/f35ctcxw.aspx) so environment variables are set properly.

The recommended generator on Windows is `Visual Studio 14 2015 Win64`:

	$ cmake .. -G "Visual Studio 14 2015 Win64" -Thost=x64

### Build Parallelism
For faster builds, use optional parallelism parameters in the last build step:

#### Unix

	$ cmake --build . -- -j <# of jobs>

#### Windows

	$ cmake --build . -- /m

## Installing
By default a system-specific location is used by CMake as the destination of the `install` target, e.g., `/usr/local` on UNIX system. To explicitly specify the location in which the Native Client will be installed, add `-DCMAKE_INSTALL_PREFIX=/path/to/installation/destination` to the _initial_ `cmake` execution command.

**Note:** For consistent results, avoid using the "~" (tilde) abbreviation when specifying paths on the CMake command line.
Interpretation of the symbol varies depending on the option being specified, and on the system or command shell in use.

Due to limitations in CMake, the documentation must be built as a separate step before installation:

    $ cd <clone>
    $ cd build
    $ cmake --build . --target docs
    $ cmake --build . --target install

# Platform-Specific Prerequisites

## Mac OS X
* Mac OS X 10.12 (Sierra) or newer
* Xcode 8.2 or newer

### Required Tools
* [Xcode](https://developer.apple.com/xcode/download/)
* Xcode command line developer tools

    `$ xcode-select --install`

### Optional Tools
* [CMake GUI](https://cmake.org/)
* [Doxygen GUI](http://ftp.stack.nl/pub/users/dimitri/Doxygen-1.8.11.dmg)
* [Eclipse CDT 8.8](https://eclipse.org/cdt/) or newer
* [CLion](https://www.jetbrains.com/clion/)

## Windows
* Windows 8.1 64-bit
* Windows 10 64-bit
* Windows Server 2012 R2 64-bit
* Windows Server 2016 64-bit

### Required Tools
* [Visual Studio 2015](https://www.visualstudio.com) or newer
* .NET 4.5.2 or later
* Chocolately
* [Other dependencies installed via Powershell](packer/windows/install-dependencies.ps1)

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
