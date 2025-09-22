#!/bin/bash

set -ex

git clone --depth=1 --branch=master https://github.com/trailofbits/pe-parse
cd pe-parse

# https://github.com/trailofbits/pe-parse/issues/187
sed -i 's/-Werror//' cmake/compilation_flags.cmake

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build . --parallel
cmake --install .

cd /
rm -rf pe-parse
