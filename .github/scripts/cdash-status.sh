#!/bin/bash

set -eo pipefail

readonly API_BASE="https://api.github.com/repos/${GITHUB_REPOSITORY}"

function require_cmd() {
  command -v "${1}" &> /dev/null || { echo "${1} not found"; exit 1; }
}

function require_env_var() {
  if [ -z "${!1}" ]; then
    echo "${1} env variable empty"
    exit 2
  fi
}

#==============================================================================

require_cmd "curl"
require_cmd "jq"
require_env_var "CDASH_BASE_URL"
require_env_var "CDASH_PROJECT"
require_env_var "COMMIT_SHA"
require_env_var "GITHUB_REPOSITORY"
require_env_var "GITHUB_STATUS_NAME"
require_env_var "GITHUB_TOKEN"

#==============================================================================

statuses=$(curl -q -s \
                -H "Content-Type: application/json" \
                -H "Accept: application/vnd.github+json" \
                -H "X-GitHub-Api-Version: 2022-11-28" \
                "${API_BASE}/commits/${COMMIT_SHA}/statuses" |\
                jq -r '[.[].context] | @json')

echo "statuses = $statuses"

post_body="$(cat<<EOF
{
  "state": "success",
  "target_url": "${CDASH_BASE_URL}/index.php?compare1=61&filtercount=1&field1=revision&project=${CDASH_PROJECT}&showfilters=0&limit=100&value1=${COMMIT_SHA}&showfeed=0",
  "description": "Build and test results available on CDash",
  "context": "${GITHUB_STATUS_NAME}"
}
EOF
)"

post_url="${API_BASE}/statuses/${COMMIT_SHA}"

if jq -re "all(. != \"${GITHUB_STATUS_NAME}\")" <<<"${statuses}"; then
  echo "Need to post a status for context ${GITHUB_STATUS_NAME}"

  curl -X POST -q -s \
       -H "Authorization: Bearer ${GITHUB_TOKEN}" \
       -H "Content-Type: application/json" \
       -H "Accept: application/vnd.github+json" \
       -H "X-GitHub-Api-Version: 2022-11-28" \
       -d "${post_body}" "${post_url}"
fi
