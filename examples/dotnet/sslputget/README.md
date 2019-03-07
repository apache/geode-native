# SSLPutGet Example
This example illustrates how to use SSL encryption for all traffic between a .NET application and Apache Geode.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Install [OpenSSL](https://www.openssl.org/)

## Running
* Run the following Powershell script which starts the Geode Locator, defines the keystore and truststore certificates, starts the Geode Server, and creates the region.
  ```
  PS> startserver.ps1
  ```
* Execute `Apache.Geode.Examples.SSLPutGetCs.exe`.
  
  output:
  ```
  Storing id and username in the region
  Getting the user info from the region
  rtimmons = Robert Timmons
  scharles = Sylvia Charles
  ```
