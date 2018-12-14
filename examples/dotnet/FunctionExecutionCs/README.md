# FunctionExecution Example
This example illustrates how to execute server side java functions.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)

## Running
* Run the following Powershell script which starts the Geode Locator, deploys the jar file containing your function, start the Geode Server, and create region.
  ```
  PS> startserver.ps1
  ```
* Execute `Apache.Geode.Examples.FunctionExecutionCs.exe`.
  
  output:
  ```
  Storing id and username in the region
  Getting the user info from the region
  rtimmons = Robert Timmons
  scharles = Sylvia Charles
  Function Execution Results:
     Count = 1
     value = Robert Timmons
     value = Sylvia Charles
  ```
