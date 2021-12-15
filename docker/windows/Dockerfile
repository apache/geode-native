# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# escape=`

# .NET 3.5 is required for NUnit 2.6 only.
ARG FROM_IMAGE=mcr.microsoft.com/dotnet/framework/runtime:3.5
FROM ${FROM_IMAGE}

SHELL ["cmd", "/S", "/C"]

WORKDIR C:\

RUN powershell -Command `
    iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'));

RUN choco install `
    git.install `
    liberica8jdk `
    cmake.portable `
    doxygen.install `
    openssl `
    strawberryperl `
    nuget.commandline -confirm

# Only for NUnit 2.6
RUN choco install nunit.install --version 2.6.4 -confirm

ARG GEODE_VERSION=1.14.2
ENV GEODE_HOME C:\apache-geode-${GEODE_VERSION}
RUN curl -L -s "https://www.apache.org/dyn/closer.lua/geode/%GEODE_VERSION%/apache-geode-%GEODE_VERSION%.tgz?action=download" | tar -zxvf - --exclude javadoc

ADD https://raw.githubusercontent.com/microsoft/vs-dockerfiles/master/managed-native-desktop/Install.cmd C:\TEMP\Install.cmd
ADD https://aka.ms/vscollect.exe C:\TEMP\collect.exe

# Download channel for fixed install.
ARG CHANNEL_URL=https://aka.ms/vs/15/release/channel
ADD ${CHANNEL_URL} C:\TEMP\VisualStudio.chman

# Download and install Build Tools for Visual Studio 2017.
ADD https://aka.ms/vs/15/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe
RUN C:\TEMP\Install.cmd C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --channelUri C:\TEMP\VisualStudio.chman `
    --installChannelUri C:\TEMP\VisualStudio.chman `
    --add Microsoft.VisualStudio.Workload.ManagedDesktopBuildTools `
    --add Microsoft.Net.Component.4.5.2.TargetingPack `
    --add Microsoft.VisualStudio.Component.TestTools.BuildTools `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.CLI.Support `
    --add Microsoft.VisualStudio.Component.VC.Tools.14.15 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.16299.Desktop `
    --installPath C:\BuildTools

ENTRYPOINT C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat x86_amd64 10.0.16299.0 -vcvars_ver=14.15 &&
CMD cmd
