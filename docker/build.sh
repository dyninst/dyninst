#! /bin/bash

set -ex

function usage {
  echo "Usage: $0 src dest [-j] [-c] [-h] [-v]"
}

function show_help {
  usage
  echo "   src      Source directory"
  echo "  dest      Install directory"
  echo "    -j      Number of CMake build jobs (default: 1)"
  echo "    -c      CMake arguments (must be quoted: -c \"-D1 -D2\")"
  echo "    -v      Verbose outputs"
}

src_dir=$1; shift
dest_dir=$1; shift
num_jobs=1
cmake_args=
verbose=

while [[ $# -gt 0 ]]; do
  case "$1" in
    -j) num_jobs="$2"; shift 2;;
    -c) cmake_args="$2"; shift 2;;
    -h) show_help; exit;;
    -v) verbose="--verbose"; shift;;
     *) echo "Unknown arg '$1'"; ;;
  esac
done

build_dir=$(mktemp -d "/tmp/XXXXXX")
mkdir -p ${dest_dir}

cmake -S ${src_dir} -B ${build_dir} -DCMAKE_INSTALL_PREFIX=${dest_dir} -DDYNINST_WARNINGS_AS_ERRORS=ON ${cmake_args}

cmake --build ${build_dir} --parallel ${num_jobs} ${verbose}

cmake --install ${build_dir}

rm -rf $build_dir
