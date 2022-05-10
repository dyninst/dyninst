#!/bin/bash
set -euo pipefail

printf "⭐️ Setting up spack environment for Dyninst\n"
. /opt/spack/share/spack/setup-env.sh
spack env activate .

# 1. Build Dyninst
printf "⭐️ Preparing to build Dyninst\n"
echo "::group::build dyninst"

DYNINST_BUILD_DIR=/opt/dyninst-env/build/dyninst
mkdir -p $DYNINST_BUILD_DIR

DYNINST_INSTALL_DIR=/opt/dyninst-env/install/dyninst
mkdir -p $DYNINST_INSTALL_DIR

CMAKE_WERROR_FLAGS='-DCMAKE_C_FLAGS="-Werror" -DCMAKE_CXX_FLAGS="-Werror"'
cmake -S /code -B $DYNINST_BUILD_DIR -DCMAKE_INSTALL_PREFIX=$DYNINST_INSTALL_DIR $CMAKE_WERROR_FLAGS
cmake --build $DYNINST_BUILD_DIR --target all --parallel 2 -- VERBOSE=1
cmake --install $DYNINST_BUILD_DIR

echo "::endgroup::"
