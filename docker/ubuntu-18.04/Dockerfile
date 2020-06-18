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

FROM ubuntu:18.04
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
    apt-add-repository "deb http://apt.llvm.org/${VERSION_CODENAME}/ llvm-toolchain-${VERSION_CODENAME}-6.0 main"

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
        python3-pip \
        clang-format-6.0 && \
    apt-get -y autoremove && \
    apt-get autoclean

RUN pip3 install --upgrade pip && \
    pip3 install cpp-coveralls

ARG CMAKE_VERSION=3.16.0
RUN curl -o /tmp/cmake -s "https://cmake.org/files/v${CMAKE_VERSION%.*}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh" && \
    bash /tmp/cmake --skip-license --prefix=/usr/local && \
    rm /tmp/cmake

ARG GEODE_VERSION=1.12.0
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
RUN curl -L -s "https://www.apache.org/dyn/closer.cgi?action=download&filename=geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tgz" | tar -zxvf - --exclude javadoc

CMD ["bash"]
