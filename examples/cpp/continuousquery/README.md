# Continuous Query Example
This is a simple example showing how to create and execute a continuous query.

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built and installed.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.
* `GEODE_HOME/bin` in the execution path.

## Running
1. Set the current directory to the `continuousquery` directory in your example workspace.

  ```
  $ cd workspace/examples/cpp/continuousquery
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

1. Execute `continuousquery`:

  ```
  $ build/continuousquery
  Create orders
  Storing orders in the region
  Getting the orders from the region
  The following orders have a quantity greater than 30:
  OrderID: 2 Product Name: product y Quantity: 37
  OrderID: 4 Product Name: product z Quantity: 102
  OrderID: 6 Product Name: product z Quantity: 42
  ```

1. Stop the server

  ```
  $ sh ./stopserver.sh
  /Users/user/geode/bin/gfsh
  (1) Executing - connect
  ...
  (2) Executing - stop server --name=server
  ...
  (3) Executing - stop locator --name=locator
  ....
  ```
