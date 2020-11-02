# authinitialize example
This example shows how to create and register a custom `IAuthInitialize` authentication
handler on the client that authenticates against a server that was started with the corresponding authenticator. 

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `authinitialize` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/authinitialize
    ```

1. Run the `startserver.ps1` script to start the Geode cluster with authentication and create a region.

   For Windows cmd:

    ```console
    $ powershell.exe -File startserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ startserver.ps1
    ```

1. Execute `Debug\dotnet-authinitialize.exe`. Expect the following output:

    ```console
    ExampleAuthInitialize::ExampleAuthInitialize called
    ExampleAuthInitialize::GetCredentials called
    a = 1
    b = 2
	```

1. Run the `stopserver.ps1` script to gracefully shutdown the Geode cluster.

   For Windows cmd:

    ```console
    $ powershell.exe -File stopserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ stopserver.ps1
    ```
