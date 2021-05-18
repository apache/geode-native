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

FROM ubuntu:groovy
LABEL maintainer="Apache Geode <dev@geode.apache.org>"
LABEL description="Minimal image for building with clang toolset."

ARG CLANG_VERSION=11
RUN apt-get update \
    && apt-get install -y \
        bash \
        libssl-dev \
        patch \
        git \
        doxygen \
        openjdk-8-jdk-headless \
        jq \
        curl \
        make \
        clang-${CLANG_VERSION} \
        lld-${CLANG_VERSION} \
        clang-format-${CLANG_VERSION} \
        clang-tidy-${CLANG_VERSION} \
    && update-alternatives --install /usr/bin/clang           clang           /usr/bin/clang-${CLANG_VERSION} 10 \
    && update-alternatives --install /usr/bin/clang++         clang++         /usr/bin/clang++-${CLANG_VERSION} 10 \
    && update-alternatives --install /usr/bin/lld             lld             /usr/bin/lld-${CLANG_VERSION} 10 \
    && update-alternatives --install /usr/bin/clang-tidy      clang-tidy      /usr/bin/clang-tidy-${CLANG_VERSION} 10 \
    && update-alternatives --install /usr/bin/run-clang-tidy  run-clang-tidy  /usr/bin/run-clang-tidy-${CLANG_VERSION} 10 \
    && update-alternatives --install /usr/bin/clang-format    clang-format    /usr/bin/clang-format-${CLANG_VERSION} 10 \
    && update-alternatives --install /usr/bin/cc              cc              /usr/bin/clang 10 \
    && update-alternatives --install /usr/bin/c++             c++             /usr/bin/clang++ 10 \
    && update-alternatives --install /usr/bin/ld              ld              /usr/bin/lld 10 \
    && apt-get -y autoremove \
    && apt-get -y autoclean \
    && rm -rf /var/lib/apt/lists/*

RUN installer=$(mktemp) \
    && curl -o ${installer} -L $(curl -s https://api.github.com/repos/Kitware/CMake/releases/latest | jq -r '.assets[].browser_download_url | select(test("linux-x86_64.sh"))') \
    && bash ${installer} --skip-license --prefix=/usr/local \
    && rm ${installer}
