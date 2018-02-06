# Contributing
This document assumes you have followed the [Apache Geode Code contribution instructions](https://cwiki.apache.org/confluence/display/GEODE/Code+contributions)

## Building the code
    see BUILDING.md

## Next steps
* Make your changes/add your feature/fix a bug.
* Test your feature branch changes
* Check your formatting
* Submit a pull request

## Testing
   Before submitting a pull request the unit and integration tests must all pass. We are using CTest, (Please see [the CTest documentation](https://cmake.org/Wiki/CMake/Testing_With_CTest) for further information.)
### Running unit tests
    $ cd <clone>
    $ cd build

   The following steps will be updated once the "run-unit-tests" target is fixed.

    $ cd cppcache/test/<Debug|Release|if needed>
    $ ./gfcppcache_unittests
### Running integration tests
    $ cd <clone>
    $ cd build
    $ cmake --build . --target run-integration-tests

   Which is equivalent to running ctest directly:

    $ cd build/cppcache/integration-test
    $ ctest --timeout 2000 -L STABLE -C <Debug|Release> -R . -j1
   This will take ~ 2 hours, YMMV... you can up the jobs to 4 and run in parallel, but you may end up with test failures that will need to be re-run sequentially.  Like so:

    $ cd build/cppcache/integration-test
    $ ctest -R <test_name> -C <Debug|Release>

## Formatting C++
For C++ it is required to follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and have a build target that uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to achieve compliance.

    $ clang-format -i --style=file <PATH_TO_FILES>

# System Requirements
See [BUILDING.md](BUILDING.md)

