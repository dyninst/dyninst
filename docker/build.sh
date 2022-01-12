#!/bin/bash
set -euo pipefail

# This script builds dyninst and the test suite (but does not run tests)

printf "⭐️ Setting up spack environment for Dyninst\n"
. /opt/spack/share/spack/setup-env.sh
spack env activate .
mkdir -p build/dyninst

# 1. Build Dyninst
printf "⭐️ Preparing to build Dyninst\n"
echo "::group::build dyninst"   
cd build/dyninst
cmake -H/code -B. -DCMAKE_INSTALL_PREFIX=. > >(tee config.out) 2> >(tee config.err >&2)
make VERBOSE=1 -j2 > >(tee build.out) 2> >(tee build.err >&2)
make install VERBOSE=1 -j2 > >(tee build-install.out) 2> >(tee build-install.err >&2)
echo "::endgroup::"

# 2. Update the test suite
printf "⭐️ Updating the testsuite\n"
echo "::group::update testsuite"   
cd /opt/testsuite
git pull origin master
cd -
echo "::endgroup::"

# 3. Build the test suite
printf "⭐️ Preparing to build the testsuite\n"
echo "::group::build tests"   
cd /opt/dyninst-env/
mkdir -p build/testsuite/tests
cd build/testsuite

cmake -H/opt/testsuite -B. -DCMAKE_INSTALL_PREFIX=$PWD/tests -DDyninst_DIR=/opt/dyninst-env/build/dyninst/lib/cmake/Dyninst > >(tee config.out) 2> >(tee config.err >&2)
make VERBOSE=1 -j2 > >(tee build.out) 2> >(tee build.err >&2)
make install VERBOSE=1 -j2 > >(tee build-install.out) 2> >(tee build-install.err >&2)
echo "::endgroup::"
