# remotequery example
This is a simple example showing how to create and execute a remote query.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `remotequery` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/cpp/remotequery
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
  
1. Execute `cpp-remotequery`, expect the following output (quantities and order of presentation may differ):

    ```
    Create orders
    Storing orders in the region
    Getting the orders from the region
    The following orders have a quantity greater than 30:
    OrderID: 2 Product Name: product y Quantity: 37
    OrderID: 4 Product Name: product z Quantity: 102
    OrderID: 6 Product Name: product z Quantity: 4
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
