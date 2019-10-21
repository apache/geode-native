# Apache Geode Native Client User Guides

This document contains instructions for building and viewing the Apache Geode Native Client User Guides.

<a name="about"></a>
## About

The Geode-Native repository provides the full source for the Apache Geode Native Client User Guides in markdown format (see _geode-project-dir_/geode-docs/CONTRIBUTE.md for more information on how to use markdown in this context). Users can build the markdown into an HTML user guide using [Bookbinder](https://github.com/pivotal-cf/bookbinder) and the instructions below.

The User Guide can be produced in two versions: one for the .NET native client, the other for the C++ native client.

## Automatic build

Documentation can be built and previewed using the utility scripts at [docs/docker](https://github.com/apache/geode-native/tree/develop/docs/docker). These scripts use Docker, removing the requirement of installing Ruby and Bookbinder. They are based on the instructions described in [docs/manual-build](https://github.com/apache/geode-native/tree/develop/manual-build).

## Manual build

Documentation can be built in a less-automated way, as described at [docs/manual-build](https://github.com/apache/geode-native/tree/develop/docs/manual-build).
