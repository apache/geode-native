# PutGetRemove Example
This is a very simple example showing how to create a `Cache` using the `CacheFactory`,
configure a `Pool` with a `PoolFactory`, and configure a `Region` with a `RegionFactory`.
We then put, get, and remove some primitive data in the region.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Running
* Start Geode Server and create region.
  ```
  gfsh>start locator --name=locator
  gfsh>start server --name=server
  gfsh>create region --name=example_userinfo --type=PARTITION
  ```
* Execute `Apache.Geode.Examples.PutGetRemove.exe`.
  
  output:
  ```
  Storing id and username in the region
  Getting the user info from the region
  rtimmons = Robert Timmons
  scharles = Sylvia Charles
  Removing rtimmons info from the region
  Info for rtimmons has been deleted
  ```
