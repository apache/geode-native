# Geode Native C++ Examples

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Install [CMake](https://cmake.org/download/)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* For Windows, Visual Studio 2015

## Building the Examples

1. Copy the `examples` directory from the native client installation folder to a folder in your workspace.

1. Navigate to the directory for a specific example:

    ```
    $ cd workspace/examples/cpp/<example>
    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build . -- <optional parallelism parameters>
    ```

## Running the Examples
To run an example,

1. Navigate to the directory for a specific example.
2. Follow the directions in the `README.md` file in that directory.

