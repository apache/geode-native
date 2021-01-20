# classaskey example

Many applications are best served by using compound keys to store and retrieve data. Geode Native fills this need by allowing users to define their own custom class to be used as a key. By leveraging the Geode Native Cacheable data types, it is very easy to implement the ICacheableKey interface and design classes that can be used as keys.

This example shows how to design a photo filter as the key for storing metadata for a photo library. The photo filter class (called PhotoKeys in the code) provides for storing and retrieving all photos containing a group of people and that were taken during a date range. The photo metadata class (called PhotoValues in the code) contains the full resolution photoId and thumbnail image for the photo key. In this example, the photoId is an integer representing an index into a photo library. The thumbnail is a small two diminsional array of pixels.

## Prerequisites

* Install [Apache Geode](https://geode.apache.org)
* Build and install [Apache Geode Native](https://github.com/apache/geode-native)
* Apache Geode Native examples, built and installed
* Set `GEODE_HOME` to the install directory of Apache Geode

## Running

1. From a command shell, set the current directory to the `classaskey` directory in your example workspace.

    ```console
    $ cd workspace/examples/build/dotnet/classaskey
    ```

1. Run the `startserver.ps1` script to start the Geode cluster with the example.jar file and create a region.

   For Windows cmd:

    ```console
    $ powershell.exe -File startserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ startserver.ps1
    ```

1. Execute `Debug\dotnet-classaskey.exe`. Expect output similar to below. Since the keys are generated using random numbers, your output will differ.

    ```console
    Registering for data serialization
    Populating the photos region

    Inserting 5 photos for key: {Alice, Bob, Carol, Ted} from 7/3/1999 12:00:00 AM to 3/17/2011 12:00:00 AM
    Inserting 2 photos for key: {Alice, Bob, Ted} from 9/22/1980 12:00:00 AM to 12/5/1980 12:00:00 AM
    Inserting 5 photos for key: {Alice, Carol} from 3/6/1990 12:00:00 AM to 9/26/1993 12:00:00 AM
    Inserting 1 photos for key: {Alice, Bob} from 1/29/1999 12:00:00 AM to 2/1/2002 12:00:00 AM
    Inserting 0 photos for key: {Alice, Bob, Carol, Ted} from 6/5/1989 12:00:00 AM to 7/7/1989 12:00:00 AM
    Inserting 1 photos for key: {Alice, Bob} from 4/8/1979 12:00:00 AM to 5/26/2011 12:00:00 AM
    Inserting 0 photos for key: {Alice, Carol, Ted} from 5/25/1977 12:00:00 AM to 5/15/1997 12:00:00 AM
    Inserting 0 photos for key: {Bob} from 2/20/1981 12:00:00 AM to 3/11/2020 12:00:00 AM
    Inserting 0 photos for key: {Alice, Bob, Carol, Ted} from 7/9/1982 12:00:00 AM to 4/20/1998 12:00:00 AM
    Inserting 3 photos for key: {Alice, Bob, Carol, Ted} from 11/24/2007 12:00:00 AM to 4/30/2012 12:00:00 AM

    Fetching photos for key: {Alice, Bob, Carol, Ted} from 7/3/1999 12:00:00 AM to 3/17/2011 12:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
       Fetching photo number 3
       Fetching photo number 4
    Fetching photos for key: {Alice, Bob, Ted} from 9/22/1980 12:00:00 AM to 12/5/1980 12:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
    Fetching photos for key: {Alice, Carol} from 3/6/1990 12:00:00 AM to 9/26/1993 12:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
       Fetching photo number 3
       Fetching photo number 4
    Fetching photos for key: {Alice, Bob} from 1/29/1999 12:00:00 AM to 2/1/2002 12:00:00 AM
       Fetching photo number 0
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 6/5/1989 12:00:00 AM to 7/7/1989 12:00:00 AM
    Fetching photos for key: {Alice, Bob} from 4/8/1979 12:00:00 AM to 5/26/2011 12:00:00 AM
       Fetching photo number 0
    Fetching photos for key: {Alice, Carol, Ted} from 5/25/1977 12:00:00 AM to 5/15/1997 12:00:00 AM
    Fetching photos for key: {Bob} from 2/20/1981 12:00:00 AM to 3/11/2020 12:00:00 AM
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 7/9/1982 12:00:00 AM to 4/20/1998 12:00:00 AM
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 11/24/2007 12:00:00 AM to 4/30/2012 12:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
    [fine 2021/01/16 23:55:35.040714 Pacific Standard Time FirstPro:26196 20716] Cache closed.
    ```
    
1. Run the `stopserver.ps1` script to gracefully shutdown the Geode cluster.

   For Windows cmd:

    ```console
    $ powershell.exe -File stopserver.ps1
    ```

   For Windows Powershell:

    ```console
    $ stopserver.ps1
    ```
