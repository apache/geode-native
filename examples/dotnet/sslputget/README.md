# sslputget example
This example illustrates how to use SSL encryption for all traffic between a .NET application and Apache Geode.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Install [OpenSSL](https://www.openssl.org/)

## Running
1. From a command shell, set the current directory to the `sslputget` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/sslputget
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

1. Execute `Debug\dotnet-sslputget.exe`. Expect the following output:

    ```console
    Storing id and username in the region
    Getting the user info from the region
    rtimmons = Robert Timmons
    scharles = Sylvia Charles 
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
