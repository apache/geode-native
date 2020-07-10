#!/usr/bin/env bash

set -xeuo pipefail

YTT=${YTT:-ytt}
FLY=${FLY:-fly}

${YTT} -f pipeline.yml -f templates.lib.yml -f templates.lib.txt -f data.yml > output.yml
${FLY} -t test set-pipeline -p test -c output.yml
