# classaskey example

Many applications are best served by using compound keys to store and retrieve data. Geode Native fills this need by allowing users to define their own custom class to be used as a key. It is very easy to implement the ICacheableKey interface and design classes that can be used as keys. In addition, by leveraging the Geode Native Objects.Hash() function will ensure client and server hashcodes match. This is important for performance sensitive applications that use single hop.

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

1. Execute `Debug\dotnet-classaskey.exe`. Expect client and server output similar to below. Since the keys are generated using random numbers, your output will differ. However, as shown in the following output, the hashCodes should match between client and server.

    Program output:
    ```console
    Registering for data serialization
    Populating the photosMetaData region

    Inserting 3 photos for key: {Alice} from 4/21/2017 7:00:00 AM to 7/18/2020 7:00:00 AM with hashCode = -401665386
    Inserting 3 photos for key: {Bob, Carol, Ted} from 7/26/2002 7:00:00 AM to 7/4/2005 7:00:00 AM with hashCode = -1032114678
    Inserting 2 photos for key: {Alice, Bob, Carol, Ted} from 4/30/1987 7:00:00 AM to 4/15/2020 7:00:00 AM with hashCode = -647461847
    Inserting 0 photos for key: {Alice, Bob, Carol, Ted} from 8/3/1971 7:00:00 AM to 2/23/2015 8:00:00 AM with hashCode = -358151561
    Inserting 2 photos for key: {Alice, Bob, Carol, Ted} from 7/5/1984 7:00:00 AM to 11/6/1985 8:00:00 AM with hashCode = 452667681
    Inserting 2 photos for key: {Bob} from 11/1/1988 7:00:00 AM to 5/3/1992 7:00:00 AM with hashCode = 651272813
    Inserting 2 photos for key: {Alice, Ted} from 7/25/1982 7:00:00 AM to 3/23/1999 7:00:00 AM with hashCode = 1995204525
    Inserting 5 photos for key: {Alice, Bob, Carol, Ted} from 12/12/1974 8:00:00 AM to 1/10/1990 8:00:00 AM with hashCode = -1945749946
    Inserting 1 photos for key: {Alice, Bob, Carol, Ted} from 12/31/1990 8:00:00 AM to 9/26/1997 7:00:00 AM with hashCode = 1893650760
    Inserting 4 photos for key: {Bob, Carol, Ted} from 5/7/2016 7:00:00 AM to 5/30/2016 7:00:00 AM with hashCode = 664954774

    Fetching photos for key: {Alice} from 4/21/2017 7:00:00 AM to 7/18/2020 7:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
    Fetching photos for key: {Bob, Carol, Ted} from 7/26/2002 7:00:00 AM to 7/4/2005 7:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 4/30/1987 7:00:00 AM to 4/15/2020 7:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 8/3/1971 7:00:00 AM to 2/23/2015 8:00:00 AM
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 7/5/1984 7:00:00 AM to 11/6/1985 8:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
    Fetching photos for key: {Bob} from 11/1/1988 7:00:00 AM to 5/3/1992 7:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
    Fetching photos for key: {Alice, Ted} from 7/25/1982 7:00:00 AM to 3/23/1999 7:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 12/12/1974 8:00:00 AM to 1/10/1990 8:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
       Fetching photo number 3
       Fetching photo number 4
    Fetching photos for key: {Alice, Bob, Carol, Ted} from 12/31/1990 8:00:00 AM to 9/26/1997 7:00:00 AM
       Fetching photo number 0
    Fetching photos for key: {Bob, Carol, Ted} from 5/7/2016 7:00:00 AM to 5/30/2016 7:00:00 AM
       Fetching photo number 0
       Fetching photo number 1
       Fetching photo number 2
       Fetching photo number 3
    [fine 2021/02/06 14:20:08.989562 Pacific Standard Time  FirstPro:548 23060] Cache closed.
    [fine 2021/02/06 14:20:09.000032 Pacific Standard Time  FirstPro:548 23060] Removing cliCallback 1
     ```
     Server Log (for readability filtered for just the hashCode):
     ```console
    [warn 2021/02/06 14:20:08.200 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = -401665386
    [warn 2021/02/06 14:20:08.236 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = -1032114678
    [warn 2021/02/06 14:20:08.272 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = -647461847
    [warn 2021/02/06 14:20:08.285 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = -358151561
    [warn 2021/02/06 14:20:08.330 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = 452667681
    [warn 2021/02/06 14:20:08.365 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = 651272813
    [warn 2021/02/06 14:20:08.404 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = 1995204525
    [warn 2021/02/06 14:20:08.443 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = -1945749946
    [warn 2021/02/06 14:20:08.469 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = 1893650760
    [warn 2021/02/06 14:20:08.509 PST <ServerConnection on port 40404 Thread 1> tid=0x42] hashCode = 664954774
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
