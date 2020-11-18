# continuousquery example
This is a simple example showing how to execute a continuous query on a Goede region.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `continuousquery` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/continuousquery
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

1. Execute `Debug\dotnet-continuousquery.exe`. Expect the following output:

    ```console
    Registering for data serialization
    Executing continuous query
    Create orders
    Putting and changing Order objects in the region
    MyCqListener::OnEvent(CREATE) called with key Order2, value Order: [2, product y, 37]
    MyCqListener::OnEvent(CREATE) called with key Order4, value Order: [4, product z, 102]
    MyCqListener::OnEvent(CREATE) called with key Order6, value Order: [6, product z, 42]
    MyCqListener::OnEvent(UPDATE) called with key Order2, value Order: [2, product y, 45]
    MyCqListener::OnEvent(DESTROY) called with key Order2, value Order: [2, product y, 29]
    MyCqListener::OnEvent(DESTROY) called with key Order6, value null
    MyCqListener::close called
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
