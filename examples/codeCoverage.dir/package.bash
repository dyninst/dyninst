#!/bin/bash

function _package
{
    DEST=$1

    make clean
    make --file=Makefile.afs codeCoverage-static libInst.so testcc
    mkdir $DEST
    cp codeCoverage-static $DEST/codeCoverage
    strip $DEST/codeCoverage
    cp libInst.so $DEST
    cp ${DYNINST_ROOT}/${PLATFORM}/lib/libdyninstAPI_RT.so.8.0 ${DEST}/libdyninstAPI_RT.so
    cp README.staticdist $DEST/README
    tar czvf $DEST.tgz $DEST
}

DIR=$PWD

if [[ $PLATFORM == "x86_64-unknown-linux2.4" ]]; then
    # Do this on x86_64
    DEST=codeCoverage-64
    echo "Packaging $DEST"
    _package $DEST
else 
    DEST=codeCoverage-32
    echo "Packaging $DEST"
    _package $DEST
fi

