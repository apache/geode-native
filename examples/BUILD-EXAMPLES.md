# Geode Native C++ Examples

## Prerequisites
*a Install [Apache Geode](https://geode.apache.org)
* Install [CMake](https://cmake.org/download/)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* **For Windows:** Visual Studio 2015

## Building the Examples

1. Copy the `examples` directory from the native client installation folder to a folder in your workspace.

   **For Windows:** For reliable behavior of the example scripts, do not choose a workspace with directory names or filenames that contain spaces.

1. Navigate to the directory for a specific example and use `cmake` to build it. For non-Windows systems, the commands are:

    ```
    $ cd workspace/examples
    $ mkdir build
    $ cd build
    $ cmake .. -DGEODE_ROOT="<geode-install-root-dir>"
    $ cmake --build . -- <optional parallelism parameter>
    ```
  where the optional parallelism parameter is of the form `-j n`.
  
   **For Windows:** Include options that specify the generator, the architecture toolset, and the path to the Geode Native installation:

    ```
    $ cd workspace/examples/
    $ mkdir build
    $ cd build
    $ cmake .. -G"Visual Studio 14 2015 Win64" -Thost=x64 -DGeodeNative_ROOT="<NC-install-root-dir>"
    $ cmake --build . -- <optional parallelism parameter>
    ```
  where the optional parallelism parameter is `/m`.
a
## Running the Examples
aTo run an example,

1. Navigate to the directory for a specific example.
2. Follow the directions in the `README.md` file in that directory.

