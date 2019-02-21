# SSLPutGet Example
This example illustrates how to use SSL encryption for all traffic between a client application and Apache Geode.

## Prerequisites
* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Install [OpenSSL]()

## Running
* Run the following shell script which starts the Geode Locator, defines the keystore and truststore certificates, starts the Geode Server, and creates the region.
  ```
  $ ./startserver.sh
  ```
* Execute `sslputget`.

  output:
  ```
  Storing id and username in the region
  Getting the user info from the region
  rtimmons = Robert Timmons
  scharles = Sylvia Charles
  ```