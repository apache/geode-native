To run the tests from Visual Studio, the following two environment variables must be set:
* GEODE_HOME: This should point to the Geode install directory, apache-geode-1.15.0, for example.
* GEODE_NATIVE_BUILD_DIR: This should point to the build directory for geode-native, c:/geode-native/build, for example.

The project runs the checkEnvironment.ps1 script as a pre-build step and will return an error if either of these are not set.