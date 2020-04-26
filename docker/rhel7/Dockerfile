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
    yum install -y --setopt=tsflags=nodocs \
        java-1.8.0-openjdk-devel \
        python27-python-pip \
        make \
        doxygen \
        zlib-devel \
        patch \
        openssl-devel \
        git \
        http://mirror.centos.org/centos/7/os/x86_64/Packages/doxygen-1.8.5-3.el7.x86_64.rpm && \
    yum -y clean all --enablerepo='*'

ENV CMAKE_VERSION 3.15.0
RUN curl -o /tmp/cmake -s "https://cmake.org/files/v${CMAKE_VERSION%.*}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh" && \
    bash /tmp/cmake --skip-license --prefix=/usr/local && \
    rm /tmp/cmake

ENV GEODE_VERSION 1.12.0
RUN curl -L -s "https://www.apache.org/dyn/closer.cgi?action=download&filename=geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tgz" | tar xvz
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
ENV PATH $PATH:$GEODE_HOME/bin

CMD ["bash"]
