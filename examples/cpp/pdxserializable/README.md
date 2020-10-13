# pdxserializable example
This is a simple example showing how to register for serialization of custom objects using the PdxSerializable class.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `pdxserializable` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/cpp/pdxserializable
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
  
1. Execute `cpp-pdxserializable`, expect the following output:

    ```
    Create orders
    Storing orders in the region
    Getting the orders from the region
    OrderID: 1
    Product Name: product x
    Quantity: 23
    OrderID: 2
    Product Name: product y
    Quantity: 37
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
