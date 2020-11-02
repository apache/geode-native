# putgetremove example
This is a very simple example showing how to create a `Cache` using the `CacheFactory`,
configure a `Pool` with a `PoolFactory`, and configure a `Region` with a `RegionFactory`.
We then put, get, and remove some primitive data in the region.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `putgetremove` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/putgetremove
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

1. Execute `Debug\dotnet-putgetremove.exe`. Expect the following output:

    ```console
    Storing id and username in the region
    Getting the user info from the region
    rtimmons = Robert Timmons
    scharles = Sylvia Charles
    Removing rtimmons info from the region
    Info for rtimmons has been deleted
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
