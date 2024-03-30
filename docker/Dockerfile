ARG base
FROM ${base}

ARG build_jobs=1

LABEL maintainer="@hainest"

COPY . /dyninst/src
COPY docker/build.sh docker/dependencies.versions /

# Build Dyninst
RUN <<EOD bash

  set -ex
  bash build.sh /dyninst/src /dyninst/install -j $build_jobs
  rm -rf /dyninst/src/.git

EOD
