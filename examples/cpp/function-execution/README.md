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
  (2) Executing - start server --name=server
  ...
(3) Executing - create region --name=example_userinfo --type=PARTITION

  Member | Status
  ------ | ----------------------------------------------
  server | Region "/partition_region" created on "the-server"
  ```
(1) Executing - start locator --name=locator

...
Locator in /nc_install/examples/cpp/function-execution/locator on 10.118.33.178[10334] as locator is currently online.
Process ID: 63773
Uptime: 2 seconds
Geode Version: 1.6.0
Java Version: 1.8.0_162
Log File: /nc_install/examples/cpp/function-execution/locator/locator.log
JVM Arguments: -Dgemfire.enable-cluster-configuration=true -Dgemfire.load-cluster-configuration-from-dir=false -Dgemfire.launcher.registerSignalHandlers=true -Djava.awt.headless=true -Dsun.rmi.dgc.server.gcInterval=9223372036854775806
Class-Path: /apache-geode-1.6.0/lib/geode-core-1.6.0.jar:/apache-geode-1.6.0/lib/geode-dependencies.jar

Successfully connected to: JMX Manager [host=10.118.33.178, port=1099]

Cluster configuration service is up and running.


(2) Executing - deploy --jar=../../javaobject.jar



(3) Executing - start server --name=the-server --server-port=40404

....
Server in /nc_install/examples/cpp/function-execution/the-server on 10.118.33.178[40404] as the-server is currently online.
Process ID: 63778
Uptime: 5 seconds
Geode Version: 1.6.0
Java Version: 1.8.0_162
Log File: /nc_install/examples/cpp/function-execution/the-server/the-server.log
JVM Arguments: -Dgemfire.default.locators=10.118.33.178[10334] -Dgemfire.start-dev-rest-api=false -Dgemfire.use-cluster-configuration=true -XX:OnOutOfMemoryError=kill -KILL %p -Dgemfire.launcher.registerSignalHandlers=true -Djava.awt.headless=true -Dsun.rmi.dgc.server.gcInterval=9223372036854775806
Class-Path: /apache-geode-1.6.0/lib/geode-core-1.6.0.jar:/apache-geode-1.6.0/lib/geode-dependencies.jar


(4) Executing - create region --name=partition_region --type=PARTITION

  Member   | Status
---------- | --------------------------------------------------
the-server | Region "/partition_region" created on "the-server"


(5) Executing - start server --name=the-second-server --server-port=50505

....
Server in /nc_install/examples/cpp/function-execution/the-second-server on 10.118.33.178[50505] as the-second-server is currently online.
Process ID: 63779
Uptime: 4 seconds
Geode Version: 1.6.0
Java Version: 1.8.0_162
Log File: /nc_install/examples/cpp/function-execution/the-second-server/the-second-server.log
JVM Arguments: -Dgemfire.default.locators=10.118.33.178[10334] -Dgemfire.start-dev-rest-api=false -Dgemfire.use-cluster-configuration=true -XX:OnOutOfMemoryError=kill -KILL %p -Dgemfire.launcher.registerSignalHandlers=true -Djava.awt.headless=true -Dsun.rmi.dgc.server.gcInterval=9223372036854775806
Class-Path: /apache-geode-1.6.0/lib/geode-core-1.6.0.jar:/apache-geode-1.6.0/lib/geode-dependencies.jar
1. Execute `function-execution`:

  ```

  $./function-execution 
Created CacheFactory
Created the Region
test data independent function with result on one server
get: result count = 17
get result[0]=VALUE--1
get result[1]=VALUE--3
get result[2]=VALUE--5
get result[3]=VALUE--7
get result[4]=VALUE--9
get result[5]=VALUE--11
get result[6]=VALUE--13
get result[7]=VALUE--15
get result[8]=VALUE--17
get result[9]=VALUE--19
get result[10]=VALUE--21
get result[11]=VALUE--23
get result[12]=VALUE--25
get result[13]=VALUE--27
get result[14]=VALUE--29
get result[15]=VALUE--31
get result[16]=VALUE--33
test data independent function without result on one server
test data independent function with result on all servers
get: result count = 34
get result[0]=KEY--1
get result[1]=KEY--3
get result[2]=KEY--5
get result[3]=KEY--7
get result[4]=KEY--9
get result[5]=KEY--11
get result[6]=KEY--13
get result[7]=KEY--15
get result[8]=VALUE--17
get result[9]=VALUE--19
get result[10]=VALUE--21
get result[11]=VALUE--23
get result[12]=VALUE--25
get result[13]=VALUE--27
get result[14]=VALUE--29
get result[15]=VALUE--31
get result[16]=VALUE--33
get result[17]=KEY--1
get result[18]=KEY--3
get result[19]=KEY--5
get result[20]=KEY--7
get result[21]=KEY--9
get result[22]=KEY--11
get result[23]=KEY--13
get result[24]=KEY--15
get result[25]=KEY--17
get result[26]=KEY--19
get result[27]=KEY--21
get result[28]=KEY--23
get result[29]=KEY--25
get result[30]=KEY--27
get result[31]=KEY--29
get result[32]=KEY--31
get result[33]=KEY--33
test data independent function without result on all servers
test data dependent function with result
Execute on Region: result count = 4
Execute on Region: result count = 34
Execute on Region: result[0]=KEY--11
Execute on Region: result[1]=KEY--5
Execute on Region: result[2]=KEY--17
Execute on Region: result[3]=KEY--27
Execute on Region: result[4]=KEY--9
Execute on Region: result[5]=KEY--29
Execute on Region: result[6]=KEY--13
Execute on Region: result[7]=KEY--23
Execute on Region: result[8]=KEY--15
Execute on Region: result[9]=KEY--11
Execute on Region: result[10]=KEY--5
Execute on Region: result[11]=KEY--17
Execute on Region: result[12]=KEY--27
Execute on Region: result[13]=KEY--9
Execute on Region: result[14]=KEY--29
Execute on Region: result[15]=KEY--13
Execute on Region: result[16]=KEY--23
Execute on Region: result[17]=KEY--15
Execute on Region: result[18]=KEY--31
Execute on Region: result[19]=KEY--1
Execute on Region: result[20]=KEY--33
Execute on Region: result[21]=KEY--21
Execute on Region: result[22]=KEY--3
Execute on Region: result[23]=KEY--7
Execute on Region: result[24]=KEY--19
Execute on Region: result[25]=KEY--25
Execute on Region: result[26]=KEY--31
Execute on Region: result[27]=KEY--1
Execute on Region: result[28]=KEY--33
Execute on Region: result[29]=KEY--21
Execute on Region: result[30]=KEY--3
Execute on Region: result[31]=KEY--7
Execute on Region: result[32]=KEY--19
Execute on Region: result[33]=KEY--25
test data dependent function without result
Closed the Geode Cache  
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

