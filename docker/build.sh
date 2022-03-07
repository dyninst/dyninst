#!/bin/bash
set -euo pipefail

# This script builds dyninst and the test suite (but does not run tests)

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

cmake -S /code -B $DYNINST_BUILD_DIR -DCMAKE_INSTALL_PREFIX=$DYNINST_INSTALL_DIR
cmake --build $DYNINST_BUILD_DIR -- -j2
cmake --install $DYNINST_BUILD_DIR

echo "::endgroup::"

# 2. Update the test suite
printf "⭐️ Updating the testsuite\n"
echo "::group::update testsuite"   
git -C /opt/testsuite pull origin master
echo "::endgroup::"

# 3. Build the test suite
printf "⭐️ Preparing to build the testsuite\n"
echo "::group::build tests"

TESTSUITE_BUILD_DIR=/opt/dyninst-env/build/testsuite
mkdir -p $TESTSUITE_BUILD_DIR

TESTSUITE_INSTALL_DIR=/opt/dyninst-env/install/testsuite
mkdir -p $TESTSUITE_INSTALL_DIR

cmake -S /opt/testsuite -B $TESTSUITE_BUILD_DIR -DCMAKE_INSTALL_PREFIX=$TESTSUITE_INSTALL_DIR -DDyninst_DIR=$DYNINST_INSTALL_DIR/lib/cmake/Dyninst
cmake --build $TESTSUITE_BUILD_DIR -- -j2
cmake --install $TESTSUITE_BUILD_DIR --prefix $TESTSUITE_INSTALL_DIR
mv $TESTSUITE_INSTALL_DIR/bin/testsuite/* $TESTSUITE_INSTALL_DIR
echo "::endgroup::"
