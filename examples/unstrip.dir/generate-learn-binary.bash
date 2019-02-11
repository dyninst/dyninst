#!/bin/bash

# This script generates a binary with the entirety of the provided static library
# linked in; this binary can then be used by unstrip to learn new semantic descriptors

LIB=$1

if [[ ! -n $LIB ]]; then
    echo "Usage: $0 <library>"
    exit 1
fi

LIB_NAME=`basename $LIB`

CC=gcc
LD=ld

OUTPUT=$LIB_NAME.bin

# LINK WHOLE ARCHIVE INTO THE BINARY
$CC -c -o foo.o foo.c
$LD foo.o --whole-archive $LIB -o $OUTPUT #2&> /dev/null

if [[ ! -d learning-binaries ]]; then mkdir learning-binaries; fi

mv $OUTPUT learning-binaries/

rm -f foo.o
