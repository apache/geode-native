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
LABEL maintainer Apache Geode <dev@geode.apache.org>

RUN \
    apt-get update \
    && apt-get install -y build-essential cmake doxygen git graphviz openjdk-8-jdk wget zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

ENV GEODE_VERSION 1.1.0
RUN \
    wget "https://www.apache.org/dyn/closer.cgi?action=download&filename=geode/${GEODE_VERSION}/apache-geode-${GEODE_VERSION}.tar.gz" -O - | tar xzf -

ENV RAT_VERSION 0.12
RUN \
    wget "https://www.apache.org/dyn/closer.cgi?action=download&filename=creadur/apache-rat-${RAT_VERSION}/apache-rat-${RAT_VERSION}-bin.tar.gz2" -O - | tar xzf -

ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64
ENV GEODE_HOME /apache-geode-${GEODE_VERSION}
ENV RAT_HOME /apache-rat-${RAT_VERSION}
ENV PATH $PATH:$GEODE_HOME/bin

CMD ["bash"]
