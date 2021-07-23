#!/bin/bash

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

function cleanup {
  rm Gemfile Gemfile.lock
  rm -r geode-native-book-* geode-native-docs-*
}

trap cleanup EXIT

set -x -e

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

BOOK_DIR_NAME=geode-native-book-${LANG}
DOCS_DIR_NAME=geode-native-docs-${LANG}

mkdir -p ${BOOK_DIR_NAME}
mkdir -p ${DOCS_DIR_NAME}

cp ../${BOOK_DIR_NAME}/Gemfile* .
cp -r ../${BOOK_DIR_NAME} ${BOOK_DIR_NAME}
cp -r ../${DOCS_DIR_NAME} ${DOCS_DIR_NAME}

docker build --build-arg lang=${LANG} -t geodenativedocs/temp:1.0 .

docker run -it -p 9292:9292 geodenativedocs/temp:1.0 /bin/bash -c "cd ${BOOK_DIR_NAME} && bundle exec bookbinder bind local && cd final_app && bundle exec rackup --host=0.0.0.0"
