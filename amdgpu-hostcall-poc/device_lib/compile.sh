#!/bin/bash
hipcc --genco --offload-arch=gfx908 device_only.cpp -o 908.bundle 
clang-offload-bundler --unbundle --input=./908.bundle  --type=o --targets=host-x86_64-unknown-linux-gnu,hip-amdgcn-amd-amdhsa--gfx908 --outputs=/dev/null,extracted_gfx908.hsaco
