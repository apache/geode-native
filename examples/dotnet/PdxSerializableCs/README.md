# PdxSerializable Example
This is a simple example showing how to register for serialization of custom objects using the IPDXSerializable class.

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
* Execute `PdxSerializableCs.exe`.
  
  output:
  ```
  Registering for data serialization
  Storing order object in the region
  order to put is Order: [65, Donuts, 12]
  Successfully put order, getting now...
  Order key: 65 = Order: [65, Donuts, 12]
  ```
