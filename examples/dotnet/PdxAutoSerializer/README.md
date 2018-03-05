# PdxAutoSerializer Example
This is a simple example showing how to register for auto-serialization of custom objects using the ReflectionBasedAutoSerializer class.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Running
* Start Geode Server and create region.
  ```
  gfsh>start locator --name=locator
  gfsh>start server --name=server
  gfsh>create region --name=example_orderobject --type=PARTITION
  ```
* Execute `PdxAutoSerializer.exe`.
  
  output:
  ```
  Registering for reflection-based auto serialization
  Storing order object in the region
  order to put is Order: [65, Vox AC30, 11]
  Successfully put order, getting now...
  Order key: 65 = Order: [65, Vox AC30, 11]
  ```
