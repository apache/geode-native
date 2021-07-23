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

set -e
set -x
# Setup GEODE_HOME, if not set use default
if [ "$GEODE_HOME" == "" ]; then
    export GEODE_HOME=$HOME/src/apache-geode
fi

# Setup BUILD_HOME, if not set use default
if [ "$BUILD_HOME" == "" ]; then
    export BUILD_HOME=$HOME/src/geode-native/netcore/build
fi

echo "Using environment vars GEODE_HOME=${GEODE_HOME} and BUILD_HOME=${BUILD_HOME}"

# Build utility .jar file # BUild NetCore library
pushd ./NetCore
dotnet build
popd

# Build NetCore tests
pushd ./NetCore.Test
dotnet build
popd

# Build utility jar file used for auth test
pushd ./utility
if [ -d "./build" ]; then
    :
else
    mkdir build
fi

cd build
cmake ..
cmake --build .
popd

