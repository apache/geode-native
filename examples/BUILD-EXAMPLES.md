# Geode Native C++ and .Net Examples

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Install [CMake](https://cmake.org/download/)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Set `GEODE_HOME` to the install directory of Apache Geode
* **For Windows:** Visual Studio 2017
* Optional: OpenSSL (to run the SSL examples)

## Building the Examples

1. Copy the `examples` directory from the native client installation folder to a folder in your workspace.

   **For Windows:** For reliable behavior of the example scripts, do not choose a workspace with directory names or filenames that contain spaces.

1. Navigate to the examples directory and use `cmake` to build all examples. For non-Windows systems, the commands are:

    ```
    $ cd workspace/examples
    $ mkdir build
    $ cd build
    $ cmake .. -DGeodeNative_ROOT="<geode-native-install-dir>"
    $ cmake --build . -- <optional parallelism parameter>
    ```
  where the optional parallelism parameter is of the form `-j n`.
  
   **For Windows:** Include options that specify the generator, the architecture toolset, and the path to the Geode Native installation:

    ```
    $ cd workspace\examples\
    $ mkdir build
    $ cd build
    $ cmake .. -G"Visual Studio 15 2017 Win64" -DGeodeNative_ROOT="<geode-native-install-dir>"  -Thost=x64
    $ cmake --build . -- <optional parallelism parameter>
    ```
    
## Running the Examples
To run an example,

1. Navigate to the directory for a specific example.
2. Follow the directions in the `README.md` file in that directory.

