#!/bin/bash

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

set -e

pkg change-facet facet.version-lock.consolidation/sunpro/sunpro-incorporation=false
pkg update sunpro-incorporation || true

pkg set-publisher \
    -k /var/pkg/ssl/pkg.oracle.com.key.pem \
    -c /var/pkg/ssl/pkg.oracle.com.certificate.pem \
    -G '*' -g https://pkg.oracle.com/solarisstudio/release solarisstudio

pkg install --accept -v developerstudio-126/c++  developerstudio-126/dbx

p='PATH=$PATH:/opt/developerstudio12.6/bin; export PATH'
echo "$p" >> ~/.profile
echo "$p" >> ~/.bashrc
echo "$p" >> /etc/skel/.profile
echo "$p" >> /etc/skel/.bashrc
