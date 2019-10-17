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

set -u

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
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

. $SCRIPT_DIR/build-image-common.sh

DOCS_DIR=${SCRIPT_DIR}/../
BOOK_DIR=${DOCS_DIR}/geode-native-book-${LANG}
LOG_FILE=${SCRIPT_DIR}/build-${LANG}-docs-output.txt

echo "Running Bookbinder inside Docker container to generate documentation..."
echo "  Complete log can be found in ${LOG_FILE}"

docker run -i -t \
  --rm=true \
  -w "${BOOK_DIR}" \
  -v "$PWD:${DOCS_DIR}" \
  ${IMAGE_NAME}-${USER_NAME} \
  /bin/bash -c "bundle exec bookbinder bind local &> ${LOG_FILE}"

SUCCESS=$(grep "Bookbinder bound your book into" ${LOG_FILE})

if [[ "${SUCCESS}" == "" ]];then
  echo "Something went wrong while generating documentation, check log."
else
  echo ${SUCCESS}
fi

docker run -i -t \
  --rm=true \
  -w "${BOOK_DIR}" \
  -v "$PWD:${DOCS_DIR}" \
  ${IMAGE_NAME}-${USER_NAME} \
  /bin/bash -c "chown -R ${USER_ID}:${GROUP_ID} ${LOG_FILE} ${BOOK_DIR}/output ${BOOK_DIR}/final_app"


popd 1> /dev/null

