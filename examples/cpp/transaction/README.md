# transaction example
This is a very simple example showing how to use TransactionManager.  This example shows
how to begin a transaction, commit a transaction, and rollback a transaction while showing
exception handling.  We commit two keys and rollback adding a third key and destroying an
existing key while showing how to handle exceptions.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `transaction` directory in your example workspace.

    ```console
    $ cd workspace/examples/cpp/transaction
    ```

2. Run the `startserver` script to start the Geode cluster with authentication and create a region.

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

3. Execute `cpp-transaction`. The output logs the cache and region creation, and the results of up to five attempts to commit the transaction.
Example execution ends when the transaction is successfully committed, or when the maximum number of attempts is reached without a successful commit.

  For example, this run ends in a successful commit:

    ```bash
    $ ./cpp-transaction
       Created cache
       Created region 'exampleRegion'
       Rolled back transaction - retrying(4)
       Rolled back transaction - retrying(3)
       Rolled back transaction - retrying(2)
       Committed transaction - exiting
    ```
    
  In contrast, this run ends without a successful commit, after five unsuccessful attempts:

    ```bash
    $ ./cpp-transaction
       Created cache
       Created region 'exampleRegion'
       Rolled back transaction - retrying(4)
       Rolled back transaction - retrying(3)
       Rolled back transaction - retrying(2)
       Rolled back transaction - retrying(1)
       Rolled back transaction - retrying(0)       
    ```
    
    You can execute `cpp-transaction` more than once, if you wish to see a variety of results.

4. Run the `stopserver` script to gracefully shutdown the Geode cluster.

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
