#!/bin/bash
set -euo pipefail

printf "⭐️ Setting up spack environment for Dyninst\n"
. /opt/spack/share/spack/setup-env.sh
spack env activate .

# 3. Run the tests
printf "⭐️ Running tests...\n"
cd /opt/dyninst-env/build/testsuite
export DYNINSTAPI_RT_LIB=/opt/dyninst-env/build/dyninst/lib/libdyninstAPI_RT.so
export OMP_NUM_THREADS=2
export LD_LIBRARY_PATH=/opt/dyninst-env/build/dyninst/lib:$PWD:$LD_LIBRARY_PATH
./runTests -64 -all -log test.log -j1 > >(tee stdout.log) 2> >(tee stderr.log >&2)

# TODO will update here to upload given merge to master
# Run the build script to collect and process the logs then upload them
# cd /opt/dyninst-env                                                                                   && \
# perl /opt/testsuite/scripts/build/build.pl --hostname=ci-github --quiet --restart=build --no-run-tests --upload --auth-token=xxxxxxxxxxxxxxxxxxx
