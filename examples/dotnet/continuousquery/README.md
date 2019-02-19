# ContinuousQuery Example
This is a simple example showing how to execute a continuous query on a Goede region.

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
* Execute `ContinuousQueryCS.exe`
  
  output:
  ```
  Registering for data serialization
  Executing continuous query
  Create orders
  Putting and changing Order objects in the region
  MyCqListener::OnEvent(CREATE) called with key Order2, value Order: [2, product y, 37]
  MyCqListener::OnEvent(CREATE) called with key Order4, value Order: [4, product z, 102]
  MyCqListener::OnEvent(CREATE) called with key Order6, value Order: [6, product z, 42]
  MyCqListener::OnEvent(UPDATE) called with key Order2, value Order: [2, product y, 45]
  MyCqListener::OnEvent(DESTROY) called with key Order2, value Order: [2, product y, 29]
  MyCqListener::OnEvent(DESTROY) called with key Order6, value null
  MyCqListener::close called
  ```
