#! /bin/bash

set -e

jobs=1     # run builds with N jobs (same as 'make -jN')
source=no  # push images after building

while [[ "$#" -gt 0 ]]; do
  case $1 in
    -j|--jobs) jobs="$2"; shift 2; ;;
    -s|--from-source) source="$2"; shift 2; ;;
  esac
done

if test "$source" = "no"; then
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
make install -j ${jobs}
cd /
rm -rf elfutils-${version}/ elfutils-${version}.tar
