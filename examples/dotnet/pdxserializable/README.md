# pdxserializable example
This is a simple example showing how to register for serialization of custom objects using the IPDXSerializable class.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `pdxserializable` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/pdxserializable
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

1. Execute `Debug\dotnet-pdxserializable.exe`. Expect the following output:

    ```console
    Registering for data serialization
    Storing order object in the region
    order to put is Order: [65, Donuts, 12]
    Successfully put order, getting now...
    Order key: 65 = Order: [65, Donuts, 12]
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
