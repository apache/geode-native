# Contributing
This document assumes you have followed the [Apache Geode Code contribution instructions](https://cwiki.apache.org/confluence/display/GEODE/Code+contributions)

## Building the code
see [BUILDING.md](BUILDING.md)

## Next steps
* Make your changes/add your feature/fix a bug.
* Conform to established [versioning requirements](README.md#versioning).
* [Test](#testing) your feature branch changes.
* Check your [formatting](#clang-format) and [style](#clang-tidy).
* Submit a pull request.

## Testing
Before submitting a pull request all tests must pass. This includes all unit tests, integration tests, and acceptance 
tests. We are using CTest for running tests (please see [the CTest documentation](https://cmake.org/Wiki/CMake/Testing_With_CTest)
for further information) and [Google Test](https://github.com/google/googletest) as testing framework.

### Running unit tests
```console
$ cd build/cppcache/test/<Debug|Release|if needed>
$ ./apache-geode_unittests
```

### Running integration tests
There are two test suites of integration tests based on different testing frameworks. The old integration tests (stored
in `geode-native/cppcache/integration-test`) are based on a custom testing framework, and the newer ones (stored in 
`geode-native/cppcache/integration/test`) are based on [Google Test](https://github.com/google/googletest).

Old integration tests are deprecated. If your changes include the implementation of new integration test/s to be
verified, they should be written using Google Test. If your change implies a significant change in one or more old test
cases, you should create the equivalent test case/s using Google Test to substitute the old one/s instead of adapting
them.

Both integration test suites can be executed using CTest.

#### Running heritage integration test suite
```console
$ cd build/cppcache/integration-test
$ ctest -C <Debug|RelWithDebInfo> --timeout 300 -j1
```
Execution will take 2 hours approximately. It is possible to increase the number of jobs changing the value of `-j`
parameter (up to `4`) for running tests in parallel, although it may end up with failed tests that will need to be
re-run sequentially.

Standalone tests can also be run as follows:
```console
$ cd build/cppcache/integration-test
$ ctest -C <Debug|RelWithDebInfo> -R <test_name>
```
For example:
```console
$ ctest -C RelWithDebInfo -R testCacheless
```

.NET integration tests can be executed similarly from `build/clicache/integration-test`.

#### Running new Google Test integration test suite
Make sure Docker is installed as it is required for SNI Tests
```console
$ cd build/cppcache/integration/test
$ ctest -C <Debug|RelWithDebInfo> -j1
```

It is possible to increase the number of jobs changing the value of `-j` parameter for running tests in parallel,
although it may end up with failed tests that will need to be re-run sequentially. Standalone tests can also be run as
follows:
```console
$ cd build/cppcache/integration/test
$ ctest -C <Debug|RelWithDebInfo> -R <test_name> -j1
```
For example:
```console
$ ctest -C RelWithDebInfo -R AuthInitializeTest.putGetWithBasicAuth -j1
```

Notice that `BasicIPv6Test` test is expected to fail due to IPv6 support is disabled by default. [BUILDING.md](BUILDING.md)
explains how to enable it.

### Running acceptance tests
Acceptance tests is a new category of tests that are designed to test end to end connectivity of a geode-native client
with a cloud based geode cluster that sits behind a proxy server.

These tests are stored in `geode-native/cppcache/acceptance-test` and `geode-native/clicache/acceptance-test` for C++
and .NET clients respectively. They utilize docker containers for the proxy server and geode servers. They are enabled
during cmake configuration only if docker and docker-compose are found. 

The acceptance tests can be run using ctest as follows. Note: Currently, the tests can only be run sequentially, hence
the -j1 flag below.

For C++ clients:
```console
$ cd build/cppcache/acceptance-test
$ ctest -C <Debug|RelWithDebInfo> -R <test_name> -j1
```
For .NET clients:
```console
$ cd build/clicache/acceptance-test
$ ctest -C <Debug|RelWithDebInfo> -R <test_name> -j1
```

## Style

### Formatting C++
For C++ it is required to follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and
have a build target that uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to achieve compliance.

#### Clang-format

Individual targets in the build tree have their own dependency of the form `<<targetName>>-clangformat`, which uses
the `clang-format` executable, wherever it is found, to format and modified files according to the rules specfied in the
.clang-format file. This is helpful when submitting changes to geode-native, because an improperly formatted file will
fail Travis-CI and have to be fixed prior to merging any pull request. If clang-format is not installed on your system,
clangformat targets will not be added to your project files, and geode-native should build normally. Under some
circumstances, however, it may become necessary to disable `clang-format` on a system where it _is_ installed. The
clang-format rules are defined in the `.clang-format` files.

To enable `clang-tidy` if CMake does not find `clang-format`:

```console
$ cmake … -DClangFormat_EXECUTABLE=/path/to/clang-format …
```

To disable `clang-format` in the build:

```console
$ cmake … -DClangFormat_EXECUTABLE='' …
```

On the other hand, it may also be desirable to run clang-format on the entire source tree. This is also easily done via
the `all-clangformat` _in a build with clang-format enabled_. If clang-format has been disabled in the cmake
configuration step, as above, the `all-clangformat` target will not exist, and the cmake configuration step will have to
be re-run with clang-format enabled.

To run clang-format on the entire source tree:

```console
$ cmake --build . --target all-clangformat
```

#### Clang-Tidy

Code style and conventions are enforced with clang-tidy. The clang-tidy rules can be found in the `.clang-tidy` files.

To enable `clang-tidy`:

```console
$ cmake … -DCMAKE_CXX_CLANG_TIDY=/path/to/clang-tidy …
```

### Code Cleanup
When editing old code please make the following changes.

* Prefer the use of [`auto`](http://en.cppreference.com/w/cpp/language/auto) C++ or [`var`](https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/var)
  in C# where applicable.
  ```c++
  std::vector<Cars> cars = lot.getCars();
  ```
  should be changed to
  ```c++
  auto cars = lot.getCars();
  ```

* Prefer [range for](http://en.cppreference.com/w/cpp/language/range-for) loops over traditional for loops where
  applicable.
  ```c++
  for (std::vector<Car>::iterator i = cars.begin(); i != cars.end(), ++i) {
    Car car = *i;
    std::cout << car.getName();
  }
  ```
  should be changed to
  ```c++
  for (const auto& car : cars) {
    std::cout << car.getName();
  }
  ```

* Fix bad variable names. Variable names should be expressive.
  ```c++
  double i = car.getFuelLevel();
  ```
  should be changed to
  ```c++
  auto fuelLevel = car.getFuelLevel();
  ```

* Use [`override`](http://en.cppreference.com/w/cpp/language/override) on all method overrides.

  Given a base class
  ```c++
  class Base {
   public:
    Base();
    virtual ~Base();
    virtual virtualMethod();
    virtual pureVirtualMethod() = 0;
  }
  ```
  the derived class
  ```c++
  class Derived : public Base {
   public:
    virtual ~Derived();
    virtual virtualMethod();
    virtual pureVirtualMethod();
  }
  ```
  should be changed to
  ```c++
  class Derived : public Base {
   public:
    Derived();
    ~Derived() override;
    virtualMethod() override;
    pureVirtualMethod() override;
  }
  ```

* Fix [`std::string::c_str()`](http://en.cppreference.com/w/cpp/string/basic_string/c_str) calls where passed to 
  [`std::string`](http://en.cppreference.com/w/cpp/string/basic_string) parameters.
  ```c++
  auto name = std::string("Delorean");
  ...
  car.setName(name.c_str());
  ```
  should be changed to
  ```c++
  auto name = std::string("Delorean");
  ...
  car.setName(name);
  ```

* Replace [`sprintf`](http://en.cppreference.com/w/cpp/io/c/fprintf) for logging messages with [`std::string`](http://en.cppreference.com/w/cpp/string/basic_string)
  or [`std::stringstream`](http://en.cppreference.com/w/cpp/io/basic_stringstream).
  ```c++
  char[1024] buffer;
  sprintf(buffer, "Car::crashed: name=%s", car.getName().c_str());
  LOG(buffer);
  ```
  should be changed to
  ```c++
  LOG("Car::crashed: name=" + car.getName());
  ```

* Replace [`dynamic_cast`](http://en.cppreference.com/w/cpp/language/dynamic_cast) on [`std::shared_ptr`](http://en.cppreference.com/w/cpp/memory/shared_ptr)
  with [`std::dynamic_pointer_cast`](http://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast). Same goes for
  [`static_cast`](http://en.cppreference.com/w/cpp/language/static_cast) to [`std::static_pointer_cast`](http://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast),
  etc.
  ```c++
  std::shared_ptr<Car> car = garage.getCar();
  Delorean* delorean = dynamic_cast<Delorean*>(car.get());
  if (nullptr != delorean) {
    delorean->setSpeed(88);
  }
  ```
  should be changed to
  ```c++
  auto car = garage.getCar();
  if (auto delorean = std::dynamic_pointer_cast<Delorean>(car)) {
    delorean->setSpeed(88);
  }
  ```

## Code Coverage

If building with GCC or Clang you can enable C++ code coverage by adding `-DUSE_CPP_COVERAGE=ON` to the CMake generator
command.

```console
$ cmake .. -DUSE_CPP_COVERAGE=ON …
```

You can then generate a C++ code coverage report by downloading [lcov](http://ltp.sourceforge.net/coverage/lcov.php).
After acquiring lcov, finish the [Steps to build](BUILDING.md#Steps-to-build) section above. Then, run the tests as 
described in the this document. Finally, run the following commands from the `build` directory:

```console
$ lcov --capture --directory . --output-file coverage.info
$ genhtml coverage.info --output-directory coverage_report
```

You can then open the `index.html` file in the `coverage_report` directory using any browser.
