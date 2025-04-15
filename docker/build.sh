#! /bin/bash

set -e

function usage {
  echo "Usage: $0 src dest [-j] [-c] [-t] [-h] [-v]"
}

function show_help {
  usage
  echo "   src      Source directory"
  echo "  dest      Install directory"
  echo "    -j      Number of CMake build jobs (default: 1)"
  echo "    -c      CMake arguments (must be quoted: -c \"-D1 -D2\")"
  echo "    -t      Run tests"
  echo "    -v      Verbose outputs"
}

src_dir=$1; shift
dest_dir=$1; shift
num_jobs=1
cmake_args=
run_tests=
verbose=

while [[ $# -gt 0 ]]; do
  case "$1" in
    -j) num_jobs="$2"; shift 2;;
    -c) cmake_args="$2"; shift 2;;
    -t) run_tests="Y"; shift;;
    -h) show_help; exit;;
    -v) verbose="--verbose"; shift;;
     *) echo "Unknown arg '$1'"; exit;;
  esac
done

build_dir=$(mktemp -d "/tmp/XXXXXX")
mkdir -p ${dest_dir}

if test "${run_tests}" = "Y"; then
  cmake_args="-DDYNINST_ENABLE_TESTS=ALL ${cmake_args}"
fi

cmake_args+="-DDYNINST_WARNINGS_AS_ERRORS=ON "
cmake_args+="-DDYNINST_ENABLE_FILEFORMAT_PE=ON "

cmake -S ${src_dir} -B ${build_dir} -DCMAKE_INSTALL_PREFIX=${dest_dir} ${cmake_args}

cmake --build ${build_dir} --parallel ${num_jobs} ${verbose}

if test "${run_tests}" = "Y"; then
  export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH
  ctest --test-dir ${build_dir} --parallel 2 --output-on-failure
fi

cmake --install ${build_dir}

rm -rf $build_dir
