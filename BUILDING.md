# Building

## Prerequisites (All Platforms)

* [CMake 3.18](https://cmake.org/) or newer
* C++11 compiler *(see platform specific requirements)*
* [Doxygen 1.8.11 or greater](https://sourceforge.net/projects/doxygen/) *(for building source documentation)*
* [OpenSSL](https://www.openssl.org) *(for building source documentation)*
* Java 8 JDK *(for building server side java functions used in some of the integration tests)*
* [Apache Geode](http://geode.apache.org/releases/) binaries installed or available to link against
* [Docker](https://www.docker.com/) (for running SNI Test)
* [Docker Compose](https://docs.docker.com/compose/install/) (for running SNI Test)

### Platform-Specific Prerequisites

* [Windows](#windows)
* [Linux](#linux)
* [Mac OS X](#mac-os-x)
* [Solaris](#solaris)

## Setting Path to Geode

Building requires access to an installation of Geode. There are two ways to achieve this:

* Set an environment variable called `GEODE_HOME` that points to your Geode installation path.
* Pass in `GEODE_ROOT` during the CMake configuration step.
    * e.g. add `-DGEODE_ROOT=/path/to/geode` to the _initial_ `cmake` execution command.

## Steps to build

```console
$ cd <clone>
$ mkdir build
$ cd build
# configuration step
$ cmake .. <platform-specific generator parameters (see below)>
# build step
$ cmake --build . -- <platform-specific parallelism parameters (see below)>
```

If OpenSSL is installed in a custom location, then you must pass `OPENSSL_ROOT_DIR` during the CMake configuration step.
For example, `-DOPENSSL_ROOT_DIR=/path/to/openssl`.

To explicitly specify the location in which the Native Client will be installed,
add `-DCMAKE_INSTALL_PREFIX=/path/to/installation/destination` to this initial `cmake` execution command.

To set the version header on the API docs, specify PRODUCT_VERSION on the configuration command line. For
example, `-DPRODUCT_VERSION=1.2.3`.

### Generator

CMake uses a "generator" to produce configuration files for use by a variety of build tools, e.g., UNIX makefiles,
Visual Studio projects. By default a system-specific generator is used by CMake during configuration. (Please
see [the CMake documentation](https://cmake.org/documentation/) for further information.) However, in many cases there
is a better choice.

#### CLion / Eclipse / Other

The recommended generator for most unix platforms is 'Makefiles' (default):

```console
$ cmake ..
```

#### Mac OSX Xcode

Install XCode from the App Store

* You have to run XCode once to get it initialize properly (software agreement).
* Install the command line tools for xcode - run `xcode-select --install` from terminal

Install the required dependencies through homebrew. If you use another package manager for your mac feel free to use
that.

```console
$ brew install geode
$ brew install openssl
$ brew install doxygen
$ brew install cmake
```

You will need to provide the path to the brew installed OpenSSL headers since macOS already has a system installed
version but without the required headers.

```console
$ cmake .. -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
```

#### Windows / Visual Studio

For Visual Studio 2017 and newer you only need to specify the correct architecture, toolset and SDK for Windows. To
build a 64-bit library using the 64 bit toolset version 14.1 with minimum ABI compatibility of 14.16 for minimum Windows
version 10.0.16299.0 use the following options.

```console
$ cmake .. -A x64 -Tv141,version=14.16,host=x64 -DCMAKE_SYSTEM_VERSION=10.0.16299.0
```

At a bare minimum you will likely need to specify the architecture, since MSVC still defaults to 32 bit, and the 64 bit
version of the toolset, because of large object files. The latest toolset version and Windows SDK will likely get picked
up.

```console
$ cmake .. -A x64 -Thost=x64
```

### Build Parallelism

For faster builds, use optional parallelism parameters in the last build step:

#### Unix

```console
$ cmake --build . -- -j <# of jobs>
```

#### Windows

```console
$ cmake --build . -- /m
```

### Optional Configuration

#### IPv6 support

IPv6 support can be enabled by adding `-DWITH_IPV6=ON` to the CMake [Generator](#generator) command.

```console
$ cmake … -DWITH_IPV6=ON …
```

## Installing

By default a system-specific location is used by CMake as the destination of the `install` target, e.g., `/usr/local` on
UNIX system. To explicitly specify the location in which the Native Client will be installed,
add `-DCMAKE_INSTALL_PREFIX=/path/to/installation/destination` to the _initial_ `cmake` execution command.

**Note:** For consistent results, avoid using the "~" (tilde) abbreviation when specifying paths on the CMake command
line. Interpretation of the symbol varies depending on the option being specified, and on the system or command shell in
use.

Due to limitations in CMake, the documentation must be built as a separate step before installation:

```console
$ cd <clone>
$ cd build
$ cmake --build . --target docs
$ cmake --build . --target install
```

# Platform-Specific Prerequisites

## Windows

* Windows 10 64-bit
* Windows Server 2016 64-bit
* Windows Server 2019 64-bit

### Required Tools

* [Visual Studio](https://www.visualstudio.com) 2017 or newer
* [.NET](https://dotnet.microsoft.com/learn/dotnet/what-is-dotnet-framework) 4.5.2 or later
* Other dependencies installed by [Packer](packer/build-windows-2016-vs-2017.json) scripts

## Linux

* RHEL/CentOS 7
* RHEL/CentOS 8
* Ubuntu 2016.04 (Xenial)
* Ubuntu 2018.04 (Bionic)
* Ubuntu 2020.04 (Focal)

Other distributions and versions may be supported given C++11 compatible compiler and runtime library.

### Required Tools

* [GCC 5](https://gcc.gnu.org) or newer

### Optional Tools

* [CLion](https://www.jetbrains.com/clion/)

## macOS

* macOS X 10.15 (Catalina) or newer
* Xcode 11 or newer

Older versions of macOS or Mac OS X and Xcode may work but are not regularly tested or developed on.
  
### Required Tools

* [Xcode](https://developer.apple.com/xcode/download/)
* Xcode command line developer tools

```console
$ xcode-select --install
```

### Optional Tools

* [CMake GUI](https://cmake.org/)
* [Doxygen GUI](http://ftp.stack.nl/pub/users/dimitri/Doxygen-1.8.11.dmg)
* [CLion](https://www.jetbrains.com/clion/)

## Solaris
* Solaris 11 SPARC
* Solaris 11 x86

Solaris is not actively developed or tested. While no effort has been made to remove Solaris support it is likely
broken.

### Required Tools

* [Solaris Studio 12.6](http://www.oracle.com/technetwork/server-storage/developerstudio/downloads/index.html) or newer
