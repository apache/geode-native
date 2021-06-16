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

if [ "$BUILD_HOME" == "" ]; then
    SAVED="`pwd`"
    cd "`dirname \"$PRG\"`/../.." >&-
    BUILD_HOME="`pwd -P`"
    cd "$SAVED" >&-
fi


# These security items typically go into gfsecurity.properties file.    For 
# simplicity we are including the security details on the commandline.

AUTH_OPTS="--J=-Dgemfire.security-username=server"
AUTH_OPTS="${AUTH_OPTS} --J=-Dgemfire.security-password=server"
AUTH_OPTS="${AUTH_OPTS} --classpath=${BUILD_HOME}/utilities/example.jar"

AUTH_LOCATOR_OPTS="${AUTH_OPTS} --J=-Dgemfire.security-manager=javaobject.SimpleSecurityManager" 


$GFSH_PATH  -e "start locator --name=locator --port=10334 --http-service-port=6060 --J=-Dgemfire.jmx-manager-port=1099" -e "start server --name=server --server-port=0"  -e "create region --name=exampleRegion --type=PARTITION"

$GFSH_PATH  -e "start locator --name=auth_locator ${AUTH_LOCATOR_OPTS} --port=10335 --http-service-port=7070 --J=-Dgemfire.jmx-manager-port=2099" -e "connect --locator=localhost[10335] --user=server --password=server" -e "start server --name=auth_server ${AUTH_OPTS} --server-port=0"  -e "create region --name=authExampleRegion --type=PARTITION"
