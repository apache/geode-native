# functionexecution example
This is a very simple example showing how to execute a function on the server.  
It creates a `Cache` using the `CacheFactory`, configures a `Pool` with a 
`PoolFactory`, and configures a `Region` with a `RegionFactory`.  The 
startserver script deploys a jar file with several custom functions in it, and 
the example app executes those functions and prints out the result set(s).

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `functionexecution` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/cpp/functionexecution
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
  
1. Execute `cpp-functionexecution`, expect the following output:

    ```
    Result count = 3

    Result[0]=VALUE--1
    Result[1]=VALUE--2
    Result[2]=VALUE--3
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
