#!/bin/bash

set -ex

git clone --depth=1 --branch=master https://github.com/trailofbits/pe-parse
cd pe-parse
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build . --parallel
cmake --install .

cd /
rm -rf pe-parse
