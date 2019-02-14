# Put-get-remove example
This is a very simple example showing how to create a `Cache` using the `CacheFactory`,
configure a `Pool` with a `PoolFactory`, and configure a `Region` with a `RegionFactory`.
We then put, get, and remove some primitive data in the region.

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built and installed.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.
* `GEODE_HOME/bin` in the execution path.

## Running
1. Set the current directory to the `put-get-remove` directory in your example workspace.

  ```
  $ cd workspace/examples/cpp/put-get-remove
  ```

1. Run the `startserver.sh` script to start the Geode server, create a region, and populate the region with sample data.

  ```
  $ sh ./startserver.sh
  /Users/user/geode/bin/gfsh

  (1) Executing - start locator --name=locator
  ...
  (2) Executing - start server --name=server
  ...
(3) Executing - create region --name=example_userinfo --type=PARTITION

  Member | Status
  ------ | ----------------------------------------------
  server | Region "/example_userinfo" created on "server"
  ```

1. Execute `put-get-remove`:

  ```
  $ ./put-get-remove
  Storing id and username in the region
  Getting the user info from the region
    rtimmons = Robert Timmons
    scharles = Sylvia Charles
  Removing rtimmons info from the region
  rtimmons's info successfully deleted
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
