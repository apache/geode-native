#!/usr/bin/env bash

# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -x -e -o pipefail

source ~/.bashrc

NCPU=2

pushd `mktemp -d`
wget -O - https://cmake.org/files/v3.16/cmake-3.16.8.tar.gz | \
    gtar --strip-components=1 -zxf -
./bootstrap --system-curl --no-qt-gui --parallel=$NCPU -- -DBUILD_CursesDialog=off
gmake -j$NCPU
gmake install
popd

p='PATH=$PATH:/usr/local/bin; export PATH'
echo "$p" >> ~/.profile
echo "$p" >> ~/.bashrc
echo "$p" >> /etc/skel/.profile
echo "$p" >> /etc/skel/.bashrc
