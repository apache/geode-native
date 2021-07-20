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

FROM registry.access.redhat.com/ubi8/ubi:latest
LABEL maintainer Apache Geode <dev@geode.apache.org>

USER root
WORKDIR /

RUN yum update -y && \
    yum -y clean all

COPY bellsoft.repo /etc/yum.repos.d/
COPY --from=centos:8 /etc/yum.repos.d/CentOS-Linux-PowerTools.repo /etc/yum.repos.d/
COPY --from=centos:8 /etc/pki/rpm-gpg/RPM-GPG-KEY-centosofficial /etc/pki/rpm-gpg/

RUN yum config-manager --set-enabled powertools && \
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

# Get latest release of CMake ignoring pre-releases
RUN installer=$(mktemp) \
    && curl -o ${installer} -L $(curl -s https://api.github.com/repos/Kitware/CMake/releases \
        | grep -P -i 'browser_download_url.*cmake-\d+\.\d+\.\d+-linux-x86_64\.sh' \
        | head -n 1 \
        | cut -d : -f 2,3 \
        | tr -d '"') \
    && bash ${installer} --skip-license --prefix=/usr/local \
    && rm ${installer}

ARG GEODE_VERSION=1.13.3
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
RUN curl -L -s "https://www.apache.org/dyn/closer.lua/geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tgz?action=download" | tar -zxvf - --exclude javadoc

CMD ["bash"]
