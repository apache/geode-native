# Apache Geode Native Client User Guide

This document contains instructions for building and viewing the Apache Geode Native Client User Guide.

- [About](#about)
- [Prerequisites](#prerequisites)
- [Bookbinder Usage](#bookbinder-usage)
- [Building the Documentation](#building-the-documentation)

## About

The geode-native repository provides the full source for the Apache Geode Native Client User Guide in markdown format (see `{geode-project-dir}/geode-docs/CONTRIBUTE.md` for more information on how to use markdown in this context). Users can build the markdown into an HTML user guide using [Bookbinder](https://github.com/pivotal-cf/bookbinder) and the instructions below.

Bookbinder is a Ruby gem that binds  a unified documentation web application from markdown, html, and/or DITA source material. The source material for bookbinder must be stored either in local directories or in GitHub repositories. Bookbinder runs [Middleman](http://middlemanapp.com/) to produce a Rackup app that can be deployed locally or as a web application.

## Prerequisites

Bookbinder requires Ruby version 2.0.0-p195 or higher.

Follow the instructions below to install Bookbinder:

1. Add gem "bookbindery" to your Gemfile.
2. Run `bundle install` to install the dependencies specified in your Gemfile.

## Bookbinder Usage

Bookbinder is meant to be used from within a project called a **book**. The book includes a configuration file that describes which documentation repositories to use as source materials. Bookbinder provides a set of scripts to aggregate those repositories and publish them to various locations.

For Geode Native Client, a preconfigured **book** is provided in the directory `{geode-native-project-dir}/docs/geode-native-book`, which gathers content from the directory `{geode-native-project-dir}/docs/geode-native-docs`. You can use this configuration to build an HTML version of the Apache Geode Native Client User Guide on your local system.

## Building the Documentation

1. The GemFile in the `geode-native-book` directory already defines the `gem "bookbindery"` dependency. Make sure you are in the `{geode-native-project-dir}/docs/geode-native-book` directory and enter:

    ```
    $ bundle install
    ```

   Note: You will not have to run `bundle install` on subsequent builds.

2. To build the documentation locally using the installed `config.yml` file, enter:

    ```
    $ bundle exec bookbinder bind local
    ```

   Bookbinder converts the markdown source into HTML, which it puts in the `final_app` directory.

3. Navigate to `{geode-native-project-dir}/docs/geode-native-book/final_app/` and enter:

    ```
    $ bundle install
    ```

   Note: You will not have to run `bundle install` on subsequent builds.

4. To start a local website of the Apache Geode Native Client User Guide, enter:

    ```
    $ rackup
    ```

   You can now view the local documentation at <http://localhost:9292>. 

