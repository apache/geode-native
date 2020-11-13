# Transaction example
This is a very simple example showing how to use TransactionManager.  This example shows
how to begin a transaction, commit a transaction, and rollback a transaction while showing
exception handling.  We commit two keys and rollback adding a third key and destroying an
existing key while showing how to handle exceptions.

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built and installed.

## Running
1. From a command shell, set the current directory to the `transaction` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/transaction
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

1. Execute `Debug\dotnet-transaction.exe`. The output logs the cache and region creation, and the results of up to five attempts to commit the transaction.
Example execution ends when the transaction is successfully committed, or when the maximum number of attempts is reached without a successful commit.

    ```console
    Created cache
    Created region 'exampleRegion'
    Rolled back transaction - retrying(4)
    Rolled back transaction - retrying(3)
    Rolled back transaction - retrying(2)
    Committed transaction - exiting
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
