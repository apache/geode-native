# AuthInitialize Example
This example shows how to create and register a custom `IAuthIntialize` authentication
handler on the client that authenticates against a server that was started with the corresponding authenticator. 

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.
* `GEODE_HOME/bin` in the execution path.

## Running
1. Set the current directory to the `AuthInitialize` directory in your example workspace.

  ```
  $ cd workspace/examples/dotnet/AuthInitialize
  ```

2. Run the `startserver.ps1` script to start the Geode cluster with authentication and create a region.

3. Execute `AuthInitialize.exe`:

  ```
.\AuthInitialize.exe
ExampleAuthInitialize::ExampleAuthInitialize called
ExampleAuthInitialize::GetCredentials called
a = 1
b = 2
  ```
