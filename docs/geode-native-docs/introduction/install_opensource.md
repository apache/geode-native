Cmake is required to build the native client and its quickstart examples. To download and install cmake, follow the instructions on [cmake.org] (https://cmake.org).

Installation on Mac:

- Checkout Geode sources

- Checkout NC branch

- cd geode. NC sources are in 'src' subdir.

- create build dir (e.g. build-nc) at same level as 'src'.

- cmake config: `cmake ../src -DCMAKE_INSTALL_PREFIX=/Users/dbarnes/Repo/geode/build-nc` (if prefix not specified, will be installed to `/usr/local`)

- cmake build: `cmake --build .`

- cmake build: `cmake --build . --target docs`

- cmake install: `cmake --build . --target install`



