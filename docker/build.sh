#! /bin/bash

set -e

function usage {
  echo "Usage: $0 src dest [-j] [-c] [-t TYPE] [-h] [-v]"
}

function show_help {
  usage
  echo "   src      Source directory"
  echo "  dest      Install directory"
  echo "    -j      Number of CMake build jobs (default: 1)"
  echo "    -c      CMake arguments (must be quoted: -c \"-D1 -D2\")"
  echo "    -t      Run tests (TYPE must be one of ALL, REGRESSION, INTEGRATION, or UNIT)"
  echo "    -v      Verbose outputs"
}

src_dir=$1; shift
dest_dir=$1; shift
num_jobs=1
cmake_args=
test_type=
verbose=

while [[ $# -gt 0 ]]; do
  case "$1" in
    -j) num_jobs="$2"; shift 2;;
    -c) cmake_args="$2"; shift 2;;
    -t) test_type="$2"; shift 2;;
    -h) show_help; exit;;
    -v) verbose="--verbose"; shift;;
     *) echo "Unknown arg '$1'"; exit;;
  esac
done

build_dir=$(mktemp -d "/tmp/XXXXXX")
mkdir -p ${dest_dir}
echo "Building in ${build_dir}"

if ! test -z "${test_type}"; then
  cmake_args="-DDYNINST_ENABLE_TESTS=${test_type} ${cmake_args}"
fi

cmake_args+="-DDYNINST_WARNINGS_AS_ERRORS=ON "
cmake_args+="-DDYNINST_ENABLE_FILEFORMAT_PE=ON "

cmake -S ${src_dir} -B ${build_dir} -DCMAKE_INSTALL_PREFIX=${dest_dir} ${cmake_args}

cmake --build ${build_dir} --parallel ${num_jobs} ${verbose}

if ! test -z "${test_type}"; then
  export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH
  ctest --test-dir ${build_dir} --parallel 2 --output-on-failure
fi

cmake --install ${build_dir}

rm -rf $build_dir
