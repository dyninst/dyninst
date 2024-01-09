#!/bin/bash

set -e

jobs=1   # run builds with N jobs (same as 'make -jN')
push=no  # push images after building

while [[ "$#" -gt 0 ]]; do
  case $1 in
    -j|--jobs) jobs="$2"; shift 2; ;;
    -p|--push) push=yes; shift; ;;
  esac
done

declare -a os_versions

function make_image {
  distro=$1
  version=$2
  extra=$3
  base="ghcr.io/dyninst/amd64/${distro}-${version}-base:latest"
  docker pull ${distro}:${version}   # Always use latest distro image; ignoring any cached versions
  docker build -f docker/Dockerfile.${distro} -t $base --build-arg build_jobs=${jobs} --build-arg version=${version} ${extra} .
  
  if test "${push}" = "yes"; then
    docker push ${base}
  fi
  
  os_versions+=("${distro}-${version}")
}


make_image ubuntu 20.04 "--build-arg build_elfutils=yes"
make_image ubuntu 22.04
make_image ubuntu 23.04
make_image ubuntu 23.10
make_image ubuntu 24.04

make_image fedora 37
make_image fedora 38
make_image fedora 39

echo -n "["
for v in "${os_versions[@]}"; do
  echo -n "'$v', "
done
echo "]"
