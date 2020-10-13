# continuousquery example
This is a simple example showing how to create a continuous query.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `continuousquery` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/cpp/continuousquery
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
  
1. Execute `cpp-continuousquery`, expect the following output:

    ```
    Executing continuous query
    Create orders
    Putting and changing Order objects in the region
    MyCqListener::OnEvent called with CREATE, key[Order2], value(2, product y, 37)
    MyCqListener::OnEvent called with CREATE, key[Order4], value(4, product z, 102)
    MyCqListener::OnEvent called with CREATE, key[Order6], value(6, product z, 42)
    MyCqListener::OnEvent called with UPDATE, key[Order2], value(2, product y, 45)
    MyCqListener::OnEvent called with DESTROY, key[Order2], value(2, product y, 29)
    MyCqListener::OnEvent called with DESTROY, key[Order6], value is nullptr
    close called
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
