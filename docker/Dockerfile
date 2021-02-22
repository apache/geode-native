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

FROM ubuntu
LABEL maintainer="Apache Geode <dev@geode.apache.org>"

ENV TZ=US/Pacific
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

ARG CLANG_VERSION=11
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
        apt-get install -y \
            libc++-dev \
            libc++abi-dev \
            libssl-dev \
            zlib1g-dev \
            clang-${CLANG_VERSION} \
            clang-tidy-${CLANG_VERSION} \
            clang-format-${CLANG_VERSION} \
            make \
            doxygen \
            git \
            graphviz \
            openjdk-8-jdk \
            curl && \
        rm -rf /var/lib/apt/lists/* && \
        ln -s /usr/lib/llvm-*/include/c++/v1/ /usr/include/c++/v1 && \
        update-alternatives --install /usr/bin/clang         clang         /usr/bin/clang-${CLANG_VERSION} 999 && \
        update-alternatives --install /usr/bin/clang++       clang++       /usr/bin/clang++-${CLANG_VERSION} 999 && \
        update-alternatives --install /usr/bin/cc            cc            /usr/bin/clang-${CLANG_VERSION} 999 && \
        update-alternatives --install /usr/bin/c++           c++           /usr/bin/clang++-${CLANG_VERSION} 999 && \
        update-alternatives --install /usr/bin/clang-tidy    clang-tidy    /usr/bin/clang-tidy-${CLANG_VERSION} 999 && \
        update-alternatives --install /usr/bin/clang-format  clang-format  /usr/bin/clang-format-${CLANG_VERSION} 999
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64

ARG GEODE_VERSION=1.13.1
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
RUN curl -L -s "https://www.apache.org/dyn/closer.cgi?action=download&filename=geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tgz" \
        | tar -zxf - --exclude javadoc

ARG RAT_VERSION=0.13
ENV RAT_HOME /apache-rat-${RAT_VERSION}
RUN curl -L -s "https://archive.apache.org/dist/creadur/apache-rat-${RAT_VERSION}/apache-rat-${RAT_VERSION}-bin.tar.gz" \
        | tar xzf -

# Get latest release of CMake ignoring pre-releases
RUN installer=$(mktemp) \
    && curl -o ${installer} -L $(curl -s https://api.github.com/repos/Kitware/CMake/releases \
        | grep -P -i 'browser_download_url.*cmake-\d+\.\d+\.\d+-linux-x86_64\.sh' \
        | head -n 1 \
        | cut -d : -f 2,3 \
        | tr -d '"') \
    && bash ${installer} --skip-license --prefix=/usr/local \
    && rm ${installer}

ENV PATH $PATH:$GEODE_HOME/bin

CMD ["bash"]
