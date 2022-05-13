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

FROM ubuntu:16.04
LABEL maintainer Apache Geode <dev@geode.apache.org>

USER root
WORKDIR /

ENV DEBIAN_FRONTEND noninteractive
RUN apt update && \
    apt -yq full-upgrade && \
    apt-get -y install \
        apt-transport-https \
        ca-certificates \
        curl \
        gnupg2 \
        software-properties-common && \
    apt-get -y autoremove && \
    apt-get autoclean

RUN . /etc/os-release && \
    curl -s https://download.bell-sw.com/pki/GPG-KEY-bellsoft | apt-key add - && \
    apt-add-repository "deb http://apt.bell-sw.com/ stable main" && \
    curl -s https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    apt-add-repository "deb http://apt.llvm.org/${VERSION_CODENAME}/ llvm-toolchain-${VERSION_CODENAME}-12 main"

RUN apt update && apt -yq full-upgrade && apt-get -y install \
        build-essential \
        libc++-dev \
        libc++abi-dev \
        zlib1g-dev \
        libssl-dev \
        bellsoft-java11 \
        git \
        doxygen \
        graphviz \
        clang-format-12 && \
    apt-get -y autoremove && \
    apt-get autoclean

# Get latest release of CMake ignoring pre-releases
RUN installer=$(mktemp) \
    && curl -o ${installer} -L $(curl -s https://api.github.com/repos/Kitware/CMake/releases \
        | grep -P -i 'browser_download_url.*cmake-\d+\.\d+\.\d+-linux-x86_64\.sh' \
        | head -n 1 \
        | cut -d : -f 2,3 \
        | tr -d '"') \
    && bash ${installer} --skip-license --prefix=/usr/local \
    && rm ${installer}

ARG GEODE_VERSION=1.14.4
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
RUN curl -L -s "https://www.apache.org/dyn/closer.lua/geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tgz?action=download" | tar -zxvf - --exclude javadoc

CMD ["bash"]
