# Building

## Prerequisites (All Platforms)
* [CMake 3.10](https://cmake.org/) or newer
* C++11 compiler *(see platform specific requirements)*
* [Doxygen 8.11](http://www.stack.nl/~dimitri/doxygen/download.html) *(for building source documentation)*
* [Apache Geode](http://geode.apache.org/releases/) binaries installed or available to link against

### Platform-Specific Prerequisites
* [Mac OS X](#mac-os-x)
* [Linux](#linux)
* [Solaris](#solaris)
* [Windows](#windows)

## Setting Path to Geode
Building requires access to an installation of Geode. There are two ways to achieve this:

* Set an environment variable called `GEODE_HOME` that points to your Geode installation path.
* Pass in `GEODE_ROOT` during the CMake configuration step.
  * e.g.  add `-DGEODE_ROOT=/path/to/geode` to the _initial_ `cmake` execution command.



## Steps to build
```console
$ cd <clone>
$ mkdir build
$ cd build
# configuration step
$ cmake .. <platform-specific generator parameters (see below)>
# build step
$ cmake --build . -- <platform-specific parallelism parameters (see below)>
```console

To explicitly specify the location in which the Native Client will be installed, add `-DCMAKE_INSTALL_PREFIX=/path/to/installation/destination` to this initial `cmake` execution command.

### Generator
CMake uses a "generator" to produce configuration files for use by a variety of build tools, e.g., UNIX makefiles, Visual Studio projects. By default a system-specific generator is used by CMake during configuration. (Please see [the CMake documentation](https://cmake.org/documentation/) for further information.) However, in many cases there is a better choice.

#### CLion / Eclipse / Other
The recommended generator for most unix platforms is 'Makefiles' (default):
```console
$ cmake ..
```

#### Xcode
The recommended generator for Xcode is `Xcode`:
```console
$ cmake .. -G "Xcode"
```

#### Windows / Visual Studio
When running cmake commands on Windows, be sure to use [Visual Studio Native Tools Command Prompt](https://msdn.microsoft.com/en-us/library/f35ctcxw.aspx) so environment variables are set properly.

The recommended generator on Windows is `Visual Studio 14 2015 Win64`:
```console
$ cmake .. -G "Visual Studio 14 2015 Win64" -Thost=x64
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

#### Code Coverage

If building with GCC or Clang you can enable C++ code coverage by adding `-DUSE_CPP_COVERAGE=ON` to the CMake [Generator](#generator) command. 
```console
$ cmake … -DUSE_CPP_COVERAGE=ON …
```
You can then generate a C++ code coverage report by downloading [lcov](http://ltp.sourceforge.net/coverage/lcov.php).  After acquiring lcov, finish the [Steps to build](#Steps-to-build) section above.  Then, run the tests as described in the [CONTRIBUTING.md](CONTRIBUTING.md). Finally, run the following commands from the `build` directory:
```console
$ lcov --capture --directory . --output-file coverage.info
$ genhtml coverage.info --output-directory coverage_report
```

You can then open the `index.html` file in the `coverage_report` directory using any browser.

#### Clang-Tidy
To enable `clang-tidy`:
```console
$ cmake … -DCMAKE_CXX_CLANG_TIDY=clang-tidy …
```
To use specific `clang-tidy`:
```console
$ cmake … -DCMAKE_CXX_CLANG_TIDY=/path/to/clang-tidy …
```
By default `clang-tidy` uses the configuration found in `.clang-tidy`
To override `clang-tidy` options:
```console
$ cmake … -DCMAKE_CXX_CLANG_TIDY=clang-tidy;<options> …
```

## Installing
By default a system-specific location is used by CMake as the destination of the `install` target, e.g., `/usr/local` on UNIX system. To explicitly specify the location in which the Native Client will be installed, add `-DCMAKE_INSTALL_PREFIX=/path/to/installation/destination` to the _initial_ `cmake` execution command.

**Note:** For consistent results, avoid using the "~" (tilde) abbreviation when specifying paths on the CMake command line.
Interpretation of the symbol varies depending on the option being specified, and on the system or command shell in use.

Due to limitations in CMake, the documentation must be built as a separate step before installation:

```console
$ cd <clone>
$ cd build
$ cmake --build . --target docs
$ cmake --build . --target install
```

# Platform-Specific Prerequisites

## Mac OS X
* Mac OS X 10.12 (Sierra) or newer
* Xcode 8.2 or newer

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
