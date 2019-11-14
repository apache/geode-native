# Contributing
This document assumes you have followed the [Apache Geode Code contribution instructions](https://cwiki.apache.org/confluence/display/GEODE/Code+contributions)

## Building the code
see [BUILDING.md](BUILDING.md)

## Next steps
* Make your changes/add your feature/fix a bug.
* Test your feature branch changes.
* Check your formatting.
* Submit a pull request.

## Testing
Before submitting a pull request the unit and integration tests must all pass. We are using CTest for running tests (please see [the CTest documentation](https://cmake.org/Wiki/CMake/Testing_With_CTest) for further information) and [Google Test](https://github.com/google/googletest) as testing framework.

### Running unit tests
```bash
$ cd <clone>
$ cd build
$ cd cppcache/test/<Debug|Release|if needed>
$ ./apache-geode_unittests
```

### Running integration tests
There are two test suites of integration tests based on different testing frameworks. The old integration tests (stored in `geode-native/cppcache/integration-test`) are based on a custom testing framework, and the newer ones (stored in `geode-native/cppcache/integration/test`) are based on [Google Test](https://github.com/google/googletest).

Old integration tests are deprecated. If your changes include the implementation of new integration test/s to be verified, they should be written using Google Test. If your change implies a significant change in one or more old test cases, you should create the equivalent test case/s using Google Test to substitute the old one/s instead of adapting them.

Both integration test suites can be executed together using CTest.

#### Running old integration test suite
```bash
$ cd build/cppcache/integration-test
$ ctest --timeout 2000 -L STABLE -C <Debug|Release> -j1
```
Execution will take 2 hours approximately. It is possible to increase the number of jobs changing the value of `-j` parameter (up to `4`) for running tests in parallel, although it may end up with failed tests that will need to be re-run sequentially. Standalone tests can also be run as follows:

```bash
$ cd build/cppcache/integration-test
$ ctest -R <test_name> -C <Debug|Release>
```
For example: `$ ctest --timeout 2000 -L STABLE -C Release -R testCacheless -j1`

.NET integration tests can be executed similarly from `build/clicache/integration-test`.

#### Running Google Test integration test suite
```bash
$ cd <clone>
$ cd build/cppcache/integration/test
$ ctest -j1
```
It is possible to increase the number of jobs changing the value of `-j` parameter for running tests in parallel, although it may end up with failed tests that will need to be re-run sequentially. Standalone tests can also be run as follows:

```bash
$ cd <clone>
$ cd build/cppcache/integration/test
$ ctest -R <test_name> -j1
```
For example: `$ ctest -R AuthInitializeTest.putGetWithBasicAuth -j1`

Notice that `BasicIPv6Test` test is expected to fail due to IPv6 support is disabled by default. [BUILDING.md](BUILDING.md) explains how to enable it.


## Style

### Formatting C++
For C++ it is required to follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and have a build target that uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to achieve compliance.
```bash
$ clang-format -i --style=file <PATH_TO_FILES>
```

### Code
When writing new or refactoring old code please make the following changes.

 * Prefer the use of [`auto`](http://en.cppreference.com/w/cpp/language/auto) C++ or [`var`](https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/var) in C# where applicable.
   ```c++
   std::vector<Cars> cars = lot.getCars();
   ```
   should be changed to
   ```c++
   auto cars = lot.getCars();
   ```

 * Prefer [range for](http://en.cppreference.com/w/cpp/language/range-for) loops over traditional for loops where applicable.
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

  * Fix [`std::string::c_str()`](http://en.cppreference.com/w/cpp/string/basic_string/c_str) calls where passed to [`std::string`](http://en.cppreference.com/w/cpp/string/basic_string) parameters.
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

  * Replace [`sprintf`](http://en.cppreference.com/w/cpp/io/c/fprintf) for logging messages with [`std::string`](http://en.cppreference.com/w/cpp/string/basic_string) or [`std::stringstream`](http://en.cppreference.com/w/cpp/io/basic_stringstream).
    ```c++
    char[1024] buffer;
    sprintf(buffer, "Car::crashed: name=%s", car.getName().c_str());
    LOG(buffer);
    ```
    should be changed to
    ```c++
    LOG("Car::crashed: name=" + car.getName());
    ```

  * Replace [`dynamic_cast`](http://en.cppreference.com/w/cpp/language/dynamic_cast) on [`std::shared_ptr`](http://en.cppreference.com/w/cpp/memory/shared_ptr) with [`std::dynamic_pointer_cast`](http://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast). Same goes for [`static_cast`](http://en.cppreference.com/w/cpp/language/static_cast) to [`std::static_pointer_cast`](http://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast), etc.
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
