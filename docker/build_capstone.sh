#!/bin/bash

set -e

jobs=1   # run builds with N jobs (same as 'make -jN')

while [[ "$#" -gt 0 ]]; do
  case $1 in
    -j|--jobs) jobs="$2"; shift 2; ;;
  esac
done

version=$(grep capstone dependencies.versions | awk '{split($0,a,":"); print a[2]}')

git clone https://github.com/capstone-engine/capstone
cd capstone
git checkout ${version}
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF
cmake --build . --parallel ${jobs}
cmake --install .
cd /
rm -rf capstone
