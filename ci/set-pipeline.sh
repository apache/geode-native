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

set -xeuo pipefail

function printHelp() {
  cat << EOF
$0 Usage:
Sets Concourse pipeline for Geode Native builds.

Options:
Environment Variable  Parameter     Description                 Default
target                --target      Fly target.                 "default"
branch                --branch      Branch to build.            Current checked out branch.
repository            --repository  Remote URL for repository.  Current tracking branch repository.
pipeline              --pipeline    Name of pipeline to set.    Based on repository owner name and branch.
fly                   --fly         Path to fly executable.     "fly"
ytt                   --ytt         Path to ytt executable.     "ytt"
output                --output      Rendered pipeline file.     Temporary file.
EOF
}

while [ $# -gt 0 ]; do
  if [[ $1 == "--help" ]]; then
    printHelp;
    exit 0;
  elif [[ $1 == *"--"*"="* ]]; then
    param="${1%%=*}"
    param="${param#--}"
    declare ${param//[^[:word:]]/_}="${1#--*=}"
  elif [[ $1 == *"--"* ]]; then
    param="${1/--/}"
    declare ${param//[^[:word:]]/_}="${2}"
    shift
  fi
  shift
done

ytt=${ytt:-ytt}
fly=${fly:-fly}

target=${target:-default}
output=${output:-$(mktemp)}

branch=${branch:-$(git rev-parse --abbrev-ref HEAD)}
git_tracking_branch=${git_tracking_branch:-$(git for-each-ref --format='%(upstream:short)' $(git symbolic-ref -q HEAD))}
git_remote=${git_remote:-$(echo ${git_tracking_branch} | cut -d/ -f1)}
repository=${repository:-$(git remote get-url ${git_remote})}

if [[ ${repository} =~ ^((https|git)(:\/\/|@)([^\/:]+)[\/:])([^\/:]+)\/(.+).git$ ]]; then
  git_base=${BASH_REMATCH[1]}
  git_owner=${BASH_REMATCH[5]}
  git_project=${BASH_REMATCH[6]}
fi

pipeline=${pipeline:-${git_owner}-${branch}}
pipeline=${pipeline//[^[:word:]-]/-}

bash -c "${ytt} \$@" ytt -f pipeline.yml -f templates.lib.yml -f templates.lib.txt -f data.yml \
  --data-value pipeline.name=${pipeline} \
  --data-value repository.url=${repository} \
  --data-value repository.branch=${branch} \
  > ${output}

bash -c "${fly} \$@" fly --target=${target} \
  set-pipeline --pipeline=${pipeline} --config=${output}
