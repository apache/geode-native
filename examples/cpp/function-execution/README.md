# Function-execution example
This is a very simple example showing how to execute a function on the server.  
It creates a `Cache` using the `CacheFactory`, configures a `Pool` with a 
`PoolFactory`, and configures a `Region` with a `RegionFactory`.  The 
startserver script deploys a jar file with several custom functions in it, and 
the example app executes those functions and prints out the result set(s).

## Prerequisites
* An installation of Apache Geode.
* Apache Geode Native, built and installed.
* Apache Geode Native examples, built and installed.
* A `GEODE_HOME` environment variable set to the location of the Apache Geode installation.
* `GEODE_HOME/bin` in the execution path.

## Running
1. Set the current directory to the `function-execution` directory in your example workspace.

  ```
  $ cd workspace/examples/cpp/function-execution
  ```

1. Run the `startserver.sh` script to start the Geode server, create a region, and populate the region with sample data.

  ```
  $ sh ./startserver.sh
  /Users/user/geode/bin/gfsh

  (1) Executing - start locator --name=locator
  ...
  (2) Executing - deploy --jar=../../utilities/example.jar
  ...
  (3) Executing - start server --name=the-server --server-port=50505
  ...
  (4) Executing - create region --name=parition_region --type=PARTITION

  Member     | Status
  ---------- | ----------------------------------------------
  the-server | Region "/partition_region" created on "the-server"
  
1. Run the `function-execution` example:

  ```
  $ ./function-execution 
  Result count = 3

       Result[0]=VALUE--1
       Result[1]=VALUE--2
       Result[2]=VALUE--3
  ```

1. Stop the server

  ```
  $ sh ./stopserver.sh
  /Users/user/geode/bin/gfsh
  (1) Executing - connect
  ...
  (2) Executing - shutdown --include-locators=true

  Shutdown is triggered
  ```

