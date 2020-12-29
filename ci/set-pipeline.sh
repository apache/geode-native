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

set -euo pipefail

function printHelp() {
  cat << EOF
$0 Usage:
Sets Concourse pipelines for Geode Native builds.

Options:
Parameter                Description                         Default
--target                 Fly target.                         "default"
--branch                 Branch to build.                    Current checked out branch.
--repository             Remote URL for repository.          Current tracking branch repository.
--pipeline               Name of pipeline to set.            Based on repository owner name and branch.
--google-zone            Google Compute project.             Current default project.
--google-project         Google Compute zone.                Concourse worker's zone.
--google-storage-bucket  Google Compute Storage bucket.      Based on google-project value.
--google-storage-key     Google Compute Storage key prefix.  Based on pipeline value.
--fly                    Path to fly executable.             "fly"
--ytt                    Path to ytt executable.             "ytt"
--variants               Pipeline variants of publish.       Both release and pr.
--output                 Rendered pipeline files directory.  Temporary directory.

Example:
\$ $0 --target=my-target --google-zone=my-zone

Environment Variables:
All options can be specified via environment variables where hyphens (-) are replaced with underscore (_).

Example:
\$ target=my-target google_zone=my-zone $0

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
output=${output:-$(mktemp -d)}

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

google_project=${google_project:-$(gcloud config get-value project)}
google_zone=${google_zone:-'$(curl "http://metadata.google.internal/computeMetadata/v1/instance/zone" -H "Metadata-Flavor: Google" -s | cut -d / -f 4)'}
google_storage_bucket=${google_storage_bucket:-${google_project}-concourse}
google_storage_key=${google_storage_key:-geode-native/${pipeline}}

variants=${variants:-"release pr"}
variants_release=${variant_release:-""}

for variant in ${variants}; do
  eval pipeline_suffix=\${variants_${variant}-"-${variant}"}

  bash -c "${ytt} \"\$@\"" ytt \
    --file lib \
    --file base \
    --file ${variant} \
    --data-value "pipeline.name=${pipeline}" \
    --data-value "pipeline.variant=${variant}" \
    --data-value "repository.url=${repository}" \
    --data-value "repository.branch=${branch}" \
    --data-value "google.project=${google_project}" \
    --data-value "google.zone=${google_zone}" \
    --data-value "google.storage.bucket=${google_storage_bucket}" \
    --data-value "google.storage.key=${google_storage_key}" \
    > "${output}/${variant}.yml"


  bash -c "${fly} \"\$@\"" fly --target=${target} \
    set-pipeline \
      "--pipeline=${pipeline}${pipeline_suffix}" \
      "--config=${output}/${variant}.yml"

done