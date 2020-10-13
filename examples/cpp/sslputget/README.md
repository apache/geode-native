# sslputget example
This example illustrates how to use SSL encryption for all traffic between a client application and Apache Geode.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode
* Install [OpenSSL](https://www.openssl.org/)

## Running
1. From a command shell, set the current directory to the `sslputget` build directory in your example workspace.

    ```console
    $ cd workspace/examples/build/cpp/sslputget
    ```

2. Run the `startserver` script to start the Geode cluster with authentication and create a region.

   For Windows cmd:

    ```console
    $ powershell.exe -File startserver.ps1
    ```

   For Windows Powershell:

    ```console
    PS> startserver.ps1
    ```

   For Bash:

    ```console
    $ ./startserver.sh
    ```
  
3. Execute (for Bash)

     `./cpp-sslputget ./ClientSslKeys/`

   Or, for Windows:

     `<build-type>/cpp-sslputget ./ClientSslKeys (where <build-type> = Debug or Release)`

   Expect the following output:

    ```
    Storing id and username in the region
    Getting the user info from the region
    rtimmons = Robert Timmons
    scharles = Sylvia Charles
    ```

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
