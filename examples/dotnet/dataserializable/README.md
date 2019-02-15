# DataSerializableCs Example
This is a simple example showing how to register for serialization of custom objects using the IDataSerializable class.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Running
* Open a PowerShell window and `cd` to the `DataSerializableCs` example directory
* Run `startserver.ps1` to start the Geode Server and create a region.

  ```
  PS C:\> startserver.ps1
  (1) Executing - start locator --name=locator
  ...
  (2) Executing - start server --name=server
  ...
  (3) Executing - create region --name=custom_orders --type=PARTITION

  Member | Status
  ------ | -------------------------------------------
  server | Region "/custom_orders" created on "server"

  ```
* Execute `DataSerializable.exe` to store and retrieve serializable `Order` objects.
  
  ```
  PS C:\> DataSerializableCs.exe
  Create orders
  Storing orders in the region
  Getting the orders from the region
  OrderID: 1
  Product Name: product x
  Quantity: 23
  OrderID: 2 Product Name: product y Quantity: 37
  ```
* Run `stopserver.ps1` to shut down the server.

  ```
  PS C:\> stopserver.ps1
  (1) Executing - connect
  ...
  (2) Executing - destroy region --name=custom_orders

  Member | Status
  ------ | ----------------------------------------------
  server | Region '/custom_orders' destroyed successfully
  
  (3) Executing - stop server --name=server
  ...
  (4) Executing - stop locator --name=locator

  ```