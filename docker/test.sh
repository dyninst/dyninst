#!/bin/bash
set -euo pipefail

printf "⭐️ Setting up spack environment for Dyninst\n"
. /opt/spack/share/spack/setup-env.sh
spack env activate .

# Build the tests
# NB: There are no tests to execute (yet), so building is the actual test
printf "⭐️ Building external-tests\n"
echo "::group::build external-tests"

DYNINST_INSTALL_DIR=/opt/dyninst-env/install/dyninst

TESTS_BUILD_DIR=/opt/dyninst-env/build/external-tests
mkdir -p $TESTS_BUILD_DIR

TESTS_INSTALL_DIR=/opt/dyninst-env/install/external-tests
mkdir -p $TESTS_INSTALL_DIR

cmake -S /opt/external-tests -B $TESTS_BUILD_DIR -DCMAKE_INSTALL_PREFIX=$TESTS_INSTALL_DIR -DDyninst_DIR=$DYNINST_INSTALL_DIR/lib/cmake/Dyninst -DDYNINST_IMPORT_DEPRECATED_TARGETS=ON
cmake --build $TESTS_BUILD_DIR
cmake --install $TESTS_BUILD_DIR

echo "::endgroup::"
