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

FROM registry.access.redhat.com/rhscl/devtoolset-4-toolchain-rhel7:latest
LABEL maintainer Apache Geode <dev@geode.apache.org>

USER root
WORKDIR /

COPY --from=registry.access.redhat.com/rhscl/s2i-core-rhel7:latest /etc/yum.repos.d/ubi.repo /etc/yum.repos.d/ubi.repo

RUN yum-config-manager --enable ubi-server-rhscl-7-rpms && \
    yum update -y && \
    yum -y clean all

COPY bellsoft.repo /etc/yum.repos.d/

RUN yum-config-manager --enable ubi-server-rhscl-7-rpms && \
    yum update -y && \
    yum install -y --setopt=tsflags=nodocs \
        git \
        make \
        zlib-devel \
        patch \
        openssl-devel \
        rh-python36-python-pip \
        bellsoft-java11 \
        http://mirror.centos.org/centos/7/os/x86_64/Packages/doxygen-1.8.5-4.el7.x86_64.rpm && \
    yum -y clean all

RUN scl enable rh-python36 "pip3 install --upgrade pip && \
    pip3 install cpp-coveralls"

# Get latest release of CMake ignoring pre-releases
RUN installer=$(mktemp) \
    && curl -o ${installer} -L $(curl -s https://api.github.com/repos/Kitware/CMake/releases \
        | grep -P -i 'browser_download_url.*cmake-\d+\.\d+\.\d+-linux-x86_64\.sh' \
        | head -n 1 \
        | cut -d : -f 2,3 \
        | tr -d '"') \
    && bash ${installer} --skip-license --prefix=/usr/local \
    && rm ${installer}

ARG GEODE_VERSION=1.15.0
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
RUN curl -L -s "https://www.apache.org/dyn/closer.lua/geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tgz?action=download" | tar -zxvf - --exclude javadoc

CMD ["bash"]
