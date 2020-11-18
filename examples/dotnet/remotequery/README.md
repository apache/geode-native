# remotequery example
This is a simple example showing how to execute a query on a remote region.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `remotequery` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/remotequery
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

1. Execute `Debug\dotnet-remotequery.exe`. Expect the following output:

    ```console
    Registering for data serialization
    Create orders
    Storing orders in the region
    Getting the orders from the region
    The following orders have a quantity greater than 30:
    Order: [6, product z, 42]
    Order: [4, product z, 102]
    Order: [2, product y, 37
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
