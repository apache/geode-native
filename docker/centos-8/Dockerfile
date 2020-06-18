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

FROM centos:8
LABEL maintainer Apache Geode <dev@geode.apache.org>

USER root
WORKDIR /

RUN yum update -y && \
    yum -y clean all

COPY bellsoft.repo /etc/yum.repos.d/

RUN yum install -y 'dnf-command(config-manager)' && \
    yum config-manager --set-enabled PowerTools && \
    yum update -y && \
    yum install -y --setopt=tsflags=nodocs \
        git \
        make \
        zlib-devel \
        patch \
        openssl-devel \
        bellsoft-java11 \
        doxygen \
        python3-pip \
        gcc-c++ && \
    yum -y clean all

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
