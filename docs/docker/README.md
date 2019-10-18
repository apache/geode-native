# Automatic generation of Apache Geode Native Client User Guides
This document contains instructions for building and viewing the Apache Geode Native Client User Guides for C++ and .NET languages.


## Building the User Guides
The build-docs.sh script invokes Bookbinder to transform the markdown files to HTML using Docker, which has been provisioned with Bookbinder and Ruby. To build the guide, run the script from a shell prompt:

- For C++ user guide:
```
$ ./build-docs.sh cpp
```

- For .NET user guide:
```
$ ./build-docs.sh dotnet
```

## Viewing the User Guides
After the HTML files are generated, view-docs.sh can be used to start a webserver and review the documentation.

- For C++ user guide:
```
$ ./view-docs.sh cpp
```
In a browser, navigate to `http://localhost:9191` to view the user guide.

- For .NET user guide:
```
$ ./view-docs.sh dotnet
```
In a browser, navigate to `http://localhost:9292` to view the user guide.


The other files in this folder (`build-image-common.sh` and `Dockerfile`) are utilities used by `build-docs.sh` and `view-docs.sh`.
