#! /bin/bash

set -e

if test x"$1" != xyes; then
  apt install -qq -y --no-install-recommends elfutils libelf-dev libdw-dev libdebuginfod-dev
  exit
fi

apt install -qq -y --no-install-recommends libcurl4-openssl-dev wget libzstd-dev libbz2-dev liblzma-dev

version=$(grep elfutils dependencies.versions | awk '{split($0,a,":"); print a[2]}')

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
