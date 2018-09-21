# RemoteQuery Example
This is a simple example showing how to execute a query on a remote region.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Running
* Start Geode Server and create region.
  ```
  gfsh>start locator --name=locator
  gfsh>start server --name=server
  gfsh>create region --name=custom_orders --type=PARTITION
  ```
* Execute `RemoteQueryCs.exe`.
  
  output:
  ```
  Registering for data serialization
  Create orders
  Storing orders in the region
  Getting the orders from the region
  The following orders have a quantity greater than 30:
  Order: [6, product z, 42]
  Order: [4, product z, 102]
  Order: [2, product y, 37]
  ```
