# authinitialize example
This example shows how to create and register a custom `authinitialize` authentication
handler on the client that authenticates against a server that was started with the corresponding authenticator. 

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `authinitialize` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/cpp/authinitialize
    ```

1. Run the `startserver` script to start the Geode cluster with authentication and create a region.

   For Windows cmd:

    ```console
    $ powershell.exe -File startserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ startserver.ps1
    ```

   For Bash:

    ```console
    $ ./startserver.sh
    ```
  
1. Execute `cpp-authinitialize`, expect the following output:

    ```
    ExampleAuthInitialize::ExampleAuthInitialize called
    ExampleAuthInitialize::getCredentials called
    a = 1
    b = 2
    ```
    
1. Run the `stopserver` script to gracefully shutdown the Geode cluster.

   For Windows cmd:

    ```console
    $ powershell.exe -File stopserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ stopserver.ps1
    ```

   For Bash:

    ```console
    $ ./stopserver.sh
    ```
