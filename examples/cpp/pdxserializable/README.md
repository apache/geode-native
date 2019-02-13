# PdxSerializable example
This is a simple example showing how to register for serialization of custom objects using the PdxSerializable class.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running
1. From a command shell, set the current directory to the `pdxserializable` directory in your example workspace.

       $ cd workspace/examples/cpp/pdxserializable

2. Run the `startserver` script to start the Geode cluster with authentication and create a region.

   For Windows cmd:

       $ powershell.exe -File startserver.ps1

   For Windows Powershell:

       $ startserver.ps1

   For Bash:

       $ ./startserver.sh

3. Execute `pdxserializable`, expect the following output:

       Create orders
       Storing orders in the region
       Getting the orders from the region
       OrderID: 1
       Product Name: product x
       Quantity: 23
       OrderID: 2
       Product Name: product y
       Quantity: 37

4. Run the `stopserver` script to gracefully shutdown the Geode cluster.

   For Windows cmd:

       $ powershell.exe -File stopserver.ps1

   For Windows Powershell:

       $ stopserver.ps1

   For Bash:

       $ ./stopserver.sh