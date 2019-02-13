# RemoteQuery Example
This is a simple example showing how to execute a query on a remote region.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `RemoteQueryCs` directory in your example workspace.

       $ cd workspace/examples/dotnet/RemoteQueryCs

2. Run the `startserver.ps1` script to start the Geode cluster with authentication and create a region.

   For Windows cmd:

       $ powershell.exe -File startserver.ps1

   For Windows Powershell:

       $ startserver.ps1

3. Execute `RemoteQueryCs.exe`, expect the following output:

       Registering for data serialization
       Create orders
       Storing orders in the region
       Getting the orders from the region
       The following orders have a quantity greater than 30:
       Order: [6, product z, 42]
       Order: [4, product z, 102]
       Order: [2, product y, 37]

4. Run the `stopserver.ps1` script to gracefully shutdown the Geode cluster.

   For Windows cmd:

       $ powershell.exe -File stopserver.ps1

   For Windows Powershell:

       $ stopserver.ps1