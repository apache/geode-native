# Apache Geode Native Client User Guides

This document contains instructions for building and viewing the Apache Geode Native Client User Guides.

## Bookbinder Usage

Bookbinder is meant to be used from within a project called a **book**. The book includes a configuration file that describes which documentation repositories to use as source materials. Bookbinder provides a set of scripts to aggregate those repositories and publish them to various locations.

For Geode Native Client, a preconfigured **book** is provided for each user guide (C++ and .NET) in the directories `_geode-native-project-dir_/docs/geode-native-book-cpp` and `_geode-native-project-dir_/docs/geode-native-book-dotnet`. Each book gathers content from the language-specific directory _geode-native-project-dir_/docs/geode-native-docs-cpp or geode-native-docs-dotnet. You can use this configuration to build an HTML version of the Apache Geode Native Client User Guides on your local system.

<a name="docker-setup"></a>
## Docker Setup

For ease of use, a Docker image is provided that contains a self-consistent Bookbinder environment. [Install Docker](https://docs.docker.com/install/) if you have not already done so.

<a name="building-the-documentation"></a>
## Building the Documentation

1. Navigate to the directory that contains the Dockerfile and run the `docker build` command to create the Bookbinder-enabled Docker image:

    ```bash
    $ cd geode-native/docs/manual-build
    $ docker build -t geode-native-userman .
    ```
    
1. Run the Docker image in interactive mode with a command similar to the following:

    ```bash
    $ docker run -it -p 9292:9292 -p 1234:1234 -v PATH-TO-GEODE-NATIVE/docs:/docs geode-native-userman
    ```

    where `PATH-TO-GEODE-NATIVE` is the fully-qualified path to the geode-native repo. This brings up the interactive Docker container, with `/` as your current working directory.

1. To build the documentation, `cd` to the book directory:

    For C++ guide:
    ```bash
    $ cd docs/geode-native-book-cpp
    ```

    For .NET guide:
    ```bash
    $ cd docs/geode-native-book-dotnet
    ```

1. Run `bundle install` to install the dependencies required to build the user guide.

    ```bash
    $ bundle install
    ```

1. Invoke bookbinder to build the user guide. Bookbinder converts the markdown source into HTML, which it puts in the `final_app` directory:

    ```bash
    $ bundle exec bookbinder bind local
    ```

1. To start a local website of the Apache Geode Native Client User Guide, navigate to the `final_app` directory and run `rackup`:

    ```bash
    $ cd final_app
    $ rackup
    ```

    Note: You may be prompted to run `bundle install` to supply any missing components. Do that, then re-try the `rackup` command.

   You can now view the local documentation in a browser at <http://localhost:9292>. 

