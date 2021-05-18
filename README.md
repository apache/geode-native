[![Apache Geode](https://geode.apache.org/img/Apache_Geode_logo.png)](http://geode.apache.org)

[![Build Status](https://concourse.apachegeode-ci.info/api/v1/teams/main/pipelines/geode-native-develop/badge)](https://concourse.apachegeode-ci.info/teams/main/pipelines/geode-native-develop)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)
[![LGTM Total Alerts](https://img.shields.io/lgtm/alerts/g/apache/geode-native.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/apache/geode-native/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/apache/geode-native.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/apache/geode-native/context:cpp)
[![Language grade: C#](https://img.shields.io/lgtm/grade/csharp/g/apache/geode-native.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/apache/geode-native/context:csharp)
[![Language grade: JavaScript](https://img.shields.io/lgtm/grade/javascript/g/apache/geode-native.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/apache/geode-native/context:javascript)

Table of Contents
=================
* [Overview](#overview)
* [Building from Source](#building-from-source)
* [Application Development](#application-development)
* [Versioning](#versioning)
* [Export Control](#export-control)

# Overview

Native Client is a client implementation for [Apache Geode](http://geode.apache.org/) that does not require the Java
server JARs.

# Building from Source

Directions to build Native Client from source can be found in the source distribution in [BUILDING.md](BUILDING.md).

# Application Development

Native Client applications can be written in these client technologies:

* [C++](https://isocpp.org)
* [.NET Framework](https://dotnet.microsoft.com/learn/dotnet/what-is-dotnet-framework)

# Versioning

Geode Native follows the [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html) specification (semver). This
specification only addresses the [API](https://en.wikipedia.org/wiki/API) compatibility with libraries and does not
cover [ABI](https://en.wikipedia.org/wiki/Application_binary_interface) compatibility.

## C++

ABI compatibility is not preserved similarly to the API compatibility. ABI compatibility is only guaranteed between
patch releases. Recompilation is required for both minor and major releases.

## .NET

ABI compatibility is preserved similarly to the API compatibility. ABI compatibility is guaranteed between patch and
minor releases. Recompilation is only require for major releases or to utilize new APIs added in minor releases.

# Export Control

This distribution includes cryptographic software.
The country in which you currently reside may have restrictions
on the import, possession, use, and/or re-export to another country,
of encryption software. BEFORE using any encryption software,
please check your country's laws, regulations and policies
concerning the import, possession, or use, and re-export of
encryption software, to see if this is permitted.
See <http://www.wassenaar.org/> for more information.

The U.S. Government Department of Commerce, Bureau of Industry and Security (BIS),
has classified this software as Export Commodity Control Number (ECCN) 5D002.C.1,
which includes information security software using or performing
cryptographic functions with asymmetric algorithms.
The form and manner of this Apache Software Foundation distribution makes
it eligible for export under the License Exception
ENC Technology Software Unrestricted (TSU) exception
(see the BIS Export Administration Regulations, Section 740.13)
for both object code and source code.

The following provides more details on the included cryptographic software:

* Apache Geode links to and uses [OpenSSL](https://www.openssl.org/) ciphers.
