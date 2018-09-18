# Apache Geode Native Client User Guide

This document contains instructions for building and viewing the Apache Geode Native Client User Guide.

- [About](#about)
- [Prerequisites](#prerequisites)
- [Bookbinder Usage](#bookbinder-usage)
- [Building the Documentation](#building-the-documentation)

## About

The Geode-Native repository provides the full source for the Apache Geode Native Client User Guide in markdown format (see _geode-project-dir_/geode-docs/CONTRIBUTE.md for more information on how to use markdown in this context). Users can build the markdown into an HTML user guide using [Bookbinder](https://github.com/pivotal-cf/bookbinder) and the instructions below.

Bookbinder is a Ruby gem that binds  a unified documentation web application from markdown, html, and/or DITA source material. The source material for bookbinder must be stored either in local directories or in GitHub repositories. Bookbinder runs [Middleman](http://middlemanapp.com/) to produce a Rackup app that can be deployed locally or as a web application.

## Bookbinder Usage

Bookbinder is meant to be used from within a project called a **book**. The book includes a configuration file that describes which documentation repositories to use as source materials. Bookbinder provides a set of scripts to aggregate those repositories and publish them to various locations.

For Geode Native Client, a preconfigured **book** is provided in the directory _geode-native-project-dir_/docs/geode-native-book, which gathers content from the directory _geode-native-project-dir_/docs/geode-native-docs. You can use this configuration to build an HTML version of the Apache Geode Native Client User Guide on your local system.

## Docker Setup

For ease of use, a Docker image is provided that contains a self-consistent Bookbinder environment. [Install Docker](https://docs.docker.com/install/) if you have not already done so.

## Building the Documentation

1. Navigate to the directory that contains the Dockerfile and run the `docker build` command to create the Bookbinder-enabled Docker image:

    ```bash
    $ cd geode-native/docs/docker
    $ docker build -t geode-native .
    ```
    
1. Run the Docker image in interactive mode with a command similar to the following:

    ```bash
    $ docker run -it -p 9292:9292 -p 1234:1234 -v PATH-TO-GEODE-NATIVE:/geode-native geode-native
    ```

    where `PATH-TO-GEODE-NATIVE` is the fully-qualified path to the geode-native repo. This brings up the interactive Docker container, with `/` as your current working directory.

1. To build the documentation, `cd` to the book directory:

    ```bash
    $ cd geode-native/geode-native-book
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

