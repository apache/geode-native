# DataSerializable example
This is a simple example showing how to register for serialization of custom objects using the DataSerializable class.

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built and installed.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.
* `GEODE_HOME/bin` in the execution path.

## Running
1. Set the current directory to the `dataserializable` directory in the installed directory tree.

  ```
  $ cd <install-dir>/examples/cpp/dataserializable
  ```

1. Run the `startserver.sh` script to start the Geode server, create a region, and populate the region with sample data.

  ```
  $ sh ./startserver.sh
  /Users/user/geode/bin/gfsh

  (1) Executing - start locator --name=locator
  ...
  (2) Executing - start server --name=server
  ...
  (3) Executing - create region --name=custom_orders --type=PARTITION

  Member | Status
  ------ | -------------------------------------------
  server | Region "/custom_orders" created on "server"
  ```

1. Execute `dataserializable`:

  ```
  $ build/dataserializable
  Create orders
  Storing orders in the region
  Getting the orders from the region
  OrderID: 1
  Product Name: product x
  Quantity: 23
  OrderID: 2
  Product Name: product y
  Quantity: 37
  ```

1. Stop the server

  ```
  $ sh ./stopserver.sh
  /Users/user/geode/bin/gfsh
  (1) Executing - connect
  ...
  (2) Executing - destroy region --name=custom_orders

  Member | Status
  ------ | ----------------------------------------------
  server | Region '/custom_orders' destroyed successfully
  ...
  (2) Executing - stop server --name=server
  ...
  (3) Executing - stop locator --name=locator
  ....
  ```
