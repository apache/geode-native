#!/usr/bin/env bash

set -xeuo pipefail

YTT=${YTT:-ytt}
FLY=${FLY:-fly}

git_branch=$(git rev-parse --abbrev-ref HEAD)
git_tracking_branch=$(git for-each-ref --format='%(upstream:short)' $(git symbolic-ref -q HEAD))
git_remote=$(echo ${git_tracking_branch} | cut -d/ -f1)
git_remote_url=$(git remote get-url ${git_remote})

#git_remote_url=$(git remote get-url origin)
git_remote_url="http://github.com/pivotal-jbarrett/geode-native.git"
git_owner=$(echo ${git_remote_url} | cut -d/ -f1)

#${YTT} -f pipeline.yml -f templates.lib.yml -f templates.lib.txt -f data.yml > output.yml
#${FLY} -t test set-pipeline -p test -c output.yml
