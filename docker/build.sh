#! /bin/bash

set -e

if test "x$1" = "x"; then
  echo "No source directory given\nUsage: $0 src [num_jobs]" >stderr
  exit 1
fi

num_jobs=1
if test "x$2" != "x"; then
  num_jobs=$2
fi

printf "⭐️ Preparing to build Dyninst\n"
echo "::group::build dyninst"

SRC_DIR=$1
BUILD_DIR=/dyninst/build
INSTALL_DIR=/dyninst/install
mkdir -p $BUILD_DIR $INSTALL_DIR

printf "⭐️ Configuring\n"
cd $BUILD_DIR
FLAGS="-DCMAKE_C_FLAGS=${DYNINST_C_FLAGS} -DCMAKE_CXX_FLAGS=${DYNINST_CXX_FLAGS}"
if test x"${DYNINST_C_COMPILER}" = x; then DYNINST_C_COMPILER=gcc; fi
if test x"${DYNINST_CXX_COMPILER}" = x; then DYNINST_CXX_COMPILER=g++; fi
COMPILERS="-DCMAKE_C_COMPILER=${DYNINST_C_COMPILER} -DCMAKE_CXX_COMPILER=${DYNINST_CXX_COMPILER}"
cmake $SRC_DIR -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR ${FLAGS} ${COMPILERS} ${EXTRA_CMAKE_FLAGS}

printf "⭐️ Building\n"
cmake --build . --parallel $num_jobs

printf "⭐️ Installing\n"
cmake --install .

printf "⭐️ Cleaning up\n"
cd /
rm -rf $BUILD_DIR

echo "::endgroup::"
