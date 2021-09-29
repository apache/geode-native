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
  rm -f Gemfile Gemfile.lock
}

trap cleanup EXIT

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

BOOK_PATH="$(pwd)/../${BOOK_DIR_NAME}"
DOCS_PATH="$(pwd)/../${DOCS_DIR_NAME}"

# Gemfile & Gemfile.lock are copied to avoid including the whole
# geode-book folder to the image context
cp ../${BOOK_DIR_NAME}/Gemfile* .

docker build -t geodenativedocs/temp:1.0 .


# "${BOOK_DIR_NAME}/final_app" and "${BOOK_DIR_NAME}/output" are created
# inside the container, so it is necessary to use the current user to
# avoid these folders are owned by root user.
export UID=$(id -u)
export GID=$(id -g)
docker run -it -p 9292:9292 --user $UID:$GID \
    --workdir="/home/$USER" \
    --volume="/etc/group:/etc/group:ro" \
    --volume="/etc/passwd:/etc/passwd:ro" \
    --volume="/etc/shadow:/etc/shadow:ro" \
    --volume="${BOOK_PATH}:/${BOOK_DIR_NAME}:rw" \
    --volume="${DOCS_PATH}:/${DOCS_DIR_NAME}:rw" \
    geodenativedocs/temp:1.0 /bin/bash -c "cd /${BOOK_DIR_NAME} && bundle exec bookbinder bind local && cd final_app && bundle exec rackup --host=0.0.0.0"


# Bookbinder creates the following links
# <your geode native repo>/docs/geode-native-book-[cpp|dotnet]/output/master_middleman/source/docs/guide/<version> -> /geode-book/output/preprocessing/sections/docs/geode-native/[cpp|dotnet]/<version>
# <your geode native repo>/docs/geode-native-book-[cpp|dotnet]/output/preprocessing/sections/docs/guide/<version> -> /geode-docs
#
# Following lines fix these wrong symbolic links:
#
ug_version=`ls ${BOOK_PATH}/final_app/public/docs/geode-native/${LANG}/`
master_middleman_folder="${BOOK_PATH}/output/master_middleman/source/docs/geode-native/${LANG}/${ug_version}"
preprocessing_folder="${BOOK_PATH}/output/preprocessing/sections/docs/geode-native/${LANG}/${ug_version}"
echo $ug_version
echo ${master_middleman_folder}
echo ${preprocessing_folder}
rm ${master_middleman_folder}
rm ${preprocessing_folder}

ln -s ${DOCS_PATH} ${preprocessing_folder}
ln -s ${preprocessing_folder} ${master_middleman_folder}

