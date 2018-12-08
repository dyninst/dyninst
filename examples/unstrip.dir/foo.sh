#!/bin/bash

DYNINST_ROOT=/p/paradyn/development/jacobson/Dyninst/githead-static
PLATFORM=x86_64-unknown-linux2.4

g++ -v -v -v -g -Wall \
    -I/u/n/a/nater/devel/parseapi/include \
    -L/u/n/a/nater/devel/parseapi/x86_64-unknown-linux2.4/lib \
    -L/p/paradyn/packages/libelf/lib \
    -L/p/paradyn/packages/libdwarf/lib \
    -o unstrip \
     unstrip.o database.o semanticDescriptor.o types.o util.o fingerprint.o callback.o \
    -Wl,-Bstatic \
    -lparseAPI \
    -lsymtabAPI \
    -linstructionAPI \
    -lcommon \
    -ldwarf \
    -lelf \
    -Wl,-Bdynamic 
