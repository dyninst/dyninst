#! /bin/bash

version=0.186

wget --no-check-certificate https://sourceware.org/elfutils/ftp/${version}/elfutils-${version}.tar.bz2
bunzip2 elfutils-${version}.tar.bz2
tar -xf elfutils-${version}.tar
cd elfutils-${version}/
mkdir build
cd build
../configure --enable-libdebuginfod --disable-debuginfod
make install
cd /
rm -rf elfutils-${version}/ elfutils-${version}.tar
