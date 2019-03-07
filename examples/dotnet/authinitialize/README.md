# AuthInitialize Example
This example shows how to create and register a custom `IAuthIntialize` authentication
handler on the client that authenticates against a server that was started with the corresponding authenticator. 

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `AuthInitialize` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/AuthInitialize
    ```

2. Run the `startserver.ps1` script to start the Geode cluster with authentication and create a region.

   For Windows cmd:

    ```console
    $ powershell.exe -File startserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ startserver.ps1
    ```

3. Execute `AuthInitialize.exe`, expect the following output:

       ExampleAuthInitialize::ExampleAuthInitialize called
       ExampleAuthInitialize::GetCredentials called
       a = 1
       b = 2

4. Run the `stopserver.ps1` script to gracefully shutdown the Geode cluster.

   For Windows cmd:

    ```console
    $ powershell.exe -File stopserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ stopserver.ps1
    ```