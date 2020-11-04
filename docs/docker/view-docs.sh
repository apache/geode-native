#!/bin/bash

# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e -u

if [ "$#" -ne 1 ]; then
  echo "ERROR: Illegal number of parameters"
  echo ""
  echo "Usage: `basename $0` [ cpp | dotnet ]"
  exit 1
fi

LANG=$1
if [ "${LANG}" != "cpp" ] && [ "${LANG}" != "dotnet" ]
then
  echo "ERROR: Incorrect language specified."
  echo ""
  echo "Usage: `basename $0` [ cpp | dotnet ]"
  exit 1
fi

PORT=9292
if [ "${LANG}" = "cpp" ]
then
  PORT=9191
fi

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

. $SCRIPT_DIR/build-image-common.sh

DOCS_DIR=${SCRIPT_DIR}/../
BOOK_DIR=${DOCS_DIR}/geode-native-book-${LANG}

echo "Starting up web server..."
docker run -i -t \
  --rm=true \
  -w "${BOOK_DIR}/final_app/" \
  -v "$PWD:${DOCS_DIR}" \
  -p 127.0.0.1:${PORT}:${PORT} \
  ${IMAGE_NAME}-${USER_NAME} \
  /bin/bash -c "bundle install; bundle exec rackup --host 0.0.0.0 -p ${PORT}"

popd 1> /dev/null

