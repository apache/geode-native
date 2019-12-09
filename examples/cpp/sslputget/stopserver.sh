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

#!/usr/bin/env bash
GFSH_PATH=""
which gfsh 2> /dev/null

if [ $? -eq 0 ]; then
    GFSH_PATH="gfsh"
else
    if [ "$GEODE_HOME" == "" ]; then
        echo "Could not find gfsh. Please set the GEODE_HOME path."
        echo "e.g. export GEODE_HOME=<path to Geode>"
    else
        GFSH_PATH=$GEODE_HOME/bin/gfsh
    fi
fi

BASEDIR="$( cd "$( dirname "$0" )" && pwd )"

$GFSH_PATH  -e "connect --use-ssl=true --key-store=${BASEDIR}/ServerSslKeys/server_keystore.p12 --trust-store=${BASEDIR}/ServerSslKeys/server_truststore.jks --trust-store-password=apachegeode --key-store-password=apachegeode" -e "shutdown --include-locators=true"

rm -rf locator server
