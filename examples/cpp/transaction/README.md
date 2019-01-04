# Transaction example
This is a very simple example showing how to use Transaction Manger.  This example shows
how to begin a transaction, commit a transaction, and rollback a transaction while showing
exception handling.  We commit two keys and rollback adding a third key and destroying an
existing key while showing how to handle exceptions.

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built and installed.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.
* `GEODE_HOME/bin` in the execution path.

## Running
1. Set the current directory to the `transaction` directory in your example workspace.

  ```
  $ cd workspace/examples/cpp/transaction
  ```

1. Run the `startserver.sh` script to start the Geode server, create a region, and populate the region with sample data.

  ```
  $ sh ./startserver.sh
  /Users/user/geode/bin/gfsh

  (1) Executing - start locator --name=locator
  ...
  (2) Executing - start server --name=server
  ...
  (3) Executing - create region --name=exampleRegion --type=PARTITION

  Member | Status
  ------ | ----------------------------------------------
  server | Region "/exampleRegion" created on "server"
  ```

1. Execute `transaction`:

  ```
  $ build/transaction
  Created cache
  Created region 'exampleRegion'
  Began transaction #1
  Committed transaction #1
  Obtained the first entry from the Region
  Obtained the second entry from the Region
  Began transaction #2
  Rolled back transaction #2
  Obtained the first entry from the Region
  Obtained the second entry from the Region
  Third entry not found
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
