# Continuous Query Example
This is a simple example showing how to create a continuous query.

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.

## Running
1. Set the current directory to the `continuousquery` directory in your installed example workspace.

  ```
  $ cd workspace/examples/cpp/continuousquery
  ```

1. Run the `startserver.sh` script to start the Geode locator, server, and create a region.

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
  $ ./continuousquery
  Executing continuous query
  Create orders
  Putting and changing Order objects in the region
  MyCqListener::OnEvent called with CREATE, key[Order2], value(2, product y, 37)
  MyCqListener::OnEvent called with CREATE, key[Order4], value(4, product z, 102)
  MyCqListener::OnEvent called with CREATE, key[Order6], value(6, product z, 42)
  MyCqListener::OnEvent called with UPDATE, key[Order2], value(2, product y, 45)
  MyCqListener::OnEvent called with DESTROY, key[Order2], value(2, product y, 29)
  MyCqListener::OnEvent called with DESTROY, key[Order6], value is nullptr
  close called
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
